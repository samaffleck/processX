// include/processX/unitop.h
#pragma once

// STL includes
#include <string>
#include <vector>
#include <variant>

// Cereal includes
#include <cereal/types/memory.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>

// ProcessX includes
#include "processX/stream.h"
#include "processX/registry.h"

namespace px {

  enum class UnitType { Valve, Mixer, Inlet, Outlet };

  class Flowsheet;

  class IUnitOp {
  public:
    virtual ~IUnitOp() = default;
    virtual UnitType get_type() const = 0;
    virtual const char* type_name() const = 0;
    virtual bool validate(const Flowsheet& fs, std::string* why) const = 0;
    virtual void register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) = 0;
    virtual void add_equations(Flowsheet& fs, ResidualSystem& sys) = 0;
    
    const std::string& get_name() const { return name; };

    std::string name{};

  private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive& ar, std::uint32_t const version) {
			ar(
				cereal::make_nvp("Unit_Operation_Name", name)
			);
    }
  };

  class Valve : public IUnitOp {
  public:
    Handle<Stream> in{};
    Handle<Stream> out{};
    Var Cv{"Cv", 1.0, false};

    UnitType get_type() const override { return UnitType::Valve; }
    const char* type_name() const override { return "Valve"; }

    bool validate(const Flowsheet& fs, std::string* why) const override;
    void register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) override;
    void add_equations(Flowsheet& fs, ResidualSystem& sys) override;

  private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive& ar, std::uint32_t const version) {
			ar(
				cereal::virtual_base_class<IUnitOp>(this),
				cereal::make_nvp("Valve_Inlet_ID_Handle", in),
				cereal::make_nvp("Valve_Outlet_ID_Handle", out),
				cereal::make_nvp("Valve_Flow_Coefficient", Cv)
			);
		}
  };

} // namespace px
