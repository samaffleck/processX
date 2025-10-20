// include/processX/stream.h
#pragma once

#include "processX/types.h"
#include <string>
#include <string_view>
#include <cstdint>

namespace px {

  using Handle = uint32_t;
  inline constexpr Handle kInvalid = Handle(-1);

  struct IdGen { Handle next{0}; Handle make() { return next++; } };

  enum class PortKind { Inlet, Outlet };
  enum class StreamType { Material /*, Energy, Signal, ...*/ };

  struct StreamProps {
    comp_var mole_fraction{};
    var mass_flow_rate{};
    var pressure{};
    var temperature{};
  };

  struct Stream {
    Stream(Handle id_, StreamType type_)  : id(id_), type(type_), name(std::to_string(id)) {}    
    
    Handle id{kInvalid};
    StreamType type{StreamType::Material};
    std::string name{};
    StreamProps props{};
  };

  inline std::string_view GetStreamTypeName(StreamType k) {
    switch (k) {
      case StreamType::Material: return "Material Stream";
      default: return "";
    }
  }

} // namespace px
