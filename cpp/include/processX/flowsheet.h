#pragma once

#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// px includes
#include "processX/core.h"
#include "processX/solver.h"
#include "processX/stream.h"
#include "processX/unitop.h"

namespace px {

  // ---------- Flowsheet ----------
  struct Flowsheet {
    std::vector<Valve*> valves;
    UnknownsRegistry reg;
    ResidualSystem sys;

    bool assemble(std::string* err=nullptr){
      reg.clear(); sys.clear();
      for (auto* v: valves) {
        std::string why;
        if (!v->validate(&why)) { if (err) *err = "Valve invalid: " + why; return false; }
        v->register_unknowns(reg);
        v->add_equations(sys); // equations are always present
      }
      // Basic DOF
      if (reg.size() != sys.size()) {
        if (err) *err = "DOF mismatch: unknowns=" + std::to_string(reg.size()) +
                        " equations=" + std::to_string(sys.size());
        return false;
      }

      // Structural analysis BEFORE solving
      auto a = analyze_system(reg, sys, /*fd_rel=*/1e-6, /*fd_abs=*/1e-8);

      // Inconsistent equations (no dependence on any unknown, but nonzero residual)
      if (!a.inconsistent_eqs.empty()) {
        if (err) {
          *err = "Inconsistent equation(s) given chosen unknowns:\n";
          for (int idx : a.inconsistent_eqs)
            *err += "  - " + sys.names[(size_t)idx] + " (residual=" + std::to_string(a.r[(size_t)idx]) + ")\n";
        }
        return false;
      }

      // Redundant equations (identities w.r.t. unknowns)
      if (!a.redundant_eqs.empty()) {
        if (err) {
          *err = "Redundant equation(s) detected (independent of unknowns and already satisfied):\n";
          for (int idx : a.redundant_eqs)
            *err += "  - " + sys.names[(size_t)idx] + "\n";
          *err += "Result: Jacobian rank " + std::to_string(a.rank) +
                  " < " + std::to_string(reg.size()) + " (rank-deficient). Adjust unknowns.";
        }
        return false;
      }

      // Rank deficiency (e.g., two different equations but linearly dependent)
      if (a.rank < (int)reg.size()) {
        if (err) {
          *err = "System is rank-deficient: rank(J)=" + std::to_string(a.rank) +
                " < unknowns=" + std::to_string(reg.size()) + ".";
          // Optional: list unconstrained unknowns
          if (!a.unconstrained_unknowns.empty()) {
            *err += "\nUnconstrained unknown(s):";
            for (int j : a.unconstrained_unknowns)
              *err += " " + reg.vars[(size_t)j]->name;
          }
        }
        return false;
      }

      return true; // structurally OK
    }

    NewtonReport solve(const NewtonOptions& opt={}) {
      return newton_solve(reg, sys, opt);
    }

    static void print_valve(const Valve& v){
      auto pf=[&](const Var& x){ return x.fixed?"(fixed)":"(free)"; };
      std::cout<<"  inlet.F = "<<v.inlet->molar_flow.value<<" "<<pf(v.inlet->molar_flow)<<"\n";
      std::cout<<"  outlet.F = "<<v.outlet->molar_flow.value<<" "<<pf(v.outlet->molar_flow)<<"\n";
      std::cout<<"  inlet.P = "<<v.inlet->pressure.value<<" "<<pf(v.inlet->pressure)<<"\n";
      std::cout<<"  outlet.P = "<<v.outlet->pressure.value<<" "<<pf(v.outlet->pressure)<<"\n";
      std::cout<<"  Cv = "<<v.Cv.value<<" "<<pf(v.Cv)<<"\n";
    }
  };

  // ---------- Demo ----------
  static void run_case(const std::string& title,
                      std::function<void(Valve&, Stream&, Stream&)> config){
    std::cout<<"\n=== "<<title<<" ===\n";
    Stream in;  in.molar_flow.name="in.F";  in.pressure.name="in.P";
    Stream out; out.molar_flow.name="out.F"; out.pressure.name="out.P";
    Valve v; v.inlet=&in; v.outlet=&out; v.Cv.name="Cv";

    // User config (choose unknowns by marking fixed=false)
    config(v, in, out);

    Flowsheet fs; fs.valves.push_back(&v);
    std::string err;
    if(!fs.assemble(&err)){ std::cerr<<"Assemble error: "<<err<<"\n"; Flowsheet::print_valve(v); return; }

    std::cout<<"Unknowns: "<<fs.reg.unknowns_list()<<"\n";
    auto rep = fs.solve(NewtonOptions{50,1e-12,1e-14,1e-6,1e-8,true});
    std::cout<<(rep.converged?"Converged":"Not converged")<<" in "<<rep.iters<<" iters, |r|_inf="<<rep.final_res<<" : "<<rep.msg<<"\n";
    Flowsheet::print_valve(v);
  }

} // end px namespace
