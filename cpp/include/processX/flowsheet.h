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
#include "processX/stream.h"
#include "processX/unitop.h"
#include "processX/registry.h"
#include "processX/fluids.h"

// Cereal includes
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>


namespace px {

  template <class T> struct TypeTag { using type = T; }; // helper
  template<class> struct dependent_false : std::false_type {};

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

    // Rebuild name counters from existing object names (call after loading from JSON)
    void rebuild_name_counters() {
      counters_.clear();

      // Helper lambda to extract counter from name like "stream-5" -> ("stream", 5)
      auto update_counter = [this](const std::string& name) {
        auto dash_pos = name.rfind('-');
        if (dash_pos != std::string::npos && dash_pos + 1 < name.size()) {
          std::string prefix = name.substr(0, dash_pos);
          std::string num_str = name.substr(dash_pos + 1);
          try {
            uint32_t num = std::stoul(num_str);
            auto& counter = counters_[prefix];
            if (num > counter) counter = num;
          } catch (...) {
            // Not a valid number, skip
          }
        }
      };

      // Scan all streams
      streams_.for_each([&](const Stream& s) {
        update_counter(s.name);
      });

      // Scan all unit operations
      valves_.for_each([&](const Valve& u) {
        update_counter(u.name);
      });
      mixers_.for_each([&](const Mixer& u) {
        update_counter(u.name);
      });
      splitters_.for_each([&](const Splitter& u) {
        update_counter(u.name);
      });
      heat_exchangers_.for_each([&](const HeatExchanger& u) {
        update_counter(u.name);
      });
      simple_heat_exchangers_.for_each([&](const SimpleHeatExchanger& u) {
        update_counter(u.name);
      });
      pumps_.for_each([&](const Pump& u) {
        update_counter(u.name);
      });
      component_splitters_.for_each([&](const ComponentSplitter& u) {
        update_counter(u.name);
      });
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
			std::cerr << "[Flowsheet::serialize] Starting..." << std::endl;
			try { ar(cereal::make_nvp("Flowsheet_Stream_Registry", streams_)); std::cerr << "[Flowsheet::serialize] ✓ Streams" << std::endl; } catch(const std::exception& e) { std::cerr << "[Flowsheet::serialize] ✗ Streams FAILED: " << e.what() << std::endl; throw; } catch(...) { std::cerr << "[Flowsheet::serialize] ✗ Streams FAILED: Unknown error" << std::endl; throw; }
			try { ar(cereal::make_nvp("Flowsheet_Valve_Registry", valves_)); std::cerr << "[Flowsheet::serialize] ✓ Valves" << std::endl; } catch(const std::exception& e) { std::cerr << "[Flowsheet::serialize] ✗ Valves FAILED: " << e.what() << std::endl; throw; } catch(...) { std::cerr << "[Flowsheet::serialize] ✗ Valves FAILED: Unknown error" << std::endl; throw; }
			try { ar(cereal::make_nvp("Flowsheet_Mixer_Registry", mixers_)); std::cerr << "[Flowsheet::serialize] ✓ Mixers" << std::endl; } catch(const std::exception& e) { std::cerr << "[Flowsheet::serialize] ✗ Mixers FAILED: " << e.what() << std::endl; throw; } catch(...) { std::cerr << "[Flowsheet::serialize] ✗ Mixers FAILED: Unknown error" << std::endl; throw; }
			try { ar(cereal::make_nvp("Flowsheet_Splitter_Registry", splitters_)); std::cerr << "[Flowsheet::serialize] ✓ Splitters" << std::endl; } catch(const std::exception& e) { std::cerr << "[Flowsheet::serialize] ✗ Splitters FAILED: " << e.what() << std::endl; throw; } catch(...) { std::cerr << "[Flowsheet::serialize] ✗ Splitters FAILED: Unknown error" << std::endl; throw; }
			try { ar(cereal::make_nvp("Flowsheet_HeatExchanger_Registry", heat_exchangers_)); std::cerr << "[Flowsheet::serialize] ✓ HeatExchangers" << std::endl; } catch(const std::exception& e) { std::cerr << "[Flowsheet::serialize] ✗ HeatExchangers FAILED: " << e.what() << std::endl; throw; } catch(...) { std::cerr << "[Flowsheet::serialize] ✗ HeatExchangers FAILED: Unknown error" << std::endl; throw; }
			try { ar(cereal::make_nvp("Flowsheet_SimpleHeatExchanger_Registry", simple_heat_exchangers_)); std::cerr << "[Flowsheet::serialize] ✓ SimpleHeatExchangers" << std::endl; } catch(const std::exception& e) { std::cerr << "[Flowsheet::serialize] ✗ SimpleHeatExchangers FAILED: " << e.what() << std::endl; throw; } catch(...) { std::cerr << "[Flowsheet::serialize] ✗ SimpleHeatExchangers FAILED: Unknown error" << std::endl; throw; }
			try { ar(cereal::make_nvp("Flowsheet_Pump_Registry", pumps_)); std::cerr << "[Flowsheet::serialize] ✓ Pumps" << std::endl; } catch(const std::exception& e) { std::cerr << "[Flowsheet::serialize] ✗ Pumps FAILED: " << e.what() << std::endl; throw; } catch(...) { std::cerr << "[Flowsheet::serialize] ✗ Pumps FAILED: Unknown error" << std::endl; throw; }
			try { ar(cereal::make_nvp("Flowsheet_ComponentSplitter_Registry", component_splitters_)); std::cerr << "[Flowsheet::serialize] ✓ ComponentSplitters" << std::endl; } catch(const std::exception& e) { std::cerr << "[Flowsheet::serialize] ✗ ComponentSplitters FAILED: " << e.what() << std::endl; throw; } catch(...) { std::cerr << "[Flowsheet::serialize] ✗ ComponentSplitters FAILED: Unknown error" << std::endl; throw; }
			try { ar(cereal::make_nvp("Flowsheet_FluidPackage_Registry", fluids)); std::cerr << "[Flowsheet::serialize] ✓ FluidPackages" << std::endl; } catch(const std::exception& e) { std::cerr << "[Flowsheet::serialize] ✗ FluidPackages FAILED: " << e.what() << std::endl; throw; } catch(...) { std::cerr << "[Flowsheet::serialize] ✗ FluidPackages FAILED: Unknown error" << std::endl; throw; }
			std::cerr << "[Flowsheet::serialize] Complete!" << std::endl;
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
