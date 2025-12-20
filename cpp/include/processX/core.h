// include/processX/core.h
#pragma once

// STL includes
#include <vector>
#include <string>
#include <cassert>
#include <functional>
#include <cstddef>
#include <algorithm>

// Cereal includes
#include <cereal/cereal.hpp>

// SUNDIALS includes
#include <sundials/sundials_core.h>


namespace px {

  // Forward declarations
  class SystemAnalysis;
  class ResidualSystem;
  
  class Var {
  public:
    Var() = default;
    Var(std::string name, double value, bool is_fixed=false) : 
      value_(value), is_fixed_(is_fixed), name_(std::move(name)) {}
    
    // Getters
    const std::string& GetName() const noexcept { return name_; }
    
    // Setters
    void SetValue(double value, bool is_fixed) { value_ = value; is_fixed_ = is_fixed; }

    double value_{0.0};
    bool is_fixed_{false};
  
  private:
    std::string name_;

    friend class cereal::access;
    template<class Archive>
    void serialize(Archive& ar, std::uint32_t /*version*/) {
      ar(
        cereal::make_nvp("Variable_Name",  name_),
        cereal::make_nvp("Variable_Value", value_),
        cereal::make_nvp("Variable_Is_Fixed", is_fixed_)
      );
    }
  };

  // Registry with a vector of Variable pointers to each free variable
  // in the system. 
  class UnknownsRegistry {
  public:    
    void Clear() { vars_.clear(); }
    std::size_t GetNumberOfUnknowns() const { return vars_.size(); }    
    void AddVariable(Var& v);
    
    // Loops through free variables and returns a vector of values
    std::vector<double> PackVariables() const;
    void PackVariables(sunrealtype* x, size_t n) const;
    
    // Assigns all of the free variables with updates values
    void UnpackVariables(const std::vector<double>& x);
    void UnpackVariables(const sunrealtype* x, size_t n);
    
    // Returns string of all free variable names
    std::string GetFeeVariableNames() const;
    
    // Get variable value by index
    // Returns the current value, or 0.0 if index is out of bounds
    double GetVariableValue(size_t index) const {
      if (index >= vars_.size()) return 0.0;
      return vars_[index]->value_;
    }
    
    // Set variable value by index (for finite difference computation)
    // Returns true if index is valid, false otherwise
    bool SetVariableValue(size_t index, double value) {
      if (index >= vars_.size()) return false;
      vars_[index]->value_ = value;
      return true;
    }

  private:
    std::vector<Var*> vars_{};

  };

  // Registry for all of the residual functions used to update our
  // vector of unknowns
  class ResidualSystem {
  public:
    void Clear(){ res_equations_.clear(); eq_names_.clear(); }
    std::size_t GetNumberOfEquations() const { return res_equations_.size(); }
    std::vector<std::string> GetEquationNames() const { return eq_names_; }

    // Add an equation that calculates the residual 
    void AddEquation(std::string eq_name, std::function<double()> res_equation);
    
    // Evaluate residuals in buffer
    template<typename T>
    void EvaluateResiduals(T* dest, size_t n) const {
      assert(n > 0);
      size_t count = std::min(n, res_equations_.size());
      for (size_t i = 0; i < count; ++i) {
        dest[i] = static_cast<T>(res_equations_[i]());
      }
    }

  private:
    std::vector<std::function<double()>> res_equations_;
    std::vector<std::string> eq_names_;
  
  };

} // end px namespace
