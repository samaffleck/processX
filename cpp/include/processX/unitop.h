// include/processX/unitop.h
#pragma once

#include "processX/stream.h"
#include <string>
#include <vector>

namespace px {

  enum class UnitType { Valve, Mixer, Splitter };

  struct UnitOp {
    Handle id{};
    UnitType type{};
    std::string name{};

    // implicit ports by names (or just counts if you prefer)
    std::vector<std::string> inlet_names;
    std::vector<std::string> outlet_names;

    UnitOp(Handle id_, UnitType type_) 
      : id(id_), type(type_), name(std::to_string(id)) {}

    uint16_t inletCount() const  { return (uint16_t)inlet_names.size(); }
    uint16_t outletCount() const { return (uint16_t)outlet_names.size(); }
  };

  // per-type data components (keep as-is)
  struct ValveData   { double cv{100.0}; };
  struct MixerData   { std::vector<double> split_ratio; };
  struct SplitterData{ std::vector<double> split_ratio; };

} // namespace px
