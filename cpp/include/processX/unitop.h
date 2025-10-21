// include/processX/unitop.h
#pragma once

#include "processX/stream.h"
#include <string>
#include <vector>
#include <variant>

namespace px {

  enum class UnitType { Valve, Mixer, Inlet, Outlet };

  struct Valve {
    Stream* inlet{nullptr};
    Stream* outlet{nullptr};
    Var Cv{"valve.Cv", 1.0, false};

    bool validate(std::string* why=nullptr) const {
      if(!inlet){ if(why)*why="inlet not set"; return false; }
      if(!outlet){ if(why)*why="outlet not set"; return false; }
      return true;
    }

    void register_unknowns(UnknownsRegistry& reg){
      reg.register_var(inlet->molar_flow);
      reg.register_var(outlet->molar_flow);
      reg.register_var(inlet->pressure);
      reg.register_var(outlet->pressure);
      reg.register_var(Cv);
    }

    void add_equations(ResidualSystem& sys){
      sys.add("Valve balance: Fin - Fout = 0", [&](){
        return inlet->molar_flow.value - outlet->molar_flow.value;
      });
      sys.add("Valve PF: Fout - Cv*(Pin - Pout) = 0", [&](){
        return outlet->molar_flow.value - Cv.value * (inlet->pressure.value - outlet->pressure.value);
      });
    }
  };

  // struct ValveData   { double cv{100.0}; };
  // struct InletData   { Handle out_stream{kInvalid}; std::vector<double> split_ratio; };
  // struct OutletData  { Handle in_stream{kInvalid}; std::vector<double> split_ratio; };
  // struct MixerData   { std::vector<Handle> in_streams; Handle out_stream{kInvalid}; std::vector<double> split_ratio; };

  // using UnitData = std::variant<ValveData, InletData, OutletData, MixerData>;

  // struct UnitOp {
  //   Handle id{};
  //   UnitType type{};
  //   std::string name{};
  //   UnitData data{};
  // };

  // inline UnitOp MakeUnit(Handle id, UnitType t) {
  //   UnitOp u{ id, t, std::to_string(id), {} };
  //   switch (t) {
  //     case UnitType::Valve:   u.data = ValveData{};   break;
  //     case UnitType::Inlet:   u.data = InletData{};   break;
  //     case UnitType::Outlet:  u.data = OutletData{};  break;
  //     case UnitType::Mixer:   u.data = MixerData{};   break;
  //     default: break;
  //   }
  //   return u;
  // }

  // struct MakeUnitFn {
  //   UnitOp operator()(Handle id, UnitType t) const { return MakeUnit(id, t); }
  // };

} // namespace px
