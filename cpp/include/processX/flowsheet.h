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


namespace px {

  template <class T> struct TypeTag { using type = T; }; // helper
  template<class> struct dependent_false : std::false_type {};

  class Flowsheet {
  public:
    Registry<Stream> streams_;
    Registry<Valve>  valves_;

    std::vector<IUnitOp*> units_;

    UnknownsRegistry reg;
    ResidualSystem sys;

    template<class T>
    Registry<T>& registry_for() {
      if constexpr (std::is_same_v<T, Stream>) {
        return streams_;
      } else if constexpr (std::is_same_v<T, Valve>) {
        return valves_;
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
        return h;
      } else if constexpr (std::is_same_v<T, Valve>) {
        auto& r = registry_for<T>();
        auto h = r.add(T{std::forward<Args>(args)...});
        auto& u = r.get(h);
        if (u.name.empty()) u.name = next_auto_name(u.type_name());
        return h;
      } else {
        static_assert(sizeof(T) == 0, "Unsupported type in Flowsheet::add<T>()");
      }
    }

    template <class T>
    T& get(Handle<T> h) {
      if constexpr (std::is_same_v<T, Stream>) return streams_.get(h);
      else if constexpr (std::is_same_v<T, Valve>) return valves_.get(h);
      else { static_assert(sizeof(T) == 0, "Unsupported type in Flowsheet::get<T>()"); }
    }

    template <class T>
    const T& get(Handle<T> h) const {
      if constexpr (std::is_same_v<T, Stream>) return streams_.get(h);
      else if constexpr (std::is_same_v<T, Valve>) return valves_.get(h);
      else { static_assert(sizeof(T) == 0, "Unsupported type in Flowsheet::get<T>()"); }
    }

    template <class T>
    bool erase(Handle<T> h) {
      if constexpr (std::is_same_v<T, Stream>) return streams_.erase(h);
      else if constexpr (std::is_same_v<T, Valve>) return valves_.erase(h);
      else { static_assert(sizeof(T) == 0, "Unsupported type in Flowsheet::erase<T>()"); }
    }

    template <class T, class Fn>
    void for_each(Fn&& fn) {
      if constexpr (std::is_same_v<T, Stream>) streams_.for_each(std::forward<Fn>(fn));
      else if constexpr (std::is_same_v<T, Valve>) valves_.for_each(std::forward<Fn>(fn));
      else { static_assert(sizeof(T) == 0, "Unsupported type in Flowsheet::for_each<T>()"); }
    }

    void build_unit_list() {
      units_.clear();
      valves_.for_each([&](Valve& v){ units_.push_back(&v); }); 
      // Add other unit types similarly...
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

  private:
    std::unordered_map<std::string, uint32_t> counters_;

    std::string next_auto_name(const std::string& prefix) {
      auto& n = counters_[prefix];
      return prefix + "-" + std::to_string(++n);
    }

  private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive& ar, std::uint32_t const version) {
			ar(
				cereal::make_nvp("Flowsheet_Stream_Registry", streams_),
				cereal::make_nvp("Flowsheet_Valve_Registry", valves_)
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
