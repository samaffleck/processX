// include/processX/stream.h
#pragma once

#include "processX/types.h"
#include <string>
#include <string_view>
#include <cstdint>

#include <cereal/cereal.hpp>

#include "processX/core.h"


namespace px {

  class Stream {
  public:
    std::string name;
    Var molar_flow{"Molar Flowrate", 0.0, false}; // mol/s
    Var pressure{"Pressure", 1.0e5, false};   // Pa
    Var temperature{"Temperature", 300.0, false}; // K
    Var molar_enthalpy{"Molar Enthalpy", 0.0, false}; // J/mol
    size_t fluid_package_id = 0; // ID of the fluid package from FluidRegistry
    
    // Composition variables (dynamic based on fluid package components)
    std::vector<Var> mole_fractions;           // Overall mole fractions (one per component)
    // std::vector<Var> mole_fractions_liquid;    // Liquid phase mole fractions (one per component)
    // std::vector<Var> mole_fractions_vapor;    // Vapor phase mole fractions (one per component)
    // Var quality{"Quality", 0.0, false};        // Vapor fraction (0 = liquid, 1 = vapor)
    
    // Helper method to initialize composition variables based on component count
    void initialize_composition(size_t num_components, const std::vector<std::string>& component_names = {}) {
      mole_fractions.clear();
      // mole_fractions_liquid.clear();
      // mole_fractions_vapor.clear();
      
      if (num_components == 0) return;
      
      // Initialize with equal fractions (will be normalized)
      double default_fraction = 1.0 / num_components;
      for (size_t i = 0; i < num_components; ++i) {
        std::string comp_name = (i < component_names.size() && !component_names[i].empty()) 
                                ? component_names[i] 
                                : ("Component_" + std::to_string(i));
        mole_fractions.emplace_back("Mole_Fraction[" + comp_name + "]", default_fraction, false);
        // mole_fractions_liquid.emplace_back("Mole_Fraction_Liquid[" + comp_name + "]", default_fraction, false);
        // mole_fractions_vapor.emplace_back("Mole_Fraction_Vapor[" + comp_name + "]", default_fraction, false);
      }
    }
    
  private:
    friend class cereal::access;
    template<class Archive>
    void serialize(Archive& ar, std::uint32_t version) {
      ar(
        cereal::make_nvp("Stream_Name", name),
        cereal::make_nvp("Stream_Molar_Flowrate", molar_flow),
        cereal::make_nvp("Stream_Pressure", pressure),
        cereal::make_nvp("Stream_Temperature", temperature),
        cereal::make_nvp("Stream_Fluid_Package_ID", fluid_package_id),
        cereal::make_nvp("Stream_Mole_Fractions", mole_fractions)
        // cereal::make_nvp("Stream_Mole_Fractions_Liquid", mole_fractions_liquid),
        // cereal::make_nvp("Stream_Mole_Fractions_Vapor", mole_fractions_vapor),
        // cereal::make_nvp("Stream_Quality", quality)
      );
    }
  };

} // namespace px
