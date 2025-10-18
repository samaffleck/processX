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
  enum class StreamKind { Material /*, Energy, Signal, ...*/ };

  struct Endpoint {
    Handle unit{kInvalid};
    uint16_t slot{0};
    PortKind kind{PortKind::Inlet};
  };

  struct StreamProps {
    std::string name{};
    comp_var mole_fraction{};
    var mass_flow_rate{};
    var pressure{};
    var temperature{};
  };

  struct Stream {
    Handle id{kInvalid};
    StreamKind kind{StreamKind::Material};
    StreamProps props{};

    Endpoint from{};           // must be Outlet
    Endpoint to{};             // must be Inlet
  };

  inline std::string_view GetStreamKindName(StreamKind k) {
    switch (k) {
      case StreamKind::Material: return "Material Stream";
      default: return "";
    }
  }

} // namespace px
