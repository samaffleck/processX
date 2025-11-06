// src/unitop.cpp

#include "processX/unitop.h"
#include "processX/flowsheet.h"
#include "processX/registry.h"

namespace px {  

  bool Valve::validate(const Flowsheet& fs, std::string* why) const {
    if (!in.valid())  { if (why) *why = "inlet not connected";  return false; }
    if (!out.valid()) { if (why) *why = "outlet not connected"; return false; }
    return true;
  }

  void Valve::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    reg.register_var(si.molar_flow);
    reg.register_var(so.molar_flow);
    reg.register_var(si.pressure);
    reg.register_var(so.pressure);
    reg.register_var(si.molar_enthalpy);
    reg.register_var(so.molar_enthalpy);
    reg.register_var(Cv);
  }

  void Valve::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    sys.add(name + ": balance", [&](){ return si.molar_flow.value - so.molar_flow.value; });
    sys.add(name + ": PF",      [&, self](){ return so.molar_flow.value - self->Cv.value * (si.pressure.value - so.pressure.value); });
    sys.add(name + ": energy",  [&](){ return so.molar_enthalpy.value * so.molar_flow.value - si.molar_enthalpy.value * si.molar_flow.value; });
  }

  bool Mixer::validate(const Flowsheet& fs, std::string* why) const {
    if (!out.valid()) { if (why) *why = "outlet not connected"; return false; }
    if (in.size() < 1) { if (why) *why = "mixer needs at least one inlet"; return false; }
    for (size_t i = 0; i < in.size(); ++i) {
      if (!in[i].valid()) { if (why) *why = "one or more inlets not connected"; return false; }
    }
    return true;
  }

  void Mixer::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& so = fs.get<Stream>(out);
    reg.register_var(so.molar_flow);
    reg.register_var(so.pressure);
    reg.register_var(so.molar_enthalpy);

    for (auto h : in) {
      auto& si = fs.get<Stream>(h);
      reg.register_var(si.molar_flow);
      reg.register_var(si.pressure);
      reg.register_var(si.molar_enthalpy);
    }
  }

  void Mixer::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto& so = fs.get<Stream>(out);

    sys.add(name + ": total balance", [&]() {
      double sum_in = 0.0;
      for (auto h : in) sum_in += fs.get<Stream>(h).molar_flow.value;
      return sum_in - so.molar_flow.value;
    });

    sys.add(name + ": energy balance", [&]() {
      double sum_in = 0.0;
      for (auto h : in) {
        auto& s = fs.get<Stream>(h);
        sum_in += s.molar_flow.value * s.molar_enthalpy.value;
      }
      return sum_in - so.molar_flow.value * so.molar_enthalpy.value;
    });

    for (size_t i = 0; i < in.size(); ++i) {
      auto& si = fs.get<Stream>(in[i]);
      sys.add(name + ": p_equal[" + std::to_string(i) + "]", [&si, &so]() {
        return si.pressure.value - so.pressure.value;
      });
    }
  }

  bool Splitter::validate(const Flowsheet& fs, std::string* why) const {
    if (!in.valid()) { if (why) *why = "inlet not connected"; return false; }
    if (out.size() < 1) { if (why) *why = "splitter needs at least one outlet"; return false; }
    for (size_t i = 0; i < out.size(); ++i) {
      if (!out[i].valid()) { if (why) *why = "one or more outlets not connected"; return false; }
    }
    return true;
  }

  void Splitter::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& si = fs.get<Stream>(in);
    reg.register_var(si.molar_flow);
    reg.register_var(si.pressure);
    reg.register_var(si.molar_enthalpy);

    for (auto h : out) {
      auto& so = fs.get<Stream>(h);
      reg.register_var(so.molar_flow);
      reg.register_var(so.pressure);
      reg.register_var(so.molar_enthalpy);
    }
  }

  void Splitter::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto& si = fs.get<Stream>(in);

    sys.add(name + ": total balance", [&]() {
      double sum_out = 0.0;
      for (auto h : out) sum_out += fs.get<Stream>(h).molar_flow.value;
      return si.molar_flow.value - sum_out;
    });

    sys.add(name + ": energy balance", [&]() {
      double sum_out = 0.0;
      for (auto h : out) {
        auto& s = fs.get<Stream>(h);
        sum_out += s.molar_flow.value * s.molar_enthalpy.value;
      }
      return si.molar_flow.value * si.molar_enthalpy.value - sum_out;
    });

    for (size_t i = 0; i < out.size(); ++i) {
      auto& so = fs.get<Stream>(out[i]);
      sys.add(name + ": p_equal[" + std::to_string(i) + "]", [&si, &so]() {
        return si.pressure.value - so.pressure.value;
      });
      sys.add(name + ": h_equal[" + std::to_string(i) + "]", [&si, &so]() {
        return si.molar_enthalpy.value - so.molar_enthalpy.value;
      });
    }
  }

} // namespace px

CEREAL_REGISTER_TYPE(px::Valve)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::Valve)
CEREAL_CLASS_VERSION(px::Valve, 0)

CEREAL_REGISTER_TYPE(px::Mixer)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::Mixer)
CEREAL_CLASS_VERSION(px::Mixer, 0)

CEREAL_REGISTER_TYPE(px::Splitter)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::Splitter)
CEREAL_CLASS_VERSION(px::Splitter, 0)
