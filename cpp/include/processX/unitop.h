// include/processX/unitop.h
#pragma once

// STL includes
#include <string>
#include <vector>
#include <variant>
#include <algorithm>
#include <stdexcept>

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

  struct Vec2d {
    double x, y{};

  private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive& ar, std::uint32_t const version) {
			ar(
				cereal::make_nvp("x", x),
				cereal::make_nvp("y", y)
			);
    }
  };

  class IUnitOp {
  public:
    virtual ~IUnitOp() = default;
    virtual UnitType get_type() const = 0;
    virtual const char* type_name() const = 0;
    virtual bool validate(const Flowsheet& fs, std::string* why) const = 0;
    virtual void register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) = 0;
    virtual void add_equations(Flowsheet& fs, ResidualSystem& sys) = 0;
    virtual int num_inputs() = 0;
    virtual int num_outputs() = 0;

    const std::string& get_name() const { return name; };

    std::string name{};
    Vec2d pos{};
    Vec2d size{};

  private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive& ar, std::uint32_t const version) {
			ar(
				cereal::make_nvp("Unit_Operation_Name", name),
				cereal::make_nvp("Unit_Position_In_Flowsheet", pos),
				cereal::make_nvp("Unit_Size", size)
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
    int num_inputs() override { return 1; };
    int num_outputs() override { return 1; };

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

  class Mixer : public IUnitOp {
  public:
    std::vector<Handle<Stream>> in{};
    Handle<Stream> out{};

    UnitType get_type() const override { return UnitType::Mixer; }
    const char* type_name() const override { return "Mixer"; }

    bool validate(const Flowsheet& fs, std::string* why) const override;
    void register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) override;
    void add_equations(Flowsheet& fs, ResidualSystem& sys) override;
    int num_inputs() override { return in.size(); };
    int num_outputs() override { return 1; };

    size_t add_inlet(const Handle<Stream>& inlet) {
      if (!inlet.valid()) throw std::invalid_argument("add_mixer_inlet: inlet handle is invalid");

      auto it = std::find_if(in.begin(), in.end(),
                            [&](const Handle<Stream>& x){ return x.index == inlet.index; });
      if (it == in.end()) {
        in.push_back(inlet);
        return in.size() - 1;
      }
      return static_cast<size_t>(std::distance(in.begin(), it));
    }

    void set_outlet(const Handle<Stream>& outlet) {
      if (!outlet.valid()) throw std::invalid_argument("set_mixer_outlet: outlet handle is invalid");
      out = outlet;
    }

  private:
    bool contains_handle(const Handle<Stream>& h) const {
      auto it = std::find_if(in.begin(), in.end(), [&](const Handle<Stream>& x){ return x.index == h.index; });
      return it != in.end();
    }

		friend class cereal::access;
		template <class Archive>
		void serialize(Archive& ar, std::uint32_t const version) {
			ar(
				cereal::virtual_base_class<IUnitOp>(this),
				cereal::make_nvp("Mixer_Inlet_ID_Handles", in),
				cereal::make_nvp("Mixer_Outlet_ID_Handle", out)			
      );
		}
  };

} // namespace px
