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
    reg.register_var(Cv);
  }

  void Valve::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    sys.add(name + ": balance", [&](){ return si.molar_flow.value - so.molar_flow.value; });
    sys.add(name + ": PF",      [&, self](){ return so.molar_flow.value - self->Cv.value * (si.pressure.value - so.pressure.value); });
  }

  bool Mixer::validate(const Flowsheet& fs, std::string* why) const {
    if (!out.valid()) { if (why) *why = "outlet not connected"; return false; }
    if (in.size() < 2) { if (why) *why = "mixer needs at least two inlets"; return false; }
    for (size_t i = 0; i < in.size(); ++i) {
      if (!in[i].valid()) { if (why) *why = "one or more inlets not connected"; return false; }
    }
    return true;
  }

  void Mixer::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& so = fs.get<Stream>(out);
    reg.register_var(so.molar_flow);
    reg.register_var(so.pressure);

    for (auto h : in) {
      auto& si = fs.get<Stream>(h);
      reg.register_var(si.molar_flow);
      reg.register_var(si.pressure);
    }
  }

  void Mixer::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto& so = fs.get<Stream>(out);

    sys.add(name + ": total balance", [&]() {
      double sum_in = 0.0;
      for (auto h : in) sum_in += fs.get<Stream>(h).molar_flow.value;
      return sum_in - so.molar_flow.value;
    });

    for (size_t i = 0; i < in.size(); ++i) {
      auto& si = fs.get<Stream>(in[i]);
      sys.add(name + ": p_equal[" + std::to_string(i) + "]", [&si, &so]() {
        return si.pressure.value - so.pressure.value;
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