// include/processX/unitop.h
#pragma once

#include "processX/stream.h"
#include <string>
#include <vector>
#include <variant>

namespace px {

  enum class UnitType { Valve, Mixer, Inlet, Outlet };

  struct ValveData   { double cv{100.0}; };
  struct InletData   { Handle out_stream{kInvalid}; std::vector<double> split_ratio; };
  struct OutletData  { Handle in_stream{kInvalid}; std::vector<double> split_ratio; };
  struct MixerData   { std::vector<Handle> in_streams; Handle out_stream{kInvalid}; std::vector<double> split_ratio; };

  using UnitData = std::variant<ValveData, InletData, OutletData, MixerData>;

  struct UnitOp {
    Handle id{};
    UnitType type{};
    std::string name{};
    UnitData data{};
  };

  inline UnitOp MakeUnit(Handle id, UnitType t) {
    UnitOp u{ id, t, std::to_string(id), {} };
    switch (t) {
      case UnitType::Valve:   u.data = ValveData{};   break;
      case UnitType::Inlet:   u.data = InletData{};   break;
      case UnitType::Outlet:  u.data = OutletData{};  break;
      case UnitType::Mixer:   u.data = MixerData{};   break;
      default: break;
    }
    return u;
  }

  struct MakeUnitFn {
    UnitOp operator()(Handle id, UnitType t) const { return MakeUnit(id, t); }
  };

} // namespace px
