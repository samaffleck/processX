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
    reg.register_var(si.molar_flow);
    reg.register_var(so.molar_flow);
    reg.register_var(si.pressure);
    reg.register_var(so.pressure);
    reg.register_var(si.molar_enthalpy);
    reg.register_var(so.molar_enthalpy);
    reg.register_var(si.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(so.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(Cv);
    
    // Register mole fractions for inlet and outlet
    for (auto& x : si.mole_fractions) {
      reg.register_var(x);
    }
    for (auto& x : so.mole_fractions) {
      reg.register_var(x);
    }
  }

  void Valve::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    sys.add(name + ": balance", [&](){ return si.molar_flow.value - so.molar_flow.value; });
    sys.add(name + ": PF",      [&, self](){ return so.molar_flow.value - self->Cv.value * (si.pressure.value - so.pressure.value); });
    sys.add(name + ": h_equal", [&](){ return si.molar_enthalpy.value - so.molar_enthalpy.value; });
    
    // Component mass balances: composition unchanged (x_in = x_out for each component)
    size_t num_components = si.mole_fractions.size();
    for (size_t i = 0; i < num_components; ++i) {
      sys.add(name + ": comp_balance[" + std::to_string(i) + "]", [&, i]() {
        return si.mole_fractions[i].value - so.mole_fractions[i].value;
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
    reg.register_var(so.molar_flow);
    reg.register_var(so.pressure);
    reg.register_var(so.molar_enthalpy);
    reg.register_var(so.temperature);  // Needed for state equation: T = f(H, P)
    
    // Register mole fractions for outlet
    for (auto& x : so.mole_fractions) {
      reg.register_var(x);
    }

    for (auto h : in) {
      auto& si = fs.get<Stream>(h);
      reg.register_var(si.molar_flow);
      reg.register_var(si.pressure);
      reg.register_var(si.molar_enthalpy);
      reg.register_var(si.temperature);  // Needed for state equation: T = f(H, P)
      
      // Register mole fractions for each inlet
      for (auto& x : si.mole_fractions) {
        reg.register_var(x);
      }
    }
  }

  void Mixer::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto& so = fs.get<Stream>(out);

    sys.add(name + ": total balance", [&]() {
      double sum_in = 0.0;
      for (auto h : in) sum_in += fs.get<Stream>(h).molar_flow.value;
      return sum_in - so.molar_flow.value;
    });

    sys.add(name + ": energy balance", [&]() {
      double sum_in = 0.0;
      for (auto h : in) {
        auto& s = fs.get<Stream>(h);
        sum_in += s.molar_flow.value * s.molar_enthalpy.value;
      }
      return sum_in - so.molar_flow.value * so.molar_enthalpy.value;
    });

    for (size_t i = 0; i < in.size(); ++i) {
      auto& si = fs.get<Stream>(in[i]);
      sys.add(name + ": p_equal[" + std::to_string(i) + "]", [&si, &so]() {
        return si.pressure.value - so.pressure.value;
      });
      // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
    }
    
    // Component mass balances: Σ(n_in * x_in,i) = n_out * x_out,i for each component i
    size_t num_components = so.mole_fractions.size();
    for (size_t comp_idx = 0; comp_idx < num_components; ++comp_idx) {
      sys.add(name + ": comp_balance[" + std::to_string(comp_idx) + "]", [&, comp_idx]() {
        double sum_in = 0.0;
        for (auto h : in) {
          auto& s = fs.get<Stream>(h);
          if (comp_idx < s.mole_fractions.size()) {
            sum_in += s.molar_flow.value * s.mole_fractions[comp_idx].value;
          }
        }
        return sum_in - so.molar_flow.value * so.mole_fractions[comp_idx].value;
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
    reg.register_var(si.molar_flow);
    reg.register_var(si.pressure);
    reg.register_var(si.molar_enthalpy);
    reg.register_var(si.temperature);  // Needed for state equation: T = f(H, P)
    
    // Register mole fractions for inlet
    for (auto& x : si.mole_fractions) {
      reg.register_var(x);
    }

    for (auto h : out) {
      auto& so = fs.get<Stream>(h);
      reg.register_var(so.molar_flow);
      reg.register_var(so.pressure);
      reg.register_var(so.molar_enthalpy);
      reg.register_var(so.temperature);  // Needed for state equation: T = f(H, P)
      
      // Register mole fractions for each outlet
      for (auto& x : so.mole_fractions) {
        reg.register_var(x);
      }
    }
  }

  void Splitter::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto& si = fs.get<Stream>(in);

    sys.add(name + ": total balance", [&]() {
      double sum_out = 0.0;
      for (auto h : out) sum_out += fs.get<Stream>(h).molar_flow.value;
      return si.molar_flow.value - sum_out;
    });

    sys.add(name + ": energy balance", [&]() {
      double sum_out = 0.0;
      for (auto h : out) {
        auto& s = fs.get<Stream>(h);
        sum_out += s.molar_flow.value * s.molar_enthalpy.value;
      }
      return si.molar_flow.value * si.molar_enthalpy.value - sum_out;
    });

    for (size_t i = 0; i < out.size(); ++i) {
      auto& so = fs.get<Stream>(out[i]);
      sys.add(name + ": p_equal[" + std::to_string(i) + "]", [&si, &so]() {
        return si.pressure.value - so.pressure.value;
      });
      sys.add(name + ": h_equal[" + std::to_string(i) + "]", [&si, &so]() {
        return si.molar_enthalpy.value - so.molar_enthalpy.value;
      });
      // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
    }
    
    // Component mass balances: n_in * x_in,i = Σ(n_out * x_out,i) for each component i
    // Equivalently: x_in,i = x_out,i for all outlets (composition unchanged)
    size_t num_components = si.mole_fractions.size();
    for (size_t comp_idx = 0; comp_idx < num_components; ++comp_idx) {
      for (size_t out_idx = 0; out_idx < out.size(); ++out_idx) {
        auto& so = fs.get<Stream>(out[out_idx]);
        sys.add(name + ": comp_balance[" + std::to_string(comp_idx) + "][" + std::to_string(out_idx) + "]", [&, comp_idx, out_idx]() {
          if (comp_idx < si.mole_fractions.size() && comp_idx < so.mole_fractions.size()) {
            return si.mole_fractions[comp_idx].value - so.mole_fractions[comp_idx].value;
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
    reg.register_var(si_hot.molar_flow);
    reg.register_var(so_hot.molar_flow);
    reg.register_var(si_hot.pressure);
    reg.register_var(so_hot.pressure);
    reg.register_var(si_hot.molar_enthalpy);
    reg.register_var(so_hot.molar_enthalpy);
    reg.register_var(si_hot.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(so_hot.temperature);  // Needed for state equation: T = f(H, P)
    
    // Register mole fractions for hot side
    for (auto& x : si_hot.mole_fractions) {
      reg.register_var(x);
    }
    for (auto& x : so_hot.mole_fractions) {
      reg.register_var(x);
    }

    // Cold side
    reg.register_var(si_cold.molar_flow);
    reg.register_var(so_cold.molar_flow);
    reg.register_var(si_cold.pressure);
    reg.register_var(so_cold.pressure);
    reg.register_var(si_cold.molar_enthalpy);
    reg.register_var(so_cold.molar_enthalpy);
    reg.register_var(si_cold.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(so_cold.temperature);  // Needed for state equation: T = f(H, P)
    
    // Register mole fractions for cold side
    for (auto& x : si_cold.mole_fractions) {
      reg.register_var(x);
    }
    for (auto& x : so_cold.mole_fractions) {
      reg.register_var(x);
    }

    // Heat exchanger parameters
    reg.register_var(dP_hot);
    reg.register_var(dP_cold);
    reg.register_var(Q);
  }

  void HeatExchanger::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto& si_hot = fs.get<Stream>(in_hot);
    auto& so_hot = fs.get<Stream>(out_hot);
    auto& si_cold = fs.get<Stream>(in_cold);
    auto& so_cold = fs.get<Stream>(out_cold);
    auto* self = this;

    // Mass balance - hot side
    sys.add(name + ": hot mass balance", [&]() {
      return si_hot.molar_flow.value - so_hot.molar_flow.value;
    });

    // Mass balance - cold side
    sys.add(name + ": cold mass balance", [&]() {
      return si_cold.molar_flow.value - so_cold.molar_flow.value;
    });

    // Pressure drop - hot side: P_out = P_in - dP
    sys.add(name + ": hot pressure drop", [&, self]() {
      return so_hot.pressure.value - (si_hot.pressure.value - self->dP_hot.value);
    });

    // Pressure drop - cold side: P_out = P_in - dP
    sys.add(name + ": cold pressure drop", [&, self]() {
      return so_cold.pressure.value - (si_cold.pressure.value - self->dP_cold.value);
    });

    // Energy balance equations
    if (self->Q.fixed) {
      // When Q is fixed, we have two independent equations:
      // Q = m_hot * (h_hot_in - h_hot_out) - constrains hot side
      // Q = m_cold * (h_cold_out - h_cold_in) - constrains cold side
      sys.add(name + ": hot energy balance", [&, self]() {
        return self->Q.value - si_hot.molar_flow.value * (si_hot.molar_enthalpy.value - so_hot.molar_enthalpy.value);
      });
      sys.add(name + ": cold energy balance", [&, self]() {
        return self->Q.value - so_cold.molar_flow.value * (so_cold.molar_enthalpy.value - si_cold.molar_enthalpy.value);
      });
    } else {
      // When Q is not fixed, we need:
      // 1. Energy balance: heat lost by hot equals heat gained by cold
      // 2. One equation to determine Q
      sys.add(name + ": energy balance", [&]() {
        double Q_hot = si_hot.molar_flow.value * (si_hot.molar_enthalpy.value - so_hot.molar_enthalpy.value);
        double Q_cold = so_cold.molar_flow.value * (so_cold.molar_enthalpy.value - si_cold.molar_enthalpy.value);
        return Q_hot - Q_cold;
      });
      sys.add(name + ": heat duty", [&, self]() {
        double Q_hot = si_hot.molar_flow.value * (si_hot.molar_enthalpy.value - so_hot.molar_enthalpy.value);
        return self->Q.value - Q_hot;
      });
    }
    
    // Component mass balances: composition unchanged on each side
    // Hot side: x_in,i = x_out,i for each component i
    size_t num_components_hot = si_hot.mole_fractions.size();
    for (size_t i = 0; i < num_components_hot; ++i) {
      sys.add(name + ": hot_comp_balance[" + std::to_string(i) + "]", [&, i]() {
        return si_hot.mole_fractions[i].value - so_hot.mole_fractions[i].value;
      });
    }
    
    // Cold side: x_in,i = x_out,i for each component i
    size_t num_components_cold = si_cold.mole_fractions.size();
    for (size_t i = 0; i < num_components_cold; ++i) {
      sys.add(name + ": cold_comp_balance[" + std::to_string(i) + "]", [&, i]() {
        return si_cold.mole_fractions[i].value - so_cold.mole_fractions[i].value;
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
    reg.register_var(si.molar_flow);
    reg.register_var(so.molar_flow);
    reg.register_var(si.pressure);
    reg.register_var(so.pressure);
    reg.register_var(si.molar_enthalpy);
    reg.register_var(so.molar_enthalpy);
    reg.register_var(si.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(so.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(dP);
    reg.register_var(Q);
    
    // Register mole fractions for inlet and outlet
    for (auto& x : si.mole_fractions) {
      reg.register_var(x);
    }
    for (auto& x : so.mole_fractions) {
      reg.register_var(x);
    }
  }

  void SimpleHeatExchanger::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    sys.add(name + ": balance", [&](){ return si.molar_flow.value - so.molar_flow.value; });
    sys.add(name + ": energy", [&, self](){ 
      // Q = m * (h_out - h_in), positive Q means heat added to stream
      return self->Q.value - so.molar_flow.value * (so.molar_enthalpy.value - si.molar_enthalpy.value); 
    });
    sys.add(name + ": pressure_drop", [&, self](){ 
      return so.pressure.value - (si.pressure.value - self->dP.value); 
    });
    
    // Component mass balances: composition unchanged (x_in = x_out for each component)
    size_t num_components = si.mole_fractions.size();
    for (size_t i = 0; i < num_components; ++i) {
      sys.add(name + ": comp_balance[" + std::to_string(i) + "]", [&, i]() {
        return si.mole_fractions[i].value - so.mole_fractions[i].value;
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
    reg.register_var(si.molar_flow);
    reg.register_var(so.molar_flow);
    reg.register_var(si.pressure);
    reg.register_var(so.pressure);
    reg.register_var(si.molar_enthalpy);
    reg.register_var(so.molar_enthalpy);
    reg.register_var(si.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(so.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(dP);
    reg.register_var(W);
    reg.register_var(eta);
    
    // Register mole fractions for inlet and outlet (composition unchanged)
    for (auto& x : si.mole_fractions) {
      reg.register_var(x);
    }
    for (auto& x : so.mole_fractions) {
      reg.register_var(x);
    }
  }

  void Pump::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    sys.add(name + ": balance", [&](){ return si.molar_flow.value - so.molar_flow.value; });
    
    // Pressure rise: P_out = P_in + dP (dP is positive for pumps)
    sys.add(name + ": pressure_rise", [&, self](){ 
      return so.pressure.value - (si.pressure.value + self->dP.value); 
    });
    
    // Isentropic constraint: s_out = s_in (isentropic process)
    // This determines T_out from P_out and s_in, then H_out from (T_out, P_out)
    // Then W is determined from energy balance
    sys.add(name + ": isentropic", [&, self]() {
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
            z_in.push_back(mf.value);
          }
        }
        
        // Calculate inlet entropy
        if (!z_in.empty()) {
          fluid_in->set_mole_fractions(z_in);
        }
        fluid_in->update(CoolProp::PT_INPUTS, si.pressure.value, si.temperature.value);
        double s_in = fluid_in->smolar(); // Molar entropy at inlet
        
        // Prepare outlet mole fractions for temperature calculation
        auto components_out = fs.fluids.GetComponents(so.fluid_package_id);
        std::vector<double> z_out;
        if (components_out.size() > 1 && so.mole_fractions.size() == components_out.size()) {
          z_out.reserve(so.mole_fractions.size());
          for (const auto& mf : so.mole_fractions) {
            z_out.push_back(mf.value);
          }
        }
        
        // Use FluidRegistry helper method to calculate isentropic outlet temperature
        double T_guess = so.temperature.value;
        if (T_guess <= 0 || T_guess < 50.0) T_guess = si.temperature.value;
        
        double T_out_isentropic = fs.fluids.CalculateTemperatureFromEntropyAndPressure(
          so.fluid_package_id,
          s_in,
          so.pressure.value,
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
        fluid_out->update(CoolProp::PT_INPUTS, so.pressure.value, T_out_isentropic);
        double h_out_calculated = fluid_out->hmolar();
        
        // Return residual: H_out should equal the calculated value
        // This ensures H_out is consistent with the isentropic T_out
        // The state equation will then verify T_out is consistent with H_out and P_out
        return so.molar_enthalpy.value - h_out_calculated;
      } catch (const std::exception& e) {
        return 1e10;
      } catch (...) {
        return 1e10;
      }
    });
    
    // Energy balance: W * eta = m * (h_out - h_in)
    // This ensures the actual enthalpy change matches the work input
    sys.add(name + ": energy", [&, self](){ 
      double delta_h = so.molar_enthalpy.value - si.molar_enthalpy.value;
      // Use inlet flow for consistency (mass balance ensures they're equal)
      return self->W.value * self->eta.value - si.molar_flow.value * delta_h; 
    });
    
    // Component mass balances: composition unchanged (x_in = x_out for each component)
    size_t num_components = si.mole_fractions.size();
    for (size_t i = 0; i < num_components; ++i) {
      sys.add(name + ": comp_balance[" + std::to_string(i) + "]", [&, i]() {
        return si.mole_fractions[i].value - so.mole_fractions[i].value;
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
