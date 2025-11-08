// include/processX/stream.h
#pragma once

#include "processX/types.h"
#include <string>
#include <string_view>
#include <cstdint>

#include <cereal/cereal.hpp>

#include "processX/core.h"


namespace px {

  class Stream {
  public:
    std::string name;
    Var molar_flow{"Molar Flowrate", 0.0, false}; // mol/s
    Var pressure{"Pressure", 1.0e5, false};   // Pa
    Var temperature{"Temperature", 300.0, false}; // K
    Var molar_enthalpy{"Molar Enthalpy", 0.0, false}; // J/mol
    
  private:
    friend class cereal::access;
    template<class Archive>
    void serialize(Archive& ar, std::uint32_t version) {
      ar(
        cereal::make_nvp("Stream_Name", name),
        cereal::make_nvp("Stream_Molar_Flowrate", molar_flow),
        cereal::make_nvp("Stream_Pressure", pressure),
        cereal::make_nvp("Stream_Temperature", temperature)
      );
    }
  };

} // namespace px
