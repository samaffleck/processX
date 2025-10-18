// src/plant.cpp
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include "processX/plant.h"

namespace px {

  Handle Plant::AddUnit(
    UnitType type,
    std::vector<std::string> in_names,
    std::vector<std::string> out_names
  ) {
    Handle id = unit_ids.make();
    units.push_back(UnitOp{ id, type });
    units.back().inlet_names  = std::move(in_names);
    units.back().outlet_names = std::move(out_names);
    unit_idx[id] = units.size() - 1;
    return id;
  }

  Handle Plant::AddStream(StreamKind kind, std::string name) {
    Handle id = stream_ids.make();
    Stream s;
    s.id = id;
    s.kind = kind;
    s.props.name = std::move(name);
    streams.push_back(std::move(s));
    stream_idx[id] = streams.size() - 1;
    return id;
  }

  void Plant::Connect(
    Handle stream,
    Handle from_unit, uint16_t from_out_slot,
    Handle to_unit,   uint16_t to_in_slot
  ) {
    auto& s = GetStream(stream);
    auto& u_from = GetUnit(from_unit);
    auto& u_to   = GetUnit(to_unit);

    if (from_out_slot >= u_from.outletCount())
      throw std::runtime_error("Outlet slot out of range");
    if (to_in_slot >= u_to.inletCount())
      throw std::runtime_error("Inlet slot out of range");

    s.from = Endpoint{ from_unit, from_out_slot, PortKind::Outlet };
    s.to   = Endpoint{ to_unit,   to_in_slot,   PortKind::Inlet   };
  }

} // namespace px
