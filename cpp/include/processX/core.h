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


namespace px {

  // Forward declarations
  struct SystemAnalysis;
  class ResidualSystem;
  
  // Forward declaration for SUNDIALS types (to avoid including SUNDIALS headers here)
  // sunrealtype is typically double, but we use a template approach for flexibility

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
    // Removes all variables
    void Clear() { vars_.clear(); }

    // Returns the number of unknowns/free variables in the registry
    std::size_t GetNumberOfUnknowns() const { return vars_.size(); }
    
    // Adds a variable to the registry
    void AddVariable(Var& v);
    
    // Loops through free variables and returns a vector of values
    std::vector<double> PackVariables() const;
    
    // Pack variables directly into a buffer (avoids std::vector copy)
    // dest must have space for at least GetNumberOfUnknowns() elements
    template<typename T>
    void PackVariables(T* dest, size_t n) const {
      assert(n >= vars_.size());
      for (size_t i = 0; i < vars_.size(); ++i) {
        dest[i] = static_cast<T>(vars_[i]->value_);
      }
    }
    
    // Assigns all of the free variables with updates values
    void UnpackVariables(const std::vector<double>& x);
    
    // Unpack variables directly from a buffer (avoids std::vector copy)
    // src must have at least GetNumberOfUnknowns() elements
    template<typename T>
    void UnpackVariables(const T* src, size_t n) {
      assert(n >= vars_.size());
      for (size_t i = 0; i < vars_.size(); ++i) {
        vars_[i]->value_ = static_cast<double>(src[i]);
      }
    }
    
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
    
    // Loops through all residual equations and returns a vector of the residuals
    std::vector<double> EvaluateResiduals() const;
    
    // Evaluate residuals directly into a buffer (avoids std::vector copy)
    // dest must have space for at least n elements
    // n can be less than GetNumberOfEquations() to write only the first n residuals
    template<typename T>
    void EvaluateResiduals(T* dest, size_t n) const {
      assert(n > 0);
      size_t count = std::min(n, res_equations_.size());
      for (size_t i = 0; i < count; ++i) {
        dest[i] = static_cast<T>(res_equations_[i]());
      }
    }
    
    // TODO: check if this is used anywhere...
    static double norm_inf(const std::vector<double>& r) { 
      double m = 0; 
      for(double v : r) {
        m = std::max(m, std::abs(v)); 
      }
      return m; 
    }

  private:
    std::vector<std::function<double()>> res_equations_;
    std::vector<std::string> eq_names_;
  
  };

}
