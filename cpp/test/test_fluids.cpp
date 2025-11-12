// Test file to validate CoolProp component-backend combinations
// Based on CoolProp's supported fluids: https://coolprop.org/fluid_properties/PurePseudoPure.html

#include <gtest/gtest.h>
#include <CoolProp.h>
#include <AbstractState.h>
#include <processX/fluids.h>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <utility>

namespace px {

  class FluidValidationTest : public ::testing::Test {
  protected:
    void SetUp() override {
      // Track valid combinations
      valid_combinations.clear();
      invalid_combinations.clear();
    }

    // Test if a component-backend combination is valid
    bool test_component_backend(const std::string& component, const std::string& backend) {
      try {
        // Try to create a fluid package with this component and backend
        std::shared_ptr<CoolProp::AbstractState> state;
        
        // For pure fluids, try to create the state
        if (backend == "IF97") {
          // IF97 only works with Water
          if (component != "Water") {
            return false;
          }
        } 
        try {
          state = std::shared_ptr<CoolProp::AbstractState>(
            CoolProp::AbstractState::factory(backend, component)
          );
        } catch (...) {
          // Update failed, combination might be invalid
          return false;
        }
        
        // Try to set a simple state (P, T) to verify it works
        double P = 101325.0; // 1 atm
        double T = 300.0;    // 300 K
        
        try {
          state->update(CoolProp::PT_INPUTS, P, T);
          // If we get here, the combination is valid
          return true;
        } catch (...) {
          // Update failed, combination might be invalid
          return false;
        }
      } catch (const std::exception& e) {
        // Exception during creation, combination is invalid
        return false;
      } catch (...) {
        // Unknown exception, assume invalid
        return false;
      }
    }

    std::set<std::pair<std::string, std::string>> valid_combinations;
    std::set<std::pair<std::string, std::string>> invalid_combinations;
  };

  // Test all component-backend combinations
  TEST_F(FluidValidationTest, ValidateAllCombinations) {
    std::cout << "\n=== Validating Component-Backend Combinations ===\n" << std::endl;
    
    int total_tested = 0;
    int total_valid = 0;
    int total_invalid = 0;
    
    // Test each component with each backend
    for (int comp_idx = 0; comp_idx < px::kNumberOfComponents; ++comp_idx) {
      std::string component(px::component_names[comp_idx]);
      
      for (int backend_idx = 0; backend_idx < px::kNumberOfBackends; ++backend_idx) {
        std::string backend(px::backend_names[backend_idx]);
        
        total_tested++;
        bool is_valid = test_component_backend(component, backend);
        
        if (is_valid) {
          valid_combinations.insert({component, backend});
          total_valid++;
        } else {
          invalid_combinations.insert({component, backend});
          total_invalid++;
        }
      }
    }
    
    // Print summary
    std::cout << "\n=== Validation Summary ===" << std::endl;
    std::cout << "Total combinations tested: " << total_tested << std::endl;
    std::cout << "Valid combinations: " << total_valid << std::endl;
    std::cout << "Invalid combinations: " << total_invalid << std::endl;
    
    std::cout << "\n=== Invalid Combinations ===" << std::endl;
    // Group by component
    std::map<std::string, std::vector<std::string>> invalid_by_component;
    for (const auto& pair : invalid_combinations) {
      invalid_by_component[pair.first].push_back(pair.second);
    }
    
    for (const auto& entry : invalid_by_component) {
      std::cout << entry.first << ": ";
      for (size_t i = 0; i < entry.second.size(); ++i) {
        std::cout << entry.second[i];
        if (i < entry.second.size() - 1) std::cout << ", ";
      }
      std::cout << std::endl;
    }
    
    // This test doesn't fail - it just reports what's valid/invalid
    // The actual filtering will be done based on these results
    EXPECT_GT(total_valid, 0) << "At least some combinations should be valid";
  }

} // namespace px

// Ensure tests are discoverable - add a simple test outside namespace to verify
TEST(FluidTestBasic, TestFileLoaded) {
  EXPECT_TRUE(true) << "test_fluids.cpp is being compiled and run";
}
