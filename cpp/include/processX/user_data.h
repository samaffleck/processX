#pragma once

#include "processX/core.h"
#include "processX/flowsheet.h"
#include "processX/ida_solver.h"
#include "processX/kinsol_solver.h"

// SUNDIALS includes
#include <ida/ida.h>
#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_band.h>
#include <sunlinsol/sunlinsol_band.h>
#include <sunnonlinsol/sunnonlinsol_newton.h>


namespace px {
  
  class UserData {
  public:
    Flowsheet fs;
    NewtonOptions newton_options;

  private:
    friend class ::cereal::access;
    template<class Archive>
    void serialize(Archive& ar, std::uint32_t const version) {
      ar(
        cereal::make_nvp("Flowsheet_Data", fs),
        cereal::make_nvp("KINSOL_Solver_Settings", newton_options)
      );
    }
  };

} // end px namespace
