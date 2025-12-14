#pragma once

// STL includes
#include <string>
#include <memory>
#include <vector>

// CoolProp includes
#include <CoolProp.h>
#include <AbstractState.h>

// Cereal includes
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>


namespace px {

  // CoolProp backend names
  extern const char* backend_names[];
  extern const int kNumberOfBackends;

  // CoolProp component names
  extern const char* component_names[];
  extern const int kNumberOfComponents;

  using FluidPackage = std::shared_ptr<CoolProp::AbstractState>;
  using ComponentList = std::vector<std::string>;

  class FluidRegistry {
  public:
    size_t AddFluidPackage(const ComponentList& components = {}, const std::string& thermo_pkg = "HEOS", const std::string& name = "") {
      size_t id = ++next_id;
      pkg_components[id] = components;
      pkg_thermo[id] = thermo_pkg;
      // Set default name if not provided
      if (name.empty()) {
        pkg_names[id] = "Package #" + std::to_string(id);
      } else {
        pkg_names[id] = name;
      }
      pkg[id] = std::shared_ptr<CoolProp::AbstractState>(
        CoolProp::AbstractState::factory(thermo_pkg, components)
      );
      // Note: Phase envelope should be built after mole fractions are set, not here
      return id;
    }

    FluidPackage GetFluidPackage(size_t id) const {
      auto it = pkg.find(id);
      if (it != pkg.end()) {
        return it->second;
      }
      return nullptr; // Return nullptr if not found
    }

    ComponentList GetComponents(size_t id) const {
      auto it = pkg_components.find(id);
      if (it != pkg_components.end()) {
        return it->second;
      }
      return ComponentList{}; // Return empty list if not found
    }

    std::string GetThermoPackage(size_t id) const {
      auto it = pkg_thermo.find(id);
      if (it != pkg_thermo.end()) {
        return it->second;
      }
      return ""; // Return empty string if not found
    }

    std::string GetPackageName(size_t id) const {
      auto it = pkg_names.find(id);
      if (it != pkg_names.end()) {
        return it->second;
      }
      return "Package #" + std::to_string(id); // Return default name if not found
    }

    void SetPackageName(size_t id, const std::string& name) {
      if (!name.empty()) {
        pkg_names[id] = name;
      }
    }

    void UpdateComponents(size_t id, const ComponentList& components) {
      auto it = pkg_components.find(id);
      if (it != pkg_components.end()) {
        // Allow empty component lists - set package to nullptr in this case
        if (components.empty()) {
          pkg_components[id] = components;
          pkg[id] = nullptr;  // Empty package - no AbstractState
        } else {
          std::string thermo_pkg = GetThermoPackage(id);
          // Try to create the fluid package first - if this fails, we don't update anything
          auto new_pkg = std::shared_ptr<CoolProp::AbstractState>(
            CoolProp::AbstractState::factory(thermo_pkg, components)
          );
          // Only update if creation succeeded
          pkg_components[id] = components;
          pkg[id] = new_pkg;
        }
      }
    }

    void RemoveFluidPackage(size_t pkg_id) {
      pkg.erase(pkg_id);
      pkg_components.erase(pkg_id);
      pkg_thermo.erase(pkg_id);
      pkg_names.erase(pkg_id);
    }

    // Get the first available fluid package ID, or 0 if none exist
    size_t GetFirstAvailablePackageId() const {
      if (pkg.empty()) return 0;
      // Return the first valid package ID
      return pkg.begin()->first;
    }

    // Get all fluid package IDs
    std::vector<size_t> GetAllPackageIds() const {
      std::vector<size_t> ids;
      ids.reserve(pkg.size());
      for (const auto& [id, fluid] : pkg) {
        ids.push_back(id);
      }
      return ids;
    }

    // Calculate temperature from enthalpy and pressure using iterative method
    // Returns the calculated temperature, or T_guess if calculation fails
    // For mixtures, mole_fractions should be provided; for pure fluids, can be empty
    double CalculateTemperatureFromEnthalpyAndPressure(
      size_t pkg_id,
      double H_target,  // Target molar enthalpy (J/mol)
      double P,          // Pressure (Pa)
      double T_guess,   // Initial temperature guess (K)
      const std::vector<double>& mole_fractions = {}  // Mole fractions for mixtures
    ) const;

    // Calculate temperature from entropy and pressure using iterative method
    // Returns the calculated temperature, or T_guess if calculation fails
    // For mixtures, mole_fractions should be provided; for pure fluids, can be empty
    double CalculateTemperatureFromEntropyAndPressure(
      size_t pkg_id,
      double S_target,  // Target molar entropy (J/mol/K)
      double P,         // Pressure (Pa)
      double T_guess,   // Initial temperature guess (K)
      const std::vector<double>& mole_fractions = {}  // Mole fractions for mixtures
    ) const;

    bool Initialise() { 
      // Reconstruct fluid packages from components if needed (e.g., after deserialization)
      for (const auto& [id, components] : pkg_components) {
        if (pkg.find(id) == pkg.end() || !pkg[id]) {
          // Reconstruct fluid package from stored components
          auto it_thermo = pkg_thermo.find(id);
          std::string thermo_pkg = (it_thermo != pkg_thermo.end()) ? it_thermo->second : "HEOS";
          pkg[id] = std::shared_ptr<CoolProp::AbstractState>(
            CoolProp::AbstractState::factory(thermo_pkg, components)
          );
          // Note: Phase envelope should be built after mole fractions are set, not here
        }
      }
      
      if (pkg.size() <= 0) return false;
      for (const auto& [id, fluid] : pkg) {
        if (!fluid) {
          return false;
        }
        if (pkg_components.at(id).size() <= 0) {
          return false; // Needs at least 1 component
        }
      }
      return true;
    }
  
  private:
    std::unordered_map<size_t, ComponentList> pkg_components;
    std::unordered_map<size_t, std::string> pkg_thermo;
    std::unordered_map<size_t, std::string> pkg_names;
    std::unordered_map<size_t, FluidPackage> pkg;
    size_t next_id = 0;

		friend class cereal::access;
		template <class Archive>
		void serialize(Archive& ar, std::uint32_t const /* version */) {
			ar(
				cereal::make_nvp("FluidPackage_Components", pkg_components),
				cereal::make_nvp("FluidPackage_Thermo", pkg_thermo),
				cereal::make_nvp("FluidPackage_Names", pkg_names),
				cereal::make_nvp("FluidPackage_NextID", next_id)
			);
			// Note: FluidPackage objects (pkg) are not serialized directly as they contain
			// CoolProp::AbstractState pointers that need to be reconstructed from components
		}
  };

} // namespace px

