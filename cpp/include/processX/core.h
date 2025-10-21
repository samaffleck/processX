// include/processX/core.h
#pragma once

// STL includes
#include <vector>
#include <string>
#include <cassert>
#include <functional>


namespace px {

  struct Var {
    std::string name;
    double value{0.0};
    bool fixed{false};
    Var() = default;
    Var(std::string n, double v, bool f=false) : name(std::move(n)), value(v), fixed(f) {}
  };

  struct UnknownsRegistry {
    std::vector<Var*> vars;
    
    void clear() { vars.clear(); }
    
    void register_var(Var& v) { if (!v.fixed) vars.push_back(&v); }
    
    std::size_t size() const { return vars.size(); }
    
    std::vector<double> pack_x() const { 
      std::vector<double> x; 
      x.reserve(vars.size()); 
      for (auto* v: vars) {
        x.push_back(v->value); 
      } 
      return x; 
    }
    
    void scatter_x(const std::vector<double>& x){ 
      assert(x.size() == vars.size()); 
      for (size_t i = 0; i < vars.size(); ++i) {
        vars[i]->value = x[i]; 
      }
    }
    
    std::string unknowns_list() const { 
      std::string s; 
      for (size_t i = 0; i < vars.size(); ++i) { 
        if(i) {
          s+=", "; 
        }
        s+=vars[i]->name;
      } 
      return s.empty()?"(none)":s; 
    }
  };

  struct ResidualSystem {
    std::vector<std::function<double()>> residuals;
    std::vector<std::string> names;

    void clear(){ 
      residuals.clear(); 
      names.clear(); 
    }
    
    void add(std::string nm, std::function<double()> r){ 
      names.push_back(std::move(nm)); 
      residuals.push_back(std::move(r)); 
    }
    
    std::size_t size() const { return residuals.size(); }
    
    std::vector<double> eval() const { 
      std::vector<double> r; 
      r.reserve(residuals.size()); 
      for (auto& f: residuals) {
        r.push_back(f()); 
      }
      return r; 
    }
    
    static double norm_inf(const std::vector<double>& r){ 
      double m = 0; 
      for(double v:r) {
        m = std::max(m, std::abs(v)); 
      }
      return m; 
    }
  };

}
