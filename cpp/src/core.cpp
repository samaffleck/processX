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

  void UnknownsRegistry::UnpackVariables(const std::vector<double>& x) { 
    assert(x.size() == vars_.size()); 

    for (size_t i = 0; i < vars_.size(); ++i) {
      vars_[i]->value_ = x[i]; 
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
    
  std::vector<double> ResidualSystem::EvaluateResiduals() const { 
    std::vector<double> residuals; 
    residuals.reserve(res_equations_.size()); 

    for (auto& f: res_equations_) {
      // Calls the residual equation and adds the result to the vector
      residuals.push_back( f() );
    }

    return residuals; 
  }
  
}

CEREAL_CLASS_VERSION(px::Var, 0)
