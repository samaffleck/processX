#include "processX/kinsol_solver.h"
#include "processX/flowsheet.h"

// CoolProp includes
#include <CoolProp.h>
#include <AbstractState.h>
#include <unordered_set>
#include <iostream>
#include <exception>
#include <cmath>
#include <algorithm>

namespace px {

  // Helper function to enforce state equation: T = f(H, P) or H = f(T, P)
  // Returns residual: 0 when state equation is satisfied
  // Helper function to normalize and validate mole fractions for CoolProp
  // Handles edge cases like near-zero values and values slightly > 1.0
  static std::vector<double> normalize_mole_fractions(const std::vector<double>& raw_fractions) {
    std::vector<double> mole_fracs;
    mole_fracs.reserve(raw_fractions.size());
    double sum = 0.0;
    const double tolerance = 1e-10;
    
    // First pass: clamp and round values very close to 0 or 1
    for (double val : raw_fractions) {
      // Round values very close to 0 or 1 to exact bounds
      if (std::abs(val) < tolerance) {
        val = 0.0;
      } else if (std::abs(val - 1.0) < tolerance) {
        val = 1.0;
      } else {
        // Clamp to [0, 1] range
        if (val < 0.0) val = 0.0;
        if (val > 1.0) val = 1.0;
      }
      mole_fracs.push_back(val);
      sum += val;
    }
    
    // Check if we have a near-pure component case (one component >> others)
    // If so, set the dominant component to 1.0 and others to 0.0
    bool is_near_pure = false;
    size_t dominant_idx = 0;
    double max_val = 0.0;
    for (size_t i = 0; i < mole_fracs.size(); ++i) {
      if (mole_fracs[i] > max_val) {
        max_val = mole_fracs[i];
        dominant_idx = i;
      }
    }
    
    // If the dominant component is > 0.99, treat as pure component
    if (max_val > 0.99 && sum > 0.99) {
      is_near_pure = true;
      for (size_t i = 0; i < mole_fracs.size(); ++i) {
        mole_fracs[i] = (i == dominant_idx) ? 1.0 : 0.0;
      }
    } else {
      // Normalize to sum to 1.0 (CoolProp requirement)
      if (sum > tolerance) {
        for (auto& mf : mole_fracs) {
          mf /= sum;
        }
      } else {
        // If sum is too small, use equal fractions
        double equal_frac = 1.0 / mole_fracs.size();
        for (auto& mf : mole_fracs) {
          mf = equal_frac;
        }
      }
      
      // Final pass: round very small values to 0 and ensure sum is exactly 1.0
      double final_sum = 0.0;
      for (auto& mf : mole_fracs) {
        if (mf < tolerance) {
          mf = 0.0;
        } else if (mf > 1.0 - tolerance) {
          mf = 1.0;
        }
        final_sum += mf;
      }
      
      // If we have a near-pure case after normalization, handle it
      if (final_sum > tolerance) {
        double max_norm = 0.0;
        size_t max_idx = 0;
        for (size_t i = 0; i < mole_fracs.size(); ++i) {
          if (mole_fracs[i] > max_norm) {
            max_norm = mole_fracs[i];
            max_idx = i;
          }
        }
        
        if (max_norm > 0.99) {
          // Near-pure: set dominant to 1.0, others to 0.0
          for (size_t i = 0; i < mole_fracs.size(); ++i) {
            mole_fracs[i] = (i == max_idx) ? 1.0 : 0.0;
          }
        } else {
          // Renormalize to ensure exact sum of 1.0
          for (auto& mf : mole_fracs) {
            mf /= final_sum;
          }
        }
      }
    }
    
    return mole_fracs;
  }

  // Requires: P must be fixed, and at least one of (T, H) must be fixed
  static double state_equation_residual(
    Stream& stream,
    Flowsheet* flowsheet
  ) {
    // Get fluid package for this stream
    auto fluid = flowsheet->fluids.GetFluidPackage(stream.fluid_package_id);
    if (!fluid) {
      std::string msg = "No fluid package found for stream '" + (stream.name.empty() ? "(unnamed)" : stream.name) + 
                       "' with fluid_package_id=" + std::to_string(stream.fluid_package_id);
      std::cerr << "[state_equation_residual] " << msg << std::endl;
      if (flowsheet) {
        flowsheet->log_message(msg, true);
      }
      return 1e10;
    }
    
    // Set mole fractions from stream variables if this is a mixture
    auto components = flowsheet->fluids.GetComponents(stream.fluid_package_id);
    if (components.size() > 1 && stream.mole_fractions.size() == components.size()) {
      try {
        // Extract raw mole fraction values
        std::vector<double> raw_fractions;
        raw_fractions.reserve(stream.mole_fractions.size());
        for (const auto& mf : stream.mole_fractions) {
          raw_fractions.push_back(mf.value_);
        }
        
        // Normalize using helper function
        std::vector<double> mole_fracs = normalize_mole_fractions(raw_fractions);
        
        fluid->set_mole_fractions(mole_fracs);
        
        // Build phase envelope for mixtures if needed (lazy initialization)
        try {
          fluid->build_phase_envelope("none");
        } catch (...) {
          // Phase envelope build may fail, continue anyway
        }
      } catch (const std::exception& e) {
        std::string msg = "Failed to set mole fractions: " + std::string(e.what()) + 
                         " | Stream: " + (stream.name.empty() ? "(unnamed)" : stream.name);
        std::cerr << "[state_equation_residual] " << msg << std::endl;
        if (flowsheet) {
          flowsheet->log_message(msg, true);
        }
        return 1e10;
      }
    }
    
    if (stream.temperature.is_fixed_) {
      // T and P are fixed -> calculate H and compare
      try {
        fluid->update(CoolProp::PT_INPUTS, stream.pressure.value_, stream.temperature.value_);
        double calculated_h = fluid->hmolar();
        return stream.molar_enthalpy.value_ - calculated_h;
      } catch (const std::exception& e) {
        std::string msg = "CoolProp exception (PT_INPUTS): " + std::string(e.what()) + 
                         " | Stream: " + (stream.name.empty() ? "(unnamed)" : stream.name) +
                         " | P = " + std::to_string(stream.pressure.value_) + " Pa, T = " + std::to_string(stream.temperature.value_) + " K";
        std::cerr << "[state_equation_residual] " << msg << std::endl;
        // Also log to callback if available
        if (flowsheet) {
          flowsheet->log_message(msg, true);
        }
        // Return a large residual to indicate failure
        return 1e10;
      } catch (...) {
        std::string msg = "Unknown exception in CoolProp update (PT_INPUTS) | Stream: " + (stream.name.empty() ? "(unnamed)" : stream.name);
        std::cerr << "[state_equation_residual] " << msg << std::endl;
        if (flowsheet) {
          flowsheet->log_message(msg, true);
        }
        return 1e10;
      }
    }
    // H and P are fixed -> calculate T and compare
    try {
      // Prepare mole fractions for the calculation
      std::vector<double> mole_fracs;
      if (components.size() > 1 && stream.mole_fractions.size() == components.size()) {
        // Extract raw mole fraction values
        std::vector<double> raw_fractions;
        raw_fractions.reserve(stream.mole_fractions.size());
        for (const auto& mf : stream.mole_fractions) {
          raw_fractions.push_back(mf.value_);
        }
        
        // Normalize using helper function
        mole_fracs = normalize_mole_fractions(raw_fractions);
      }
      
      // Use FluidRegistry helper method to calculate temperature
      double T_guess = stream.temperature.value_;
      if (T_guess <= 0 || T_guess < 50.0) T_guess = 300.0;
      
      double calculated_t = flowsheet->fluids.CalculateTemperatureFromEnthalpyAndPressure(
        stream.fluid_package_id,
        stream.molar_enthalpy.value_,
        stream.pressure.value_,
        T_guess,
        mole_fracs
      );
      
      return stream.temperature.value_ - calculated_t;
    } catch (const std::exception& e) {
      std::string msg = "CoolProp exception (H,P->T): " + std::string(e.what()) + 
                       " | Stream: " + (stream.name.empty() ? "(unnamed)" : stream.name) +
                       " | P = " + std::to_string(stream.pressure.value_) + " Pa, H = " + std::to_string(stream.molar_enthalpy.value_) + " J/mol";
      std::cerr << "[state_equation_residual] " << msg << std::endl;
      if (flowsheet) {
        flowsheet->log_message(msg, true);
      }
      // Return a large residual to indicate failure
      return 1e10;
    } catch (...) {
      std::string msg = "Unknown exception in CoolProp update (H,P->T) | Stream: " + (stream.name.empty() ? "(unnamed)" : stream.name);
      std::cerr << "[state_equation_residual] " << msg << std::endl;
      if (flowsheet) {
        flowsheet->log_message(msg, true);
      }
      return 1e10;
    }
  }

  // Helper function to get all streams connected to a given stream through unit operations
  static std::vector<Handle<Stream>> get_connected_streams(Handle<Stream> stream_handle, Flowsheet& fs) {
    std::vector<Handle<Stream>> connected;
    
    // Search through all unit operations to find connections
    fs.valves_.for_each([&](Valve& v) {
      if (v.in.index == stream_handle.index && v.out.valid()) connected.push_back(v.out);
      if (v.out.index == stream_handle.index && v.in.valid()) connected.push_back(v.in);
    });
    
    fs.mixers_.for_each([&](Mixer& m) {
      if (m.out.index == stream_handle.index) {
        for (const auto& h : m.in) {
          if (h.valid()) connected.push_back(h);
        }
      }
      for (const auto& h : m.in) {
        if (h.index == stream_handle.index && m.out.valid()) {
          connected.push_back(m.out);
        }
      }
    });
    
    fs.splitters_.for_each([&](Splitter& s) {
      if (s.in.index == stream_handle.index) {
        for (const auto& h : s.out) {
          if (h.valid()) connected.push_back(h);
        }
      }
      for (const auto& h : s.out) {
        if (h.index == stream_handle.index && s.in.valid()) {
          connected.push_back(s.in);
        }
      }
    });
    
    fs.heat_exchangers_.for_each([&](HeatExchanger& hx) {
      if (hx.in_hot.index == stream_handle.index && hx.out_hot.valid()) connected.push_back(hx.out_hot);
      if (hx.out_hot.index == stream_handle.index && hx.in_hot.valid()) connected.push_back(hx.in_hot);
      if (hx.in_cold.index == stream_handle.index && hx.out_cold.valid()) connected.push_back(hx.out_cold);
      if (hx.out_cold.index == stream_handle.index && hx.in_cold.valid()) connected.push_back(hx.in_cold);
    });
    
    fs.simple_heat_exchangers_.for_each([&](SimpleHeatExchanger& shx) {
      if (shx.in.index == stream_handle.index && shx.out.valid()) connected.push_back(shx.out);
      if (shx.out.index == stream_handle.index && shx.in.valid()) connected.push_back(shx.in);
    });
    
    fs.pumps_.for_each([&](Pump& p) {
      if (p.in.index == stream_handle.index && p.out.valid()) connected.push_back(p.out);
      if (p.out.index == stream_handle.index && p.in.valid()) connected.push_back(p.in);
    });
    
    fs.component_splitters_.for_each([&](ComponentSplitter& cs) {
      if (cs.in.index == stream_handle.index) {
        if (cs.overhead.valid()) connected.push_back(cs.overhead);
        if (cs.bottom.valid()) connected.push_back(cs.bottom);
      }
      if (cs.overhead.index == stream_handle.index && cs.in.valid()) connected.push_back(cs.in);
      if (cs.bottom.index == stream_handle.index && cs.in.valid()) connected.push_back(cs.in);
    });
    
    return connected;
  }

  // Helper function to auto-assign fluid packages to streams that don't have one
  static bool auto_assign_fluid_packages(Flowsheet& fs, std::string* err) {
    // First, try to assign from neighbors
    bool changed = true;
    while (changed) {
      changed = false;
      fs.streams_.for_each_with_handle([&](Stream& s, Handle<Stream> h) {
        // Skip if stream already has a valid fluid package
        if (s.fluid_package_id != 0 && fs.fluids.GetFluidPackage(s.fluid_package_id)) {
          return;
        }
        
        // Try to find a fluid package from connected streams
        auto connected = get_connected_streams(h, fs);
        for (const auto& conn_h : connected) {
          auto& conn_s = fs.get<Stream>(conn_h);
          if (conn_s.fluid_package_id != 0 && fs.fluids.GetFluidPackage(conn_s.fluid_package_id)) {
            s.fluid_package_id = conn_s.fluid_package_id;
            // Initialize composition variables based on fluid package components
            auto components = fs.fluids.GetComponents(conn_s.fluid_package_id);
            s.initialize_composition(components.size(), components);
            changed = true;
            return;
          }
        }
      });
    }
    
    // For any remaining streams without a package, assign the first available
    size_t first_pkg_id = fs.fluids.GetFirstAvailablePackageId();
    if (first_pkg_id == 0) {
      // No packages available at all
      if (err) *err = "No fluid packages available in registry. At least one fluid package must be added.";
      return false;
    }
    
    fs.streams_.for_each([&](Stream& s) {
      if (s.fluid_package_id == 0 || !fs.fluids.GetFluidPackage(s.fluid_package_id)) {
        s.fluid_package_id = first_pkg_id;
        // Initialize composition variables based on fluid package components
        auto components = fs.fluids.GetComponents(first_pkg_id);
        s.initialize_composition(components.size(), components);
      }
    });
    
    return true;
  }

  // Helper function to clean up invalid stream references from unit operations
  static void cleanup_invalid_stream_references(Flowsheet& fs) {
    // Clean up valves
    fs.valves_.for_each([&](Valve& v) {
      if (v.in.valid() && !fs.streams_.contains(v.in)) {
        v.in = Handle<Stream>{}; // Clear invalid handle
      }
      if (v.out.valid() && !fs.streams_.contains(v.out)) {
        v.out = Handle<Stream>{}; // Clear invalid handle
      }
    });
    
    // Clean up mixers
    fs.mixers_.for_each([&](Mixer& m) {
      // Remove invalid inlets
      m.in.erase(
        std::remove_if(m.in.begin(), m.in.end(),
          [&](const Handle<Stream>& h) {
            return h.valid() && !fs.streams_.contains(h);
          }),
        m.in.end()
      );
      // Clear invalid outlet
      if (m.out.valid() && !fs.streams_.contains(m.out)) {
        m.out = Handle<Stream>{};
      }
    });
    
    // Clean up splitters
    fs.splitters_.for_each([&](Splitter& s) {
      // Clear invalid inlet
      if (s.in.valid() && !fs.streams_.contains(s.in)) {
        s.in = Handle<Stream>{};
      }
      // Remove invalid outlets
      s.out.erase(
        std::remove_if(s.out.begin(), s.out.end(),
          [&](const Handle<Stream>& h) {
            return h.valid() && !fs.streams_.contains(h);
          }),
        s.out.end()
      );
    });
    
    // Clean up heat exchangers
    fs.heat_exchangers_.for_each([&](HeatExchanger& hx) {
      if (hx.in_hot.valid() && !fs.streams_.contains(hx.in_hot)) {
        hx.in_hot = Handle<Stream>{};
      }
      if (hx.out_hot.valid() && !fs.streams_.contains(hx.out_hot)) {
        hx.out_hot = Handle<Stream>{};
      }
      if (hx.in_cold.valid() && !fs.streams_.contains(hx.in_cold)) {
        hx.in_cold = Handle<Stream>{};
      }
      if (hx.out_cold.valid() && !fs.streams_.contains(hx.out_cold)) {
        hx.out_cold = Handle<Stream>{};
      }
    });
    
    // Clean up simple heat exchangers
    fs.simple_heat_exchangers_.for_each([&](SimpleHeatExchanger& shx) {
      if (shx.in.valid() && !fs.streams_.contains(shx.in)) {
        shx.in = Handle<Stream>{};
      }
      if (shx.out.valid() && !fs.streams_.contains(shx.out)) {
        shx.out = Handle<Stream>{};
      }
    });
    
    // Clean up pumps
    fs.pumps_.for_each([&](Pump& p) {
      if (p.in.valid() && !fs.streams_.contains(p.in)) {
        p.in = Handle<Stream>{};
      }
      if (p.out.valid() && !fs.streams_.contains(p.out)) {
        p.out = Handle<Stream>{};
      }
    });
    
    // Clean up component splitters
    fs.component_splitters_.for_each([&](ComponentSplitter& cs) {
      if (cs.in.valid() && !fs.streams_.contains(cs.in)) {
        cs.in = Handle<Stream>{};
      }
      if (cs.overhead.valid() && !fs.streams_.contains(cs.overhead)) {
        cs.overhead = Handle<Stream>{};
      }
      if (cs.bottom.valid() && !fs.streams_.contains(cs.bottom)) {
        cs.bottom = Handle<Stream>{};
      }
    });
  }

  // Helper function to validate that connected streams have the same fluid package
  static bool validate_fluid_consistency(IUnitOp* u, Flowsheet& fs, std::string* err) {
    // Get all connected streams for this unit operation
    std::vector<Handle<Stream>> connected_streams;
    
    if (auto* v = dynamic_cast<Valve*>(u)) {
      if (v->in.valid() && fs.streams_.contains(v->in)) connected_streams.push_back(v->in);
      if (v->out.valid() && fs.streams_.contains(v->out)) connected_streams.push_back(v->out);
    } else if (auto* m = dynamic_cast<Mixer*>(u)) {
      for (const auto& h : m->in) {
        if (h.valid() && fs.streams_.contains(h)) connected_streams.push_back(h);
      }
      if (m->out.valid() && fs.streams_.contains(m->out)) connected_streams.push_back(m->out);
    } else if (auto* s = dynamic_cast<Splitter*>(u)) {
      if (s->in.valid() && fs.streams_.contains(s->in)) connected_streams.push_back(s->in);
      for (const auto& h : s->out) {
        if (h.valid() && fs.streams_.contains(h)) connected_streams.push_back(h);
      }
    } else if (auto* hx = dynamic_cast<HeatExchanger*>(u)) {
      // For heat exchangers, hot side and cold side can have different fluids
      // But streams on the same side must have the same fluid
      if (hx->in_hot.valid() && fs.streams_.contains(hx->in_hot)) connected_streams.push_back(hx->in_hot);
      if (hx->out_hot.valid() && fs.streams_.contains(hx->out_hot)) connected_streams.push_back(hx->out_hot);
      // Check hot side consistency
      if (connected_streams.size() >= 2) {
        size_t hot_fluid_id = fs.get<Stream>(connected_streams[0]).fluid_package_id;
        for (size_t i = 1; i < connected_streams.size(); ++i) {
          if (fs.get<Stream>(connected_streams[i]).fluid_package_id != hot_fluid_id) {
            if (err) *err = "Hot side streams in " + std::string(u->type_name()) + 
                           " '" + u->get_name() + "' have different fluid packages";
            return false;
          }
        }
      }
      connected_streams.clear();
      if (hx->in_cold.valid() && fs.streams_.contains(hx->in_cold)) connected_streams.push_back(hx->in_cold);
      if (hx->out_cold.valid() && fs.streams_.contains(hx->out_cold)) connected_streams.push_back(hx->out_cold);
      // Check cold side consistency
      if (connected_streams.size() >= 2) {
        size_t cold_fluid_id = fs.get<Stream>(connected_streams[0]).fluid_package_id;
        for (size_t i = 1; i < connected_streams.size(); ++i) {
          if (fs.get<Stream>(connected_streams[i]).fluid_package_id != cold_fluid_id) {
            if (err) *err = "Cold side streams in " + std::string(u->type_name()) + 
                           " '" + u->get_name() + "' have different fluid packages";
            return false;
          }
        }
      }
      return true; // Heat exchanger validation complete
    } else if (auto* shx = dynamic_cast<SimpleHeatExchanger*>(u)) {
      if (shx->in.valid() && fs.streams_.contains(shx->in)) connected_streams.push_back(shx->in);
      if (shx->out.valid() && fs.streams_.contains(shx->out)) connected_streams.push_back(shx->out);
    } else if (auto* p = dynamic_cast<Pump*>(u)) {
      if (p->in.valid() && fs.streams_.contains(p->in)) connected_streams.push_back(p->in);
      if (p->out.valid() && fs.streams_.contains(p->out)) connected_streams.push_back(p->out);
    } else if (auto* cs = dynamic_cast<ComponentSplitter*>(u)) {
      if (cs->in.valid() && fs.streams_.contains(cs->in)) connected_streams.push_back(cs->in);
      if (cs->overhead.valid() && fs.streams_.contains(cs->overhead)) connected_streams.push_back(cs->overhead);
      if (cs->bottom.valid() && fs.streams_.contains(cs->bottom)) connected_streams.push_back(cs->bottom);
    }
    
    // For all other unit operations, check that all connected streams have the same fluid package
    if (connected_streams.size() >= 2) {
      size_t fluid_id = fs.get<Stream>(connected_streams[0]).fluid_package_id;
      for (size_t i = 1; i < connected_streams.size(); ++i) {
        if (fs.get<Stream>(connected_streams[i]).fluid_package_id != fluid_id) {
          if (err) *err = "Connected streams in " + std::string(u->type_name()) + 
                         " '" + u->get_name() + "' have different fluid packages";
          return false;
        }
      }
    }
    
    return true;
  }

  bool Flowsheet::assemble(std::string* err) {
    // Ensure fluid is initialized before using it
    ensure_fluid_initialized();
    
    // Clean up invalid stream references from unit operations (e.g., deleted streams)
    cleanup_invalid_stream_references(*this);
    
    units_.clear();
    build_unit_list();

    reg.Clear(); sys.Clear();

    // Auto-assign fluid packages to streams that don't have one
    if (!auto_assign_fluid_packages(*this, err)) {
      return false;
    }

    // Validate units
    for (auto* u : units_) {
      std::string why;
      if (!u->validate(*this, &why)) {
        if (err) *err = std::string(u->type_name()) + " '" + u->get_name() + "' invalid: " + why;
        return false;
      }
    }

    // Validate fluid package consistency for connected streams
    for (auto* u : units_) {
      std::string why;
      if (!validate_fluid_consistency(u, *this, &why)) {
        if (err) *err = why;
        return false;
      }
    }

    // Register unknowns & equations
    for (auto* u : units_) u->register_unknowns(*this, reg);
    for (auto* u : units_) u->add_equations(*this, sys);
    
    // Initialize molar enthalpy from temperature and pressure if not already set
    // This provides sensible initial guesses to avoid CoolProp exceptions
    streams_.for_each([this](Stream& s) {
      // Only initialize if enthalpy is not fixed and has a zero or unreasonable value
      if (!s.molar_enthalpy.is_fixed_) {
        auto fluid = fluids.GetFluidPackage(s.fluid_package_id);
        if (fluid) {
          try {
            // For mixtures, set mole fractions before updating
            // CoolProp requires mole fractions to be set for mixtures
            auto components = fluids.GetComponents(s.fluid_package_id);
            if (components.size() > 1) {
              std::vector<double> mole_fracs;
              
              if (s.mole_fractions.size() == components.size()) {
                // Extract raw mole fraction values
                std::vector<double> raw_fractions;
                raw_fractions.reserve(s.mole_fractions.size());
                for (const auto& mf : s.mole_fractions) {
                  raw_fractions.push_back(mf.value_);
                }
                
                // Normalize using helper function
                mole_fracs = normalize_mole_fractions(raw_fractions);
              } else {
                // Mole fractions not initialized yet - use equal fractions as default
                // This can happen when streams are first created before unit operations initialize them
                mole_fracs.resize(components.size(), 1.0 / components.size());
              }
              
              fluid->set_mole_fractions(mole_fracs);
            }
            
            fluid->update(CoolProp::PT_INPUTS, s.pressure.value_, s.temperature.value_);
            s.molar_enthalpy.value_ = fluid->hmolar();
          } catch (const std::exception& e) {
            // If CoolProp fails, leave enthalpy as is (will be set by solver)
            std::string msg = "CoolProp exception during enthalpy initialization: " + std::string(e.what()) + 
                             " | Stream: " + (s.name.empty() ? "(unnamed)" : s.name) +
                             " | P = " + std::to_string(s.pressure.value_) + " Pa, T = " + std::to_string(s.temperature.value_) + " K";
            std::cerr << "[Flowsheet::assemble] " << msg << std::endl;
            log_message(msg, true);
          } catch (...) {
            std::string msg = "Unknown exception in CoolProp update during enthalpy initialization | Stream: " + (s.name.empty() ? "(unnamed)" : s.name);
            std::cerr << "[Flowsheet::assemble] " << msg << std::endl;
            log_message(msg, true);
          }
        } else {
          std::string msg = "No fluid package found for stream '" + (s.name.empty() ? "(unnamed)" : s.name) + 
                           "' with fluid_package_id=" + std::to_string(s.fluid_package_id);
          std::cerr << "[Flowsheet::assemble] " << msg << std::endl;
          log_message(msg, true);
        }
      }
    });
    
    // Add state equations once per stream to avoid duplicates
    // Use a set to track which streams have already had state equations added
    std::unordered_set<uint32_t> streams_with_state_eq;
    streams_.for_each_with_handle([&](Stream& s, Handle<Stream> h) {
      // Only add state equation once per stream (track by handle index)
      if (streams_with_state_eq.find(h.index) == streams_with_state_eq.end()) {
        streams_with_state_eq.insert(h.index);
        sys.AddEquation("state_eq[" + s.name + "]", [&s, this]() {
          return state_equation_residual(s, this);
        });
      }

      if (!s.molar_flow.is_fixed_ && s.molar_flow.value_ == 0.0) {
        s.molar_flow.value_ = 1e-12;
      }
    });

    // DOF check: allow equations >= unknowns (redundancy will be caught by rank analysis)
    int num_of_unknowns = reg.GetNumberOfUnknowns();
    int num_of_equations = sys.GetNumberOfEquations();
    if (num_of_unknowns > num_of_equations) {
      if (err) *err = "DOF mismatch: unknowns=" + std::to_string(num_of_unknowns)
                    + " > equations=" + std::to_string(num_of_equations) + " (under-specified)";
      return false;
    }
    // Note: equations > unknowns is OK - rank analysis will detect redundant equations

    // Structural analysis (same helpers we built earlier)
    // Use default values for tiny_row and tiny_res
    auto a = analyze_system(reg, sys, 1e-6, 1e-8, 1e-14, 1e-12);
    if (!a.inconsistent_eqs.empty()) {
      if (err) {
        *err = "Inconsistent equations:\n";
        for (int i : a.inconsistent_eqs)
          *err += "  - " + sys.GetEquationNames()[(size_t)i] + " (res=" + std::to_string(a.r[(size_t)i]) + ")\n";
      }
      return false;
    }
    // Check for redundant equations - only fail if rank is insufficient
    // Redundant equations with zero residual are OK (e.g., pressure equalities when all pressures are fixed)
    if (!a.redundant_eqs.empty() && a.rank < num_of_unknowns) {
      if (err) {
        *err = "Redundant equations causing rank deficiency:\n";
        for (int i : a.redundant_eqs) *err += "  - " + sys.GetEquationNames()[(size_t)i] + "\n";
        *err += "rank(J)=" + std::to_string(a.rank) + " < unknowns=" + std::to_string(num_of_unknowns);
      }
      return false;
    }
    if (a.rank < num_of_unknowns) {
      if (err) {
        *err = "Rank-deficient: rank(J)=" + std::to_string(a.rank) + " < " + std::to_string(num_of_unknowns);
        if (!a.unconstrained_unknowns.empty()) {
          *err += "\nUnconstrained unknowns:";
          for (int j : a.unconstrained_unknowns) *err += " " + reg.GetFeeVariableNames()[(size_t)j];
        }
      }
      return false;
    }

    return true;
  }
  
} // namespace  px

CEREAL_CLASS_VERSION(px::Flowsheet, 0)
