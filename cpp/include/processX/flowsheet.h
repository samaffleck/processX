#pragma once

// STL includes
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>
#include <unordered_map> 
#include <fstream>
#include <functional>

// px includes
#include "processX/core.h"
#include "processX/solver.h"
#include "processX/stream.h"
#include "processX/unitop.h"
#include "processX/registry.h"

// Cereal includes
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

// CoolProp includes
#include <CoolProp.h>
#include <AbstractState.h>


namespace px {

  template <class T> struct TypeTag { using type = T; }; // helper
  template<class> struct dependent_false : std::false_type {};

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

  class Flowsheet {
  public:
    using LogCallback = std::function<void(const std::string& message, bool is_error)>;
    void set_log_callback(LogCallback cb) { log_callback_ = cb; }
    void clear_log_callback() { log_callback_ = nullptr; }

    Registry<Stream> streams_;
    Registry<Valve>  valves_;
    Registry<Mixer> mixers_;
    Registry<Splitter> splitters_;
    Registry<HeatExchanger> heat_exchangers_;
    Registry<SimpleHeatExchanger> simple_heat_exchangers_;
    Registry<Pump> pumps_;
    Registry<ComponentSplitter> component_splitters_;

    std::vector<IUnitOp*> units_;

    FluidRegistry fluids;
    UnknownsRegistry reg;
    ResidualSystem sys;

    template<class T>
    Registry<T>& registry_for() {
      if constexpr (std::is_same_v<T, Stream>) {
        return streams_;
      } else if constexpr (std::is_same_v<T, Valve>) {
        return valves_;
      } else if constexpr (std::is_same_v<T, Mixer>) {
        return mixers_;
      } else if constexpr (std::is_same_v<T, Splitter>) {
        return splitters_;
      } else if constexpr (std::is_same_v<T, HeatExchanger>) {
        return heat_exchangers_;
      } else if constexpr (std::is_same_v<T, SimpleHeatExchanger>) {
        return simple_heat_exchangers_;
      } else if constexpr (std::is_same_v<T, Pump>) {
        return pumps_;
      } else if constexpr (std::is_same_v<T, ComponentSplitter>) {
        return component_splitters_;
      } else {
        static_assert(dependent_false<T>::value, "Unsupported type in registry_for<T>()");
      }
    }

    template<class T>
    const Registry<T>& registry_for() const {
      if constexpr (std::is_same_v<T, Stream>) {
        return streams_;
      } else if constexpr (std::is_same_v<T, Valve>) {
        return valves_;
      } else if constexpr (std::is_same_v<T, Mixer>) {
        return mixers_;
      } else if constexpr (std::is_same_v<T, Splitter>) {
        return splitters_;
      } else if constexpr (std::is_same_v<T, HeatExchanger>) {
        return heat_exchangers_;
      } else if constexpr (std::is_same_v<T, SimpleHeatExchanger>) {
        return simple_heat_exchangers_;
      } else if constexpr (std::is_same_v<T, Pump>) {
        return pumps_;
      } else if constexpr (std::is_same_v<T, ComponentSplitter>) {
        return component_splitters_;
      } else {
        static_assert(dependent_false<T>::value, "Unsupported type in registry_for<T>()");
      }
    }

    template <class T, class... Args>
    Handle<T> add(Args&&... args) {
      if constexpr (std::is_same_v<T, Stream>) {
        auto h = streams_.add(Stream{std::forward<Args>(args)...});
        auto& s = streams_.get(h);
        if (s.name.empty()) s.name = next_auto_name("stream");
        // Initialize composition variables if fluid package is set
        if (s.fluid_package_id != 0) {
          auto components = fluids.GetComponents(s.fluid_package_id);
          s.initialize_composition(components.size(), components);
        }
        return h;
      } else if constexpr (std::is_same_v<T, Valve>) {
        auto& r = registry_for<T>();
        auto h = r.add(T{std::forward<Args>(args)...});
        auto& u = r.get(h);
        if (u.name.empty()) u.name = next_auto_name(u.type_name());
        return h;
      } else if constexpr (std::is_same_v<T, Mixer>) {
        auto& r = registry_for<T>();
        auto h = r.add(T{std::forward<Args>(args)...});
        auto& u = r.get(h);
        if (u.name.empty()) u.name = next_auto_name(u.type_name());
        return h;
      } else if constexpr (std::is_same_v<T, Splitter>) {
        auto& r = registry_for<T>();
        auto h = r.add(T{std::forward<Args>(args)...});
        auto& u = r.get(h);
        if (u.name.empty()) u.name = next_auto_name(u.type_name());
        return h;
      } else if constexpr (std::is_same_v<T, HeatExchanger>) {
        auto& r = registry_for<T>();
        auto h = r.add(T{std::forward<Args>(args)...});
        auto& u = r.get(h);
        if (u.name.empty()) u.name = next_auto_name(u.type_name());
        return h;
      } else if constexpr (std::is_same_v<T, SimpleHeatExchanger>) {
        auto& r = registry_for<T>();
        auto h = r.add(T{std::forward<Args>(args)...});
        auto& u = r.get(h);
        if (u.name.empty()) u.name = next_auto_name(u.type_name());
        return h;
      } else if constexpr (std::is_same_v<T, Pump>) {
        auto& r = registry_for<T>();
        auto h = r.add(T{std::forward<Args>(args)...});
        auto& u = r.get(h);
        if (u.name.empty()) u.name = next_auto_name(u.type_name());
        return h;
      } else if constexpr (std::is_same_v<T, ComponentSplitter>) {
        auto& r = registry_for<T>();
        auto h = r.add(T{std::forward<Args>(args)...});
        auto& u = r.get(h);
        if (u.name.empty()) u.name = next_auto_name(u.type_name());
        return h;
      } else {
        static_assert(dependent_false<T>::value, "Unsupported type in Flowsheet::add<T>()");
        return Handle<T>{}; // Never reached, but satisfies compiler
      }
    }

    template <class T>
    T& get(Handle<T> h) {
      if constexpr (std::is_same_v<T, Stream>) return streams_.get(h);
      else if constexpr (std::is_same_v<T, Valve>) return valves_.get(h);
      else if constexpr (std::is_same_v<T, Mixer>) return mixers_.get(h);
      else if constexpr (std::is_same_v<T, Splitter>) return splitters_.get(h);
      else if constexpr (std::is_same_v<T, HeatExchanger>) return heat_exchangers_.get(h);
      else if constexpr (std::is_same_v<T, SimpleHeatExchanger>) return simple_heat_exchangers_.get(h);
      else if constexpr (std::is_same_v<T, Pump>) return pumps_.get(h);
      else if constexpr (std::is_same_v<T, ComponentSplitter>) return component_splitters_.get(h);
      else { 
        static_assert(dependent_false<T>::value, "Unsupported type in Flowsheet::get<T>()");
        return streams_.get(Handle<Stream>{}); // Never reached, but satisfies compiler
      }
    }

    template <class T>
    const T& get(Handle<T> h) const {
      if constexpr (std::is_same_v<T, Stream>) return streams_.get(h);
      else if constexpr (std::is_same_v<T, Valve>) return valves_.get(h);
      else if constexpr (std::is_same_v<T, Mixer>) return mixers_.get(h);
      else if constexpr (std::is_same_v<T, Splitter>) return splitters_.get(h);
      else if constexpr (std::is_same_v<T, HeatExchanger>) return heat_exchangers_.get(h);
      else if constexpr (std::is_same_v<T, SimpleHeatExchanger>) return simple_heat_exchangers_.get(h);
      else if constexpr (std::is_same_v<T, Pump>) return pumps_.get(h);
      else if constexpr (std::is_same_v<T, ComponentSplitter>) return component_splitters_.get(h);
      else { 
        static_assert(dependent_false<T>::value, "Unsupported type in Flowsheet::get<T>()");
        return streams_.get(Handle<Stream>{}); // Never reached, but satisfies compiler
      }
    }

    template <class T>
    bool erase(Handle<T> h) {
      if constexpr (std::is_same_v<T, Stream>) return streams_.erase(h);
      else if constexpr (std::is_same_v<T, Valve>) return valves_.erase(h);
      else if constexpr (std::is_same_v<T, Mixer>) return mixers_.erase(h);
      else if constexpr (std::is_same_v<T, Splitter>) return splitters_.erase(h);
      else if constexpr (std::is_same_v<T, HeatExchanger>) return heat_exchangers_.erase(h);
      else if constexpr (std::is_same_v<T, SimpleHeatExchanger>) return simple_heat_exchangers_.erase(h);
      else if constexpr (std::is_same_v<T, Pump>) return pumps_.erase(h);
      else if constexpr (std::is_same_v<T, ComponentSplitter>) return component_splitters_.erase(h);
      else { 
        static_assert(dependent_false<T>::value, "Unsupported type in Flowsheet::erase<T>()");
        return false; // Never reached, but satisfies compiler
      }
    }

    template <class T, class Fn>
    void for_each(Fn&& fn) {
      if constexpr (std::is_same_v<T, Stream>) streams_.for_each(std::forward<Fn>(fn));
      else if constexpr (std::is_same_v<T, Valve>) valves_.for_each(std::forward<Fn>(fn));
      else if constexpr (std::is_same_v<T, Mixer>) mixers_.for_each(std::forward<Fn>(fn));
      else if constexpr (std::is_same_v<T, Splitter>) splitters_.for_each(std::forward<Fn>(fn));
      else if constexpr (std::is_same_v<T, HeatExchanger>) heat_exchangers_.for_each(std::forward<Fn>(fn));
      else if constexpr (std::is_same_v<T, SimpleHeatExchanger>) simple_heat_exchangers_.for_each(std::forward<Fn>(fn));
      else if constexpr (std::is_same_v<T, Pump>) pumps_.for_each(std::forward<Fn>(fn));
      else if constexpr (std::is_same_v<T, ComponentSplitter>) component_splitters_.for_each(std::forward<Fn>(fn));
      else { 
        static_assert(dependent_false<T>::value, "Unsupported type in Flowsheet::for_each<T>()");
      }
    }

    void build_unit_list() {
      units_.clear();
      valves_.for_each([&](Valve& v){ units_.push_back(&v); }); 
      mixers_.for_each([&](Mixer& v){ units_.push_back(&v); }); 
      splitters_.for_each([&](Splitter& v){ units_.push_back(&v); }); 
      heat_exchangers_.for_each([&](HeatExchanger& v){ units_.push_back(&v); }); 
      simple_heat_exchangers_.for_each([&](SimpleHeatExchanger& v){ units_.push_back(&v); }); 
      pumps_.for_each([&](Pump& v){ units_.push_back(&v); }); 
      component_splitters_.for_each([&](ComponentSplitter& v){ units_.push_back(&v); }); 
    }

    bool assemble(std::string* err=nullptr);
    NewtonReport solve(const NewtonOptions& opt) { return newton_solve(reg, sys, opt); }

    template <typename UnitOpType>
    bool connect_in(Handle<UnitOpType> v, Handle<Stream> s)  { auto& V=get(v); V.in = s;  return true; }

    template <typename UnitOpType>
    bool connect_out(Handle<UnitOpType> v, Handle<Stream> s) { auto& V=get(v); V.out = s; return true; }
    
    template <typename UnitOpType>
    void disconnect_in(Handle<UnitOpType> v)  { get(v).in  = {}; }
    
    template <typename UnitOpType>
    void disconnect_out(Handle<UnitOpType> v) { get(v).out = {}; }

    // Get fluid package for a stream
    FluidPackage get_fluid_package(Handle<Stream> s) {
      auto& stream = get(s);
      return fluids.GetFluidPackage(stream.fluid_package_id);
    }

    // Set fluid package for a stream
    void set_fluid_package(Handle<Stream> s, size_t fluid_package_id) {
      auto& stream = get(s);
      stream.fluid_package_id = fluid_package_id;
      // Initialize composition variables based on fluid package components
      auto components = fluids.GetComponents(fluid_package_id);
      stream.initialize_composition(components.size(), components);
    }

    // Internal logging function (can be called from static helpers)
    void log_message(const std::string& message, bool is_error = false) {
      if (log_callback_) {
        log_callback_(message, is_error);
      }
    }

  private:
    std::unordered_map<std::string, uint32_t> counters_;
    LogCallback log_callback_;

    std::string next_auto_name(const std::string& prefix) {
      auto& n = counters_[prefix];
      return prefix + "-" + std::to_string(++n);
    }

    void ensure_fluid_initialized() {
      if (!fluids.Initialise()) {
        // Add error
      }
    }

  private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive& ar, std::uint32_t const /* version */) {
			ar(
				cereal::make_nvp("Flowsheet_Stream_Registry", streams_),
				cereal::make_nvp("Flowsheet_Valve_Registry", valves_),
				cereal::make_nvp("Flowsheet_Mixer_Registry", mixers_),
				cereal::make_nvp("Flowsheet_Splitter_Registry", splitters_),
				cereal::make_nvp("Flowsheet_HeatExchanger_Registry", heat_exchangers_),
				cereal::make_nvp("Flowsheet_SimpleHeatExchanger_Registry", simple_heat_exchangers_),
				cereal::make_nvp("Flowsheet_Pump_Registry", pumps_),
				cereal::make_nvp("Flowsheet_ComponentSplitter_Registry", component_splitters_),
				cereal::make_nvp("Flowsheet_FluidPackage_Registry", fluids)
			);
    }
  };

  inline void SaveToJson(const Flowsheet& data, const std::string& file_name) {
		std::ofstream file(file_name);
		cereal::JSONOutputArchive archive(file);
		archive(cereal::make_nvp("Flowsheet_Data", data));
	}

	inline void LoadFromJson(Flowsheet& data, const std::string& file_name) {
		std::ifstream ifs(file_name);
		cereal::JSONInputArchive archive(ifs);
		archive(data);
	}

} // end px namespace
