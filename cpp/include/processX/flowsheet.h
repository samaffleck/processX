#pragma once

#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>
#include <unordered_map> 

// px includes
#include "processX/core.h"
#include "processX/solver.h"
#include "processX/stream.h"
#include "processX/unitop.h"
#include "processX/registry.h"

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
  };

  // ---------- Valve Demo Helper ----------
  static void run_valve_case(const std::string& title, std::function<void(Valve&, Stream&, Stream&)> config){
    std::cout<<"\n=== "<<title<<" ===\n";
    Flowsheet fs; 
    
    auto s_in  = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto v1 = fs.add<Valve>();

    auto& in  = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& val = fs.get<Valve>(v1);

    fs.connect_in<Valve>(v1, s_in);
    fs.connect_out<Valve>(v1, s_out);

    config(val, in, out);

    std::string err;
    if(!fs.assemble(&err)){ 
      std::cerr<<"Assemble error: "<<err<<"\n"; 
      return; 
    }

    std::cout<<"Unknowns: "<<fs.reg.unknowns_list()<<"\n";

    auto rep = fs.solve(NewtonOptions{50,1e-12,1e-14,1e-6,1e-8,true});
    
    std::cout<<(rep.converged?"Converged":"Not converged")<<" in "<<rep.iters<<" iters, |r|_inf="<<rep.final_res<<" : "<<rep.msg<<"\n";    
  }

} // end px namespace
