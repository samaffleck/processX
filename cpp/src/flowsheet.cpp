#include "processX/flowsheet.h"


namespace px {

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

    // DOF check
    if (reg.size() != sys.size()) {
      if (err) *err = "DOF mismatch: unknowns=" + std::to_string(reg.size())
                    + " equations=" + std::to_string(sys.size());
      return false;
    }

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
    if (!a.redundant_eqs.empty()) {
      if (err) {
        *err = "Redundant equations:\n";
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
