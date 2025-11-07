#include "processX/flowsheet.h"

// CoolProp includes
#include <CoolProp.h>
#include <AbstractState.h>
#include <unordered_set>

namespace px {

  // Helper function to enforce state equation: T = f(H, P) or H = f(T, P)
  // Returns residual: 0 when state equation is satisfied
  // Requires: P must be fixed, and at least one of (T, H) must be fixed
  static double state_equation_residual(
    Stream& stream,
    std::shared_ptr<CoolProp::AbstractState>& fluid
  ) {    
    if (stream.temperature.fixed) {
      // T and P are fixed -> calculate H and compare
      fluid->update(CoolProp::PT_INPUTS, stream.pressure.value, stream.temperature.value);
      double calculated_h = fluid->hmolar();
      return stream.molar_enthalpy.value - calculated_h;
    }
    // H and P are fixed -> calculate T and compare
    fluid->update(CoolProp::HmolarP_INPUTS, stream.molar_enthalpy.value, stream.pressure.value);
    double calculated_t = fluid->T();
    return stream.temperature.value - calculated_t;
  }

  bool Flowsheet::assemble(std::string* err) {
    units_.clear();
    build_unit_list();

    reg.clear(); sys.clear();

    // Validate units
    for (auto* u : units_) {
      std::string why;
      if (!u->validate(*this, &why)) {
        if (err) *err = std::string(u->type_name()) + " '" + u->get_name() + "' invalid: " + why;
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
      if (!s.molar_enthalpy.fixed && 
          (s.molar_enthalpy.value == 0.0 || 
           s.molar_enthalpy.value < -1e6 || 
           s.molar_enthalpy.value > 1e6)) {
        // Check if we have reasonable T and P values to calculate H from
        if (s.temperature.value > 0.0 && s.temperature.value < 10000.0 &&
            s.pressure.value > 100.0 && s.pressure.value < 1e10) {
          try {
            fluid->update(CoolProp::PT_INPUTS, s.pressure.value, s.temperature.value);
            s.molar_enthalpy.value = fluid->hmolar();
          } catch (...) {
            // If CoolProp fails, leave enthalpy as is (will be set by solver)
          }
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
          return state_equation_residual(s, this->fluid);
        });
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
