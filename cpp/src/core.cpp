#include "processX/core.h"

namespace px {

  void UnknownsRegistry::AddVariable(Var& v) {
    // Variable is fixes so don't register it
    if (v.is_fixed_) return; 

    for (auto* p : vars_) {
      if (p == &v) return; // Already registered
    }

    // Not registered, so add
    vars_.push_back(&v);
  }

  std::vector<double> UnknownsRegistry::PackVariables() const { 
    std::vector<double> x; 
    x.reserve(vars_.size()); 

    for (auto* v: vars_) {
      x.push_back(v->value_); 
    } 
    
    return x; 
  }

  void UnknownsRegistry::PackVariables(sunrealtype* x, size_t n) const {
    assert(x);
    assert(n >= vars_.size());

    for (size_t i = 0; i < vars_.size(); ++i) {
      x[i] = vars_[i]->value_;
    }
  }

  void UnknownsRegistry::UnpackVariables(const std::vector<double>& x) { 
    assert(x.size() == vars_.size()); 

    for (size_t i = 0; i < vars_.size(); ++i) {
      vars_[i]->value_ = x[i]; 
    }
  }

  void UnknownsRegistry::UnpackVariables(const sunrealtype* x, size_t n) {
    assert(x);
    assert(n >= vars_.size());

    for (size_t i = 0; i < vars_.size(); ++i) {
      vars_[i]->value_ = static_cast<double>(x[i]); 
    }
  }
  
  std::string UnknownsRegistry::GetFeeVariableNames() const { 
    std::string s; 
    for (size_t i = 0; i < vars_.size(); ++i) { 
      if(i) { // Skip the first one
        s+=", "; 
      }

      s += vars_[i]->GetName();
    } 
    return s.empty() ? "(none)" : s; // Return (none) if empty 
  }

  void ResidualSystem::AddEquation(std::string eq_name, std::function<double()> res_equation){ 
    eq_names_.push_back(std::move(eq_name)); 
    res_equations_.push_back(std::move(res_equation)); 
  }
  
}

CEREAL_CLASS_VERSION(px::Var, 0)
