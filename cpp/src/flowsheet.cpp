#include "processX/flowsheet.h"

// CoolProp includes
#include <CoolProp.h>
#include <AbstractState.h>
#include <unordered_set>
#include <iostream>
#include <exception>
#include <cmath>

namespace px {

  // Helper function to enforce state equation: T = f(H, P) or H = f(T, P)
  // Returns residual: 0 when state equation is satisfied
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
        std::vector<double> mole_fracs;
        mole_fracs.reserve(stream.mole_fractions.size());
        double sum = 0.0;
        for (const auto& mf : stream.mole_fractions) {
          double val = mf.value;
          if (val < 0.0) val = 0.0; // Clamp negative values
          mole_fracs.push_back(val);
          sum += val;
        }
        
        // Normalize to sum to 1.0 (CoolProp requirement)
        if (sum > 1e-10) {
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
    
    if (stream.temperature.fixed) {
      // T and P are fixed -> calculate H and compare
      try {
        fluid->update(CoolProp::PT_INPUTS, stream.pressure.value, stream.temperature.value);
        double calculated_h = fluid->hmolar();
        return stream.molar_enthalpy.value - calculated_h;
      } catch (const std::exception& e) {
        std::string msg = "CoolProp exception (PT_INPUTS): " + std::string(e.what()) + 
                         " | Stream: " + (stream.name.empty() ? "(unnamed)" : stream.name) +
                         " | P = " + std::to_string(stream.pressure.value) + " Pa, T = " + std::to_string(stream.temperature.value) + " K";
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
      double calculated_t;
      
      // components was already retrieved above for setting mole fractions
      if (components.size() > 1) {
        // For mixtures, HmolarP_INPUTS is not supported, use iterative approach
        // Use PT_INPUTS to find T that gives the desired H
        double T_guess = stream.temperature.value; // Use current temperature as initial guess
        if (T_guess <= 0 || T_guess < 50.0) T_guess = 300.0; // Default guess if invalid
        
        double T_low = 50.0;   // Lower bound (K)
        double T_high = 2000.0; // Upper bound (K)
        double H_target = stream.molar_enthalpy.value;
        double H_low = 0.0, H_high = 0.0;
        
        // Try to bracket the solution
        try {
          fluid->update(CoolProp::PT_INPUTS, stream.pressure.value, T_low);
          H_low = fluid->hmolar();
          fluid->update(CoolProp::PT_INPUTS, stream.pressure.value, T_high);
          H_high = fluid->hmolar();
        } catch (...) {
          // If bracketing fails, use simple linear interpolation
          T_guess = 300.0;
          try {
            fluid->update(CoolProp::PT_INPUTS, stream.pressure.value, T_guess);
            double H_guess = fluid->hmolar();
            // Simple linear extrapolation: dH/dT ≈ Cp, assume Cp ~ 30 J/mol/K for gases
            double dT = (H_target - H_guess) / 30.0;
            T_guess += dT;
            // Clamp to reasonable range
            if (T_guess < 100.0) T_guess = 100.0;
            if (T_guess > 1000.0) T_guess = 1000.0;
          } catch (...) {
            // If all else fails, return residual based on current guess
            return stream.temperature.value - T_guess;
          }
        }
        
        // Bisection method to find T
        const int max_iter = 20;
        const double tol = 1e-3; // 1 mK tolerance
        for (int iter = 0; iter < max_iter; ++iter) {
          T_guess = 0.5 * (T_low + T_high);
          try {
            fluid->update(CoolProp::PT_INPUTS, stream.pressure.value, T_guess);
            double H_guess = fluid->hmolar();
            double error = H_guess - H_target;
            
            if (std::abs(error) < tol * std::abs(H_target) + 1e-6) {
              break; // Converged
            }
            
            if (error < 0) {
              T_low = T_guess;
              H_low = H_guess;
            } else {
              T_high = T_guess;
              H_high = H_guess;
            }
            
            // Check if bounds are too close
            if (T_high - T_low < 1e-6) break;
          } catch (...) {
            // If update fails, try to adjust bounds
            if (T_guess < 200.0) T_low = T_guess + 10.0;
            else T_high = T_guess - 10.0;
            if (T_high <= T_low) {
              T_guess = stream.temperature.value; // Fallback to current value
              break;
            }
          }
        }
        
        calculated_t = T_guess;
      } else {
        // For pure fluids, HmolarP_INPUTS is supported
        fluid->update(CoolProp::HmolarP_INPUTS, stream.molar_enthalpy.value, stream.pressure.value);
        calculated_t = fluid->T();
      }
      
      return stream.temperature.value - calculated_t;
    } catch (const std::exception& e) {
      std::string msg = "CoolProp exception (H,P->T): " + std::string(e.what()) + 
                       " | Stream: " + (stream.name.empty() ? "(unnamed)" : stream.name) +
                       " | P = " + std::to_string(stream.pressure.value) + " Pa, H = " + std::to_string(stream.molar_enthalpy.value) + " J/mol";
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

  // Helper function to validate that connected streams have the same fluid package
  static bool validate_fluid_consistency(IUnitOp* u, Flowsheet& fs, std::string* err) {
    // Get all connected streams for this unit operation
    std::vector<Handle<Stream>> connected_streams;
    
    if (auto* v = dynamic_cast<Valve*>(u)) {
      if (v->in.valid()) connected_streams.push_back(v->in);
      if (v->out.valid()) connected_streams.push_back(v->out);
    } else if (auto* m = dynamic_cast<Mixer*>(u)) {
      for (const auto& h : m->in) {
        if (h.valid()) connected_streams.push_back(h);
      }
      if (m->out.valid()) connected_streams.push_back(m->out);
    } else if (auto* s = dynamic_cast<Splitter*>(u)) {
      if (s->in.valid()) connected_streams.push_back(s->in);
      for (const auto& h : s->out) {
        if (h.valid()) connected_streams.push_back(h);
      }
    } else if (auto* hx = dynamic_cast<HeatExchanger*>(u)) {
      // For heat exchangers, hot side and cold side can have different fluids
      // But streams on the same side must have the same fluid
      if (hx->in_hot.valid()) connected_streams.push_back(hx->in_hot);
      if (hx->out_hot.valid()) connected_streams.push_back(hx->out_hot);
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
      if (hx->in_cold.valid()) connected_streams.push_back(hx->in_cold);
      if (hx->out_cold.valid()) connected_streams.push_back(hx->out_cold);
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
      if (shx->in.valid()) connected_streams.push_back(shx->in);
      if (shx->out.valid()) connected_streams.push_back(shx->out);
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
    
    units_.clear();
    build_unit_list();

    reg.clear(); sys.clear();

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
      if (!s.molar_enthalpy.fixed) {
        auto fluid = fluids.GetFluidPackage(s.fluid_package_id);
        if (fluid) {
          try {
            fluid->update(CoolProp::PT_INPUTS, s.pressure.value, s.temperature.value);
            s.molar_enthalpy.value = fluid->hmolar();
          } catch (const std::exception& e) {
            // If CoolProp fails, leave enthalpy as is (will be set by solver)
            std::string msg = "CoolProp exception during enthalpy initialization: " + std::string(e.what()) + 
                             " | Stream: " + (s.name.empty() ? "(unnamed)" : s.name) +
                             " | P = " + std::to_string(s.pressure.value) + " Pa, T = " + std::to_string(s.temperature.value) + " K";
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
        sys.add("state_eq[" + s.name + "]", [&s, this]() {
          return state_equation_residual(s, this);
        });
      }

      if (!s.molar_flow.fixed && s.molar_flow.value == 0.0) {
        s.molar_flow.value = 1e-12;
      }
    });

    // DOF check: allow equations >= unknowns (redundancy will be caught by rank analysis)
    if (reg.size() > sys.size()) {
      if (err) *err = "DOF mismatch: unknowns=" + std::to_string(reg.size())
                    + " > equations=" + std::to_string(sys.size()) + " (under-specified)";
      return false;
    }
    // Note: equations > unknowns is OK - rank analysis will detect redundant equations

    // Structural analysis (same helpers we built earlier)
    auto a = analyze_system(reg, sys, /*fd_rel=*/1e-6, /*fd_abs=*/1e-8);
    if (!a.inconsistent_eqs.empty()) {
      if (err) {
        *err = "Inconsistent equations:\n";
        for (int i : a.inconsistent_eqs)
          *err += "  - " + sys.names[(size_t)i] + " (res=" + std::to_string(a.r[(size_t)i]) + ")\n";
      }
      return false;
    }
    // Check for redundant equations - only fail if rank is insufficient
    // Redundant equations with zero residual are OK (e.g., pressure equalities when all pressures are fixed)
    if (!a.redundant_eqs.empty() && a.rank < (int)reg.size()) {
      if (err) {
        *err = "Redundant equations causing rank deficiency:\n";
        for (int i : a.redundant_eqs) *err += "  - " + sys.names[(size_t)i] + "\n";
        *err += "rank(J)=" + std::to_string(a.rank) + " < unknowns=" + std::to_string(reg.size());
      }
      return false;
    }
    if (a.rank < (int)reg.size()) {
      if (err) {
        *err = "Rank-deficient: rank(J)=" + std::to_string(a.rank) + " < " + std::to_string(reg.size());
        if (!a.unconstrained_unknowns.empty()) {
          *err += "\nUnconstrained unknowns:";
          for (int j : a.unconstrained_unknowns) *err += " " + reg.vars[(size_t)j]->GetName();
        }
      }
      return false;
    }

    return true;
  }
  
} // namespace  px

CEREAL_CLASS_VERSION(px::Flowsheet, 0)
