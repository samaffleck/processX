// src/plant.cpp
#pragma once

#include <vector>
#include <unordered_map>
#include <stdexcept>

#include "processX/unitop.h"
#include "processX/stream.h"

namespace px {

  class Plant {
  public:
    std::vector<UnitOp> units;
    std::vector<Stream> streams;

    std::unordered_map<Handle, size_t> unit_idx;
    std::unordered_map<Handle, size_t> stream_idx;

    IdGen unit_ids{};
    IdGen stream_ids{};

    Handle AddUnit(
      UnitType type,
      std::vector<std::string> in_names,
      std::vector<std::string> out_names
    );
    Handle AddStream(StreamKind kind, std::string name);
    UnitOp& GetUnit(Handle h)   { return units.at(unit_idx.at(h)); }
    Stream& GetStream(Handle h) { return streams.at(stream_idx.at(h)); }
    void Connect(
      Handle stream,
      Handle from_unit, uint16_t from_out_slot,
      Handle to_unit,   uint16_t to_in_slot
    );

  };

} // namespace px
