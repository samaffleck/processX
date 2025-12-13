// src/unitop.cpp

#include "processX/unitop.h"
#include "processX/flowsheet.h"
#include "processX/registry.h"

namespace px {

  bool Valve::validate(const Flowsheet& fs, std::string* why) const {
    if (!in.valid())  { if (why) *why = "inlet not connected";  return false; }
    if (!out.valid()) { if (why) *why = "outlet not connected"; return false; }
    return true;
  }

  void Valve::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    reg.AddVariable(si.molar_flow);
    reg.AddVariable(so.molar_flow);
    reg.AddVariable(si.pressure);
    reg.AddVariable(so.pressure);
    reg.AddVariable(si.molar_enthalpy);
    reg.AddVariable(so.molar_enthalpy);
    reg.AddVariable(si.temperature);  // Needed for state equation: T = f(H, P)
    reg.AddVariable(so.temperature);  // Needed for state equation: T = f(H, P)
    reg.AddVariable(Cv);
    
    // Register mole fractions for inlet and outlet
    for (auto& x : si.mole_fractions) {
      reg.AddVariable(x);
    }
    for (auto& x : so.mole_fractions) {
      reg.AddVariable(x);
    }
  }

  void Valve::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    sys.AddEquation(name + ": balance", [&](){ return si.molar_flow.value_ - so.molar_flow.value_; });
    sys.AddEquation(name + ": PF",      [&, self](){ return so.molar_flow.value_ - self->Cv.value_ * (si.pressure.value_ - so.pressure.value_); });
    sys.AddEquation(name + ": h_equal", [&](){ return si.molar_enthalpy.value_ - so.molar_enthalpy.value_; });
    
    // Component mass balances: composition unchanged (x_in = x_out for each component)
    size_t num_components = si.mole_fractions.size();
    for (size_t i = 0; i < num_components; ++i) {
      sys.AddEquation(name + ": comp_balance[" + std::to_string(i) + "]", [&, i]() {
        return si.mole_fractions[i].value_ - so.mole_fractions[i].value_;
      });
    }
    // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
  }

  bool Mixer::validate(const Flowsheet& fs, std::string* why) const {
    if (!out.valid()) { if (why) *why = "outlet not connected"; return false; }
    if (in.size() < 1) { if (why) *why = "mixer needs at least one inlet"; return false; }
    for (size_t i = 0; i < in.size(); ++i) {
      if (!in[i].valid()) { if (why) *why = "one or more inlets not connected"; return false; }
    }
    return true;
  }

  void Mixer::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& so = fs.get<Stream>(out);
    reg.AddVariable(so.molar_flow);
    reg.AddVariable(so.pressure);
    reg.AddVariable(so.molar_enthalpy);
    reg.AddVariable(so.temperature);  // Needed for state equation: T = f(H, P)
    
    // Register mole fractions for outlet
    for (auto& x : so.mole_fractions) {
      reg.AddVariable(x);
    }

    for (auto h : in) {
      auto& si = fs.get<Stream>(h);
      reg.AddVariable(si.molar_flow);
      reg.AddVariable(si.pressure);
      reg.AddVariable(si.molar_enthalpy);
      reg.AddVariable(si.temperature);  // Needed for state equation: T = f(H, P)
      
      // Register mole fractions for each inlet
      for (auto& x : si.mole_fractions) {
        reg.AddVariable(x);
      }
    }
  }

  void Mixer::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto& so = fs.get<Stream>(out);

    sys.AddEquation(name + ": total balance", [&]() {
      double sum_in = 0.0;
      for (auto h : in) sum_in += fs.get<Stream>(h).molar_flow.value_;
      return sum_in - so.molar_flow.value_;
    });

    sys.AddEquation(name + ": energy balance", [&]() {
      double sum_in = 0.0;
      for (auto h : in) {
        auto& s = fs.get<Stream>(h);
        sum_in += s.molar_flow.value_ * s.molar_enthalpy.value_;
      }
      return sum_in - so.molar_flow.value_ * so.molar_enthalpy.value_;
    });

    for (size_t i = 0; i < in.size(); ++i) {
      auto& si = fs.get<Stream>(in[i]);
      sys.AddEquation(name + ": p_equal[" + std::to_string(i) + "]", [&si, &so]() {
        return si.pressure.value_ - so.pressure.value_;
      });
      // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
    }
    
    // Component mass balances: Σ(n_in * x_in,i) = n_out * x_out,i for each component i
    size_t num_components = so.mole_fractions.size();
    for (size_t comp_idx = 0; comp_idx < num_components; ++comp_idx) {
      sys.AddEquation(name + ": comp_balance[" + std::to_string(comp_idx) + "]", [&, comp_idx]() {
        double sum_in = 0.0;
        for (auto h : in) {
          auto& s = fs.get<Stream>(h);
          if (comp_idx < s.mole_fractions.size()) {
            sum_in += s.molar_flow.value_ * s.mole_fractions[comp_idx].value_;
          }
        }
        return sum_in - so.molar_flow.value_ * so.mole_fractions[comp_idx].value_;
      });
    }
  }

  bool Splitter::validate(const Flowsheet& fs, std::string* why) const {
    if (!in.valid()) { if (why) *why = "inlet not connected"; return false; }
    if (out.size() < 1) { if (why) *why = "splitter needs at least one outlet"; return false; }
    for (size_t i = 0; i < out.size(); ++i) {
      if (!out[i].valid()) { if (why) *why = "one or more outlets not connected"; return false; }
    }
    return true;
  }

  void Splitter::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& si = fs.get<Stream>(in);
    reg.AddVariable(si.molar_flow);
    reg.AddVariable(si.pressure);
    reg.AddVariable(si.molar_enthalpy);
    reg.AddVariable(si.temperature);  // Needed for state equation: T = f(H, P)
    
    // Register mole fractions for inlet
    for (auto& x : si.mole_fractions) {
      reg.AddVariable(x);
    }

    for (auto h : out) {
      auto& so = fs.get<Stream>(h);
      reg.AddVariable(so.molar_flow);
      reg.AddVariable(so.pressure);
      reg.AddVariable(so.molar_enthalpy);
      reg.AddVariable(so.temperature);  // Needed for state equation: T = f(H, P)
      
      // Register mole fractions for each outlet
      for (auto& x : so.mole_fractions) {
        reg.AddVariable(x);
      }
    }
  }

  void Splitter::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto& si = fs.get<Stream>(in);

    sys.AddEquation(name + ": total balance", [&]() {
      double sum_out = 0.0;
      for (auto h : out) sum_out += fs.get<Stream>(h).molar_flow.value_;
      return si.molar_flow.value_ - sum_out;
    });

    sys.AddEquation(name + ": energy balance", [&]() {
      double sum_out = 0.0;
      for (auto h : out) {
        auto& s = fs.get<Stream>(h);
        sum_out += s.molar_flow.value_ * s.molar_enthalpy.value_;
      }
      return si.molar_flow.value_ * si.molar_enthalpy.value_ - sum_out;
    });

    for (size_t i = 0; i < out.size(); ++i) {
      auto& so = fs.get<Stream>(out[i]);
      sys.AddEquation(name + ": p_equal[" + std::to_string(i) + "]", [&si, &so]() {
        return si.pressure.value_ - so.pressure.value_;
      });
      sys.AddEquation(name + ": h_equal[" + std::to_string(i) + "]", [&si, &so]() {
        return si.molar_enthalpy.value_ - so.molar_enthalpy.value_;
      });
      // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
    }
    
    // Component mass balances: n_in * x_in,i = Σ(n_out * x_out,i) for each component i
    // Equivalently: x_in,i = x_out,i for all outlets (composition unchanged)
    size_t num_components = si.mole_fractions.size();
    for (size_t comp_idx = 0; comp_idx < num_components; ++comp_idx) {
      for (size_t out_idx = 0; out_idx < out.size(); ++out_idx) {
        auto& so = fs.get<Stream>(out[out_idx]);
        sys.AddEquation(name + ": comp_balance[" + std::to_string(comp_idx) + "][" + std::to_string(out_idx) + "]", [&, comp_idx, out_idx]() {
          if (comp_idx < si.mole_fractions.size() && comp_idx < so.mole_fractions.size()) {
            return si.mole_fractions[comp_idx].value_ - so.mole_fractions[comp_idx].value_;
          }
          return 0.0; // Should not happen if streams are properly initialized
        });
      }
    }
  }

  bool HeatExchanger::validate(const Flowsheet& fs, std::string* why) const {
    if (!in_hot.valid())  { if (why) *why = "hot inlet not connected";  return false; }
    if (!out_hot.valid()) { if (why) *why = "hot outlet not connected"; return false; }
    if (!in_cold.valid())  { if (why) *why = "cold inlet not connected";  return false; }
    if (!out_cold.valid()) { if (why) *why = "cold outlet not connected"; return false; }
    return true;
  }

  void HeatExchanger::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& si_hot = fs.get<Stream>(in_hot);
    auto& so_hot = fs.get<Stream>(out_hot);
    auto& si_cold = fs.get<Stream>(in_cold);
    auto& so_cold = fs.get<Stream>(out_cold);

    // Hot side
    reg.AddVariable(si_hot.molar_flow);
    reg.AddVariable(so_hot.molar_flow);
    reg.AddVariable(si_hot.pressure);
    reg.AddVariable(so_hot.pressure);
    reg.AddVariable(si_hot.molar_enthalpy);
    reg.AddVariable(so_hot.molar_enthalpy);
    reg.AddVariable(si_hot.temperature);  // Needed for state equation: T = f(H, P)
    reg.AddVariable(so_hot.temperature);  // Needed for state equation: T = f(H, P)
    
    // Register mole fractions for hot side
    for (auto& x : si_hot.mole_fractions) {
      reg.AddVariable(x);
    }
    for (auto& x : so_hot.mole_fractions) {
      reg.AddVariable(x);
    }

    // Cold side
    reg.AddVariable(si_cold.molar_flow);
    reg.AddVariable(so_cold.molar_flow);
    reg.AddVariable(si_cold.pressure);
    reg.AddVariable(so_cold.pressure);
    reg.AddVariable(si_cold.molar_enthalpy);
    reg.AddVariable(so_cold.molar_enthalpy);
    reg.AddVariable(si_cold.temperature);  // Needed for state equation: T = f(H, P)
    reg.AddVariable(so_cold.temperature);  // Needed for state equation: T = f(H, P)
    
    // Register mole fractions for cold side
    for (auto& x : si_cold.mole_fractions) {
      reg.AddVariable(x);
    }
    for (auto& x : so_cold.mole_fractions) {
      reg.AddVariable(x);
    }

    // Heat exchanger parameters
    reg.AddVariable(dP_hot);
    reg.AddVariable(dP_cold);
    reg.AddVariable(Q);
  }

  void HeatExchanger::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto& si_hot = fs.get<Stream>(in_hot);
    auto& so_hot = fs.get<Stream>(out_hot);
    auto& si_cold = fs.get<Stream>(in_cold);
    auto& so_cold = fs.get<Stream>(out_cold);
    auto* self = this;

    // Mass balance - hot side
    sys.AddEquation(name + ": hot mass balance", [&]() {
      return si_hot.molar_flow.value_ - so_hot.molar_flow.value_;
    });

    // Mass balance - cold side
    sys.AddEquation(name + ": cold mass balance", [&]() {
      return si_cold.molar_flow.value_ - so_cold.molar_flow.value_;
    });

    // Pressure drop - hot side: P_out = P_in - dP
    sys.AddEquation(name + ": hot pressure drop", [&, self]() {
      return so_hot.pressure.value_ - (si_hot.pressure.value_ - self->dP_hot.value_);
    });

    // Pressure drop - cold side: P_out = P_in - dP
    sys.AddEquation(name + ": cold pressure drop", [&, self]() {
      return so_cold.pressure.value_ - (si_cold.pressure.value_ - self->dP_cold.value_);
    });

    // Energy balance equations
    if (self->Q.is_fixed_) {
      // When Q is fixed, we have two independent equations:
      // Q = m_hot * (h_hot_in - h_hot_out) - constrains hot side
      // Q = m_cold * (h_cold_out - h_cold_in) - constrains cold side
      sys.AddEquation(name + ": hot energy balance", [&, self]() {
        return self->Q.value_ - si_hot.molar_flow.value_ * (si_hot.molar_enthalpy.value_ - so_hot.molar_enthalpy.value_);
      });
      sys.AddEquation(name + ": cold energy balance", [&, self]() {
        return self->Q.value_ - so_cold.molar_flow.value_ * (so_cold.molar_enthalpy.value_ - si_cold.molar_enthalpy.value_);
      });
    } else {
      // When Q is not fixed, we need:
      // 1. Energy balance: heat lost by hot equals heat gained by cold
      // 2. One equation to determine Q
      sys.AddEquation(name + ": energy balance", [&]() {
        double Q_hot = si_hot.molar_flow.value_ * (si_hot.molar_enthalpy.value_ - so_hot.molar_enthalpy.value_);
        double Q_cold = so_cold.molar_flow.value_ * (so_cold.molar_enthalpy.value_ - si_cold.molar_enthalpy.value_);
        return Q_hot - Q_cold;
      });
      sys.AddEquation(name + ": heat duty", [&, self]() {
        double Q_hot = si_hot.molar_flow.value_ * (si_hot.molar_enthalpy.value_ - so_hot.molar_enthalpy.value_);
        return self->Q.value_ - Q_hot;
      });
    }
    
    // Component mass balances: composition unchanged on each side
    // Hot side: x_in,i = x_out,i for each component i
    size_t num_components_hot = si_hot.mole_fractions.size();
    for (size_t i = 0; i < num_components_hot; ++i) {
      sys.AddEquation(name + ": hot_comp_balance[" + std::to_string(i) + "]", [&, i]() {
        return si_hot.mole_fractions[i].value_ - so_hot.mole_fractions[i].value_;
      });
    }
    
    // Cold side: x_in,i = x_out,i for each component i
    size_t num_components_cold = si_cold.mole_fractions.size();
    for (size_t i = 0; i < num_components_cold; ++i) {
      sys.AddEquation(name + ": cold_comp_balance[" + std::to_string(i) + "]", [&, i]() {
        return si_cold.mole_fractions[i].value_ - so_cold.mole_fractions[i].value_;
      });
    }

    // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
  }

  bool SimpleHeatExchanger::validate(const Flowsheet& fs, std::string* why) const {
    if (!in.valid())  { if (why) *why = "inlet not connected";  return false; }
    if (!out.valid()) { if (why) *why = "outlet not connected"; return false; }
    return true;
  }

  void SimpleHeatExchanger::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    reg.AddVariable(si.molar_flow);
    reg.AddVariable(so.molar_flow);
    reg.AddVariable(si.pressure);
    reg.AddVariable(so.pressure);
    reg.AddVariable(si.molar_enthalpy);
    reg.AddVariable(so.molar_enthalpy);
    reg.AddVariable(si.temperature);  // Needed for state equation: T = f(H, P)
    reg.AddVariable(so.temperature);  // Needed for state equation: T = f(H, P)
    reg.AddVariable(dP);
    reg.AddVariable(Q);
    
    // Register mole fractions for inlet and outlet
    for (auto& x : si.mole_fractions) {
      reg.AddVariable(x);
    }
    for (auto& x : so.mole_fractions) {
      reg.AddVariable(x);
    }
  }

  void SimpleHeatExchanger::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    sys.AddEquation(name + ": balance", [&](){ return si.molar_flow.value_ - so.molar_flow.value_; });
    sys.AddEquation(name + ": energy", [&, self](){ 
      // Q = m * (h_out - h_in), positive Q means heat added to stream
      return self->Q.value_ - so.molar_flow.value_ * (so.molar_enthalpy.value_ - si.molar_enthalpy.value_); 
    });
    sys.AddEquation(name + ": pressure_drop", [&, self](){ 
      return so.pressure.value_ - (si.pressure.value_ - self->dP.value_); 
    });
    
    // Component mass balances: composition unchanged (x_in = x_out for each component)
    size_t num_components = si.mole_fractions.size();
    for (size_t i = 0; i < num_components; ++i) {
      sys.AddEquation(name + ": comp_balance[" + std::to_string(i) + "]", [&, i]() {
        return si.mole_fractions[i].value_ - so.mole_fractions[i].value_;
      });
    }
    // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
  }

  bool Pump::validate(const Flowsheet& fs, std::string* why) const {
    if (!in.valid())  { if (why) *why = "inlet not connected";  return false; }
    if (!out.valid()) { if (why) *why = "outlet not connected"; return false; }
    return true;
  }

  void Pump::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    reg.AddVariable(si.molar_flow);
    reg.AddVariable(so.molar_flow);
    reg.AddVariable(si.pressure);
    reg.AddVariable(so.pressure);
    reg.AddVariable(si.molar_enthalpy);
    reg.AddVariable(so.molar_enthalpy);
    reg.AddVariable(si.temperature);  // Needed for state equation: T = f(H, P)
    reg.AddVariable(so.temperature);  // Needed for state equation: T = f(H, P)
    reg.AddVariable(dP);
    reg.AddVariable(W);
    reg.AddVariable(eta);
    
    // Register mole fractions for inlet and outlet (composition unchanged)
    for (auto& x : si.mole_fractions) {
      reg.AddVariable(x);
    }
    for (auto& x : so.mole_fractions) {
      reg.AddVariable(x);
    }
  }

  void Pump::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    sys.AddEquation(name + ": balance", [&](){ return si.molar_flow.value_ - so.molar_flow.value_; });
    
    // Pressure rise: P_out = P_in + dP (dP is positive for pumps)
    sys.AddEquation(name + ": pressure_rise", [&, self](){ 
      return so.pressure.value_ - (si.pressure.value_ + self->dP.value_); 
    });
    
    // Isentropic constraint: s_out = s_in (isentropic process)
    // This determines T_out from P_out and s_in, then H_out from (T_out, P_out)
    // Then W is determined from energy balance
    sys.AddEquation(name + ": isentropic", [&, self]() {
      try {
        // Calculate inlet entropy from inlet state (T_in, P_in)
        auto fluid_in = fs.fluids.GetFluidPackage(si.fluid_package_id);
        if (!fluid_in) {
          return 1e10;
        }
        
        // Set inlet composition if mixture
        auto components_in = fs.fluids.GetComponents(si.fluid_package_id);
        std::vector<double> z_in;
        if (components_in.size() > 1 && si.mole_fractions.size() == components_in.size()) {
          z_in.reserve(si.mole_fractions.size());
          for (const auto& mf : si.mole_fractions) {
            z_in.push_back(mf.value_);
          }
        }
        
        // Calculate inlet entropy
        if (!z_in.empty()) {
          fluid_in->set_mole_fractions(z_in);
        }
        fluid_in->update(CoolProp::PT_INPUTS, si.pressure.value_, si.temperature.value_);
        double s_in = fluid_in->smolar(); // Molar entropy at inlet
        
        // Prepare outlet mole fractions for temperature calculation
        auto components_out = fs.fluids.GetComponents(so.fluid_package_id);
        std::vector<double> z_out;
        if (components_out.size() > 1 && so.mole_fractions.size() == components_out.size()) {
          z_out.reserve(so.mole_fractions.size());
          for (const auto& mf : so.mole_fractions) {
            z_out.push_back(mf.value_);
          }
        }
        
        // Use FluidRegistry helper method to calculate isentropic outlet temperature
        double T_guess = so.temperature.value_;
        if (T_guess <= 0 || T_guess < 50.0) T_guess = si.temperature.value_;
        
        double T_out_isentropic = fs.fluids.CalculateTemperatureFromEntropyAndPressure(
          so.fluid_package_id,
          s_in,
          so.pressure.value_,
          T_guess,
          z_out
        );
        
        // Calculate H_out from the determined T_out and P_out
        auto fluid_out = fs.fluids.GetFluidPackage(so.fluid_package_id);
        if (!fluid_out) {
          return 1e10;
        }
        
        if (!z_out.empty()) {
          fluid_out->set_mole_fractions(z_out);
        }
        fluid_out->update(CoolProp::PT_INPUTS, so.pressure.value_, T_out_isentropic);
        double h_out_calculated = fluid_out->hmolar();
        
        // Return residual: H_out should equal the calculated value
        // This ensures H_out is consistent with the isentropic T_out
        // The state equation will then verify T_out is consistent with H_out and P_out
        return so.molar_enthalpy.value_ - h_out_calculated;
      } catch (const std::exception& e) {
        return 1e10;
      } catch (...) {
        return 1e10;
      }
    });
    
    // Energy balance: W * eta = m * (h_out - h_in)
    // This ensures the actual enthalpy change matches the work input
    sys.AddEquation(name + ": energy", [&, self](){ 
      double delta_h = so.molar_enthalpy.value_ - si.molar_enthalpy.value_;
      // Use inlet flow for consistency (mass balance ensures they're equal)
      return self->W.value_ * self->eta.value_ - si.molar_flow.value_ * delta_h; 
    });
    
    // Component mass balances: composition unchanged (x_in = x_out for each component)
    size_t num_components = si.mole_fractions.size();
    for (size_t i = 0; i < num_components; ++i) {
      sys.AddEquation(name + ": comp_balance[" + std::to_string(i) + "]", [&, i]() {
        return si.mole_fractions[i].value_ - so.mole_fractions[i].value_;
      });
    }
    // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
  }

  bool ComponentSplitter::validate(const Flowsheet& fs, std::string* why) const {
    if (!in.valid())  { if (why) *why = "inlet not connected";  return false; }
    if (!overhead.valid()) { if (why) *why = "overhead outlet not connected"; return false; }
    if (!bottom.valid()) { if (why) *why = "bottom outlet not connected"; return false; }
    return true;
  }

  void ComponentSplitter::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& si = fs.get<Stream>(in);
    auto& so_overhead = fs.get<Stream>(overhead);
    auto& so_bottom = fs.get<Stream>(bottom);

    // Initialize outlet streams' fluid packages to match inlet
    if (si.fluid_package_id != 0) {
      if (so_overhead.fluid_package_id != si.fluid_package_id) {
        so_overhead.fluid_package_id = si.fluid_package_id;
      }
      if (so_bottom.fluid_package_id != si.fluid_package_id) {
        so_bottom.fluid_package_id = si.fluid_package_id;
      }
    }

    // Initialize mole fractions for outlet streams based on inlet's fluid package
    if (si.fluid_package_id != 0) {
      auto components = fs.fluids.GetComponents(si.fluid_package_id);
      if (!components.empty()) {
        // Initialize overhead stream mole fractions
        if (so_overhead.mole_fractions.size() != components.size()) {
          so_overhead.initialize_composition(components.size(), components);
        }
        // Initialize bottom stream mole fractions
        if (so_bottom.mole_fractions.size() != components.size()) {
          so_bottom.initialize_composition(components.size(), components);
        }
        // Initialize split ratios if needed
        if (overhead_split_ratios.size() != components.size()) {
          overhead_split_ratios.resize(components.size());
          for (size_t i = 0; i < components.size(); ++i) {
            if (overhead_split_ratios[i].GetName().empty()) {
              overhead_split_ratios[i] = Var("split_ratio_" + components[i], 0.5, false);
            }
          }
        }
      }
    }

    reg.AddVariable(si.molar_flow);
    reg.AddVariable(so_overhead.molar_flow);
    reg.AddVariable(so_bottom.molar_flow);

    reg.AddVariable(si.pressure);
    reg.AddVariable(so_overhead.pressure);
    reg.AddVariable(so_bottom.pressure);

    reg.AddVariable(si.molar_enthalpy);
    reg.AddVariable(so_overhead.molar_enthalpy);
    reg.AddVariable(so_bottom.molar_enthalpy);

    reg.AddVariable(si.temperature);
    reg.AddVariable(so_overhead.temperature);
    reg.AddVariable(so_bottom.temperature);

    reg.AddVariable(Q);

    // Register split ratios (one per component)
    for (auto& ratio : overhead_split_ratios) {
      reg.AddVariable(ratio);
    }

    // Register mole fractions for inlet and outlets
    for (auto& x : si.mole_fractions) {
      reg.AddVariable(x);
    }
    for (auto& x : so_overhead.mole_fractions) {
      reg.AddVariable(x);
    }
    for (auto& x : so_bottom.mole_fractions) {
      reg.AddVariable(x);
    }
  }

  void ComponentSplitter::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so_overhead = fs.get<Stream>(overhead);
    auto& so_bottom = fs.get<Stream>(bottom);
    
    // Mass balance: m_in = m_overhead + m_bottom
    // sys.AddEquation(name + ": balance", [&](){ 
    //   return si.molar_flow.value_ - so_overhead.molar_flow.value_ - so_bottom.molar_flow.value_; 
    // });

    sys.AddEquation(name + ": overhead_mass_balance", [&, self]() {
      double Fi_in = 0.0;
      size_t num_components = si.mole_fractions.size();
      for (size_t i = 0; i < num_components; ++i) {
        Fi_in += si.mole_fractions[i].value_ * si.molar_flow.value_ * self->overhead_split_ratios[i].value_;
      }
      return Fi_in - so_overhead.molar_flow.value_;
    });

    sys.AddEquation(name + ": bottom_mass_balance", [&, self]() {
      double Fi_in = 0.0;
      size_t num_components = si.mole_fractions.size();
      for (size_t i = 0; i < num_components; ++i) {
        Fi_in += si.mole_fractions[i].value_ * si.molar_flow.value_ * (1.0 - self->overhead_split_ratios[i].value_);
      }
      return Fi_in - so_bottom.molar_flow.value_;
    });
    
    // Energy balance: Q = m_overhead * h_overhead + m_bottom * h_bottom - m_in * h_in
    sys.AddEquation(name + ": energy", [&, self](){ 
      return self->Q.value_ 
        - so_overhead.molar_flow.value_ * so_overhead.molar_enthalpy.value_
        - so_bottom.molar_flow.value_ * so_bottom.molar_enthalpy.value_
        + si.molar_flow.value_ * si.molar_enthalpy.value_; 
    });
    
    // Pressure drop: P_overhead = P_bottom
    sys.AddEquation(name + ": overhead_pressure_drop", [&](){ 
      return so_overhead.pressure.value_ - so_bottom.pressure.value_; 
    });

    // Pressure drop: P_bottom = P_in
    sys.AddEquation(name + ": bottom_pressure_drop", [&](){ 
      return so_bottom.pressure.value_ - si.pressure.value_; 
    });

    // Component mass balances: split ratios determine how each component is distributed
    // For each component i:
    //   - Overhead: n_in * x_in,i * split_ratio_i = n_overhead * x_overhead,i
    //   - Bottom: n_in * x_in,i * (1 - split_ratio_i) = n_bottom * x_bottom,i
    size_t num_components = si.mole_fractions.size();
    
    // Safety check: ensure all streams have the same number of components
    if (num_components == 0) {
      // If inlet has no components, skip component balance equations
      return;
    }
    
    // Ensure outlet streams and split ratios are properly sized
    if (so_overhead.mole_fractions.size() != num_components ||
        so_bottom.mole_fractions.size() != num_components ||
        self->overhead_split_ratios.size() != num_components) {
      // This should have been initialized in register_unknowns, but add a safety check
      return;
    }
    
    for (size_t i = 0; i < num_components; ++i) {
      sys.AddEquation(name + ": overhead_comp_balance[" + std::to_string(i) + "]", [&, i, self]() {
        return si.mole_fractions[i].value_ * si.molar_flow.value_ * self->overhead_split_ratios[i].value_ 
          - so_overhead.mole_fractions[i].value_ * so_overhead.molar_flow.value_;
      });

      sys.AddEquation(name + ": bottom_comp_balance[" + std::to_string(i) + "]", [&, i, self]() {
        return si.mole_fractions[i].value_ * si.molar_flow.value_ * (1.0 - self->overhead_split_ratios[i].value_) 
          - so_bottom.mole_fractions[i].value_ * so_bottom.molar_flow.value_;
      });
    
    }
    // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
  }

} // namespace px

CEREAL_REGISTER_TYPE(px::Valve)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::Valve)
CEREAL_CLASS_VERSION(px::Valve, 0)

CEREAL_REGISTER_TYPE(px::Mixer)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::Mixer)
CEREAL_CLASS_VERSION(px::Mixer, 0)

CEREAL_REGISTER_TYPE(px::Splitter)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::Splitter)
CEREAL_CLASS_VERSION(px::Splitter, 0)

CEREAL_REGISTER_TYPE(px::HeatExchanger)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::HeatExchanger)
CEREAL_CLASS_VERSION(px::HeatExchanger, 0)

CEREAL_REGISTER_TYPE(px::SimpleHeatExchanger)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::SimpleHeatExchanger)
CEREAL_CLASS_VERSION(px::SimpleHeatExchanger, 0)

CEREAL_REGISTER_TYPE(px::Pump)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::Pump)
CEREAL_CLASS_VERSION(px::Pump, 0)

CEREAL_REGISTER_TYPE(px::ComponentSplitter)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::ComponentSplitter)
CEREAL_CLASS_VERSION(px::ComponentSplitter, 0)
