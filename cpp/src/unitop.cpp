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
    reg.register_var(si.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(so.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(Cv);
  }

  void Valve::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    sys.add(name + ": balance", [&](){ return si.molar_flow.value - so.molar_flow.value; });
    sys.add(name + ": PF",      [&, self](){ return so.molar_flow.value - self->Cv.value * (si.pressure.value - so.pressure.value); });
    sys.add(name + ": h_equal", [&](){ return si.molar_enthalpy.value - so.molar_enthalpy.value; });
    // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
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
    reg.register_var(so.temperature);  // Needed for state equation: T = f(H, P)

    for (auto h : in) {
      auto& si = fs.get<Stream>(h);
      reg.register_var(si.molar_flow);
      reg.register_var(si.pressure);
      reg.register_var(si.molar_enthalpy);
      reg.register_var(si.temperature);  // Needed for state equation: T = f(H, P)
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
      // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
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
    reg.register_var(si.temperature);  // Needed for state equation: T = f(H, P)

    for (auto h : out) {
      auto& so = fs.get<Stream>(h);
      reg.register_var(so.molar_flow);
      reg.register_var(so.pressure);
      reg.register_var(so.molar_enthalpy);
      reg.register_var(so.temperature);  // Needed for state equation: T = f(H, P)
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
      // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
    }
  }

  bool HeatExchanger::validate(const Flowsheet& fs, std::string* why) const {
    if (!in_hot.valid())  { if (why) *why = "hot inlet not connected";  return false; }
    if (!out_hot.valid()) { if (why) *why = "hot outlet not connected"; return false; }
    if (!in_cold.valid())  { if (why) *why = "cold inlet not connected";  return false; }
    if (!out_cold.valid()) { if (why) *why = "cold outlet not connected"; return false; }
    return true;
  }

  void HeatExchanger::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& si_hot = fs.get<Stream>(in_hot);
    auto& so_hot = fs.get<Stream>(out_hot);
    auto& si_cold = fs.get<Stream>(in_cold);
    auto& so_cold = fs.get<Stream>(out_cold);

    // Hot side
    reg.register_var(si_hot.molar_flow);
    reg.register_var(so_hot.molar_flow);
    reg.register_var(si_hot.pressure);
    reg.register_var(so_hot.pressure);
    reg.register_var(si_hot.molar_enthalpy);
    reg.register_var(so_hot.molar_enthalpy);
    reg.register_var(si_hot.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(so_hot.temperature);  // Needed for state equation: T = f(H, P)

    // Cold side
    reg.register_var(si_cold.molar_flow);
    reg.register_var(so_cold.molar_flow);
    reg.register_var(si_cold.pressure);
    reg.register_var(so_cold.pressure);
    reg.register_var(si_cold.molar_enthalpy);
    reg.register_var(so_cold.molar_enthalpy);
    reg.register_var(si_cold.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(so_cold.temperature);  // Needed for state equation: T = f(H, P)

    // Heat exchanger parameters
    reg.register_var(dP_hot);
    reg.register_var(dP_cold);
    reg.register_var(Q);
  }

  void HeatExchanger::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto& si_hot = fs.get<Stream>(in_hot);
    auto& so_hot = fs.get<Stream>(out_hot);
    auto& si_cold = fs.get<Stream>(in_cold);
    auto& so_cold = fs.get<Stream>(out_cold);
    auto* self = this;

    // Mass balance - hot side
    sys.add(name + ": hot mass balance", [&]() {
      return si_hot.molar_flow.value - so_hot.molar_flow.value;
    });

    // Mass balance - cold side
    sys.add(name + ": cold mass balance", [&]() {
      return si_cold.molar_flow.value - so_cold.molar_flow.value;
    });

    // Energy balance - hot side: Q = m_hot * (h_hot_in - h_hot_out)
    // Hot side loses heat, so enthalpy decreases
    sys.add(name + ": hot energy balance", [&, self]() {
      return self->Q.value - si_hot.molar_flow.value * (si_hot.molar_enthalpy.value - so_hot.molar_enthalpy.value);
    });

    // Energy balance - cold side: Q = m_cold * (h_cold_out - h_cold_in)
    // Cold side gains heat, so enthalpy increases
    sys.add(name + ": cold energy balance", [&, self]() {
      return self->Q.value - so_cold.molar_flow.value * (so_cold.molar_enthalpy.value - si_cold.molar_enthalpy.value);
    });

    // Pressure drop - hot side: P_out = P_in - dP
    sys.add(name + ": hot pressure drop", [&, self]() {
      return so_hot.pressure.value - (si_hot.pressure.value - self->dP_hot.value);
    });

    // Pressure drop - cold side: P_out = P_in - dP
    sys.add(name + ": cold pressure drop", [&, self]() {
      return so_cold.pressure.value - (si_cold.pressure.value - self->dP_cold.value);
    });

    // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
  }

  bool SimpleHeatExchanger::validate(const Flowsheet& fs, std::string* why) const {
    if (!in.valid())  { if (why) *why = "inlet not connected";  return false; }
    if (!out.valid()) { if (why) *why = "outlet not connected"; return false; }
    return true;
  }

  void SimpleHeatExchanger::register_unknowns(Flowsheet& fs, UnknownsRegistry& reg) {
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    reg.register_var(si.molar_flow);
    reg.register_var(so.molar_flow);
    reg.register_var(si.pressure);
    reg.register_var(so.pressure);
    reg.register_var(si.molar_enthalpy);
    reg.register_var(so.molar_enthalpy);
    reg.register_var(si.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(so.temperature);  // Needed for state equation: T = f(H, P)
    reg.register_var(dP);
    reg.register_var(Q);
  }

  void SimpleHeatExchanger::add_equations(Flowsheet& fs, ResidualSystem& sys) {
    auto* self = this;
    auto& si = fs.get<Stream>(in);
    auto& so = fs.get<Stream>(out);
    sys.add(name + ": balance", [&](){ return si.molar_flow.value - so.molar_flow.value; });
    sys.add(name + ": energy", [&, self](){ 
      // Q = m * (h_out - h_in), positive Q means heat added to stream
      return self->Q.value - so.molar_flow.value * (so.molar_enthalpy.value - si.molar_enthalpy.value); 
    });
    sys.add(name + ": pressure_drop", [&, self](){ 
      return so.pressure.value - (si.pressure.value - self->dP.value); 
    });
    // Note: State equations are added once per stream in Flowsheet::assemble() to avoid duplicates
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

CEREAL_REGISTER_TYPE(px::HeatExchanger)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::HeatExchanger)
CEREAL_CLASS_VERSION(px::HeatExchanger, 0)

CEREAL_REGISTER_TYPE(px::SimpleHeatExchanger)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::SimpleHeatExchanger)
CEREAL_CLASS_VERSION(px::SimpleHeatExchanger, 0)
