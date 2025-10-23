// include/processX/unitop.h
#pragma once

#include <string>
#include <vector>
#include <variant>

#include "processX/stream.h"
#include "processX/registry.h"

namespace px {

  enum class UnitType { Valve, Mixer, Inlet, Outlet };

  class Flowsheet;

  struct IUnitOp {
    virtual ~IUnitOp() = default;
    virtual UnitType get_type() const = 0;
    virtual const char* type_name() const = 0;
    virtual bool validate(const Flowsheet& fs, std::string* why) const = 0;
    virtual void register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) = 0;
    virtual void add_equations(Flowsheet& fs, ResidualSystem& sys) = 0;
    
    const std::string& get_name() const { return name; };

    std::string name{};
  };

  struct Valve : public IUnitOp {
    Handle<Stream> in{};
    Handle<Stream> out{};
    Var Cv{"Cv", 1.0, false};

    UnitType get_type() const override { return UnitType::Valve; }
    const char* type_name() const override { return "Valve"; }

    bool validate(const Flowsheet& fs, std::string* why) const override;
    void register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) override;
    void add_equations(Flowsheet& fs, ResidualSystem& sys) override;
  };

} // namespace px
