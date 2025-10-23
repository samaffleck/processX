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

} // namespace px

CEREAL_REGISTER_TYPE(px::Valve)
CEREAL_REGISTER_POLYMORPHIC_RELATION(px::IUnitOp, px::Valve)
CEREAL_CLASS_VERSION(px::Valve, 0)

