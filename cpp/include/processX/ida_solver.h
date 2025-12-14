// include/processX/core.h
#pragma once

// STL includes
#include <vector>
#include <string>
#include <cassert>
#include <functional>
#include <iostream>

#include "processX/core.h"
#include "processX/user_data.h"

// SUNDIALS includes
#include <ida/ida.h>
#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_band.h>
#include <sunlinsol/sunlinsol_band.h>
#include <sunnonlinsol/sunnonlinsol_newton.h>


namespace px {
	
  class UserData;

  struct IntegrationStats {
    long int num_steps = 0;
    long int num_res_evals = 0;
    long int num_nonlin_iters = 0;
    long int num_err_test_fails = 0;
    long int num_conv_fails = 0;
  };

  class IDASettings {
  public:
    long int max_num_steps = 100000; 
    double init_step = 1e-10;
    double min_step = 1e-15; 
    double max_step = 2.0;
    double reltol = 1e-6; 
    int max_order = 5; // BDF [1->5]
    int max_nonlin_iters = 15; // per step
    int max_conv_fails = 200; 
    int max_err_test_fails = 40; 
    sunrealtype nonlin_conv_coef = 0.1; // 0 => default; otherwise (0,1]

  private:
    friend class ::cereal::access;
    template<class Archive>
    void serialize(Archive& ar, std::uint32_t const version) {
      ar(
        cereal::make_nvp("Maximum_Number_Of_Steps", max_num_steps),
        cereal::make_nvp("Initial_Step_Size", init_step),
        cereal::make_nvp("Minimum_Step_Size", min_step),
        cereal::make_nvp("Maximum_Step_Size", max_step),
        cereal::make_nvp("Relative_Tolerance", reltol),
        cereal::make_nvp("Maximum_Backwards_Difference_Formula_Order", max_order),
        cereal::make_nvp("Maximum_Nonlinear_Itterations_Per_Step", max_nonlin_iters),
        cereal::make_nvp("Maximum_Nonlinear_Convergence_Failures", max_conv_fails),
        cereal::make_nvp("Maximum_Error_Test_Failures", max_err_test_fails)
      );
    }
  };

  inline int ida_accumulate(int current, int flag, const char* what) {
    if (flag != IDA_SUCCESS) {
      std::fprintf(stderr, "[IDA] %s failed with flag = %d\n", what, flag);
      if (current == IDA_SUCCESS) return flag;
    }
    return current;
  }

  inline int ApplyIDASettings(void* ida_mem, const IDASettings& s) {
    int rc = IDA_SUCCESS;

    if (s.max_num_steps > 0)
      rc = ida_accumulate(rc, IDASetMaxNumSteps(ida_mem, s.max_num_steps), "IDASetMaxNumSteps");
    if (s.init_step > 0.0)
      rc = ida_accumulate(rc, IDASetInitStep(ida_mem, s.init_step), "IDASetInitStep");
    if (s.min_step > 0.0)
      rc = ida_accumulate(rc, IDASetMinStep(ida_mem, s.min_step), "IDASetMinStep");
    if (s.max_step > 0.0)
      rc = ida_accumulate(rc, IDASetMaxStep(ida_mem, s.max_step), "IDASetMaxStep");

    if (s.max_order > 0)
      rc = ida_accumulate(rc, IDASetMaxOrd(ida_mem, s.max_order), "IDASetMaxOrd");
    if (s.max_nonlin_iters > 0)
      rc = ida_accumulate(rc, IDASetMaxNonlinIters(ida_mem, s.max_nonlin_iters), "IDASetMaxNonlinIters");
    if (s.max_conv_fails > 0)
      rc = ida_accumulate(rc, IDASetMaxConvFails(ida_mem, s.max_conv_fails), "IDASetMaxConvFails");
    if (s.max_err_test_fails > 0)
      rc = ida_accumulate(rc, IDASetMaxErrTestFails(ida_mem, s.max_err_test_fails), "IDASetMaxErrTestFails");
    if (s.nonlin_conv_coef > 0.0)
      rc = ida_accumulate(rc, IDASetNonlinConvCoef(ida_mem, s.nonlin_conv_coef), "IDASetNonlinConvCoef");

    // Additional performance optimizations
    // Optimize step size control (reduces overhead from step size changes)
    rc = ida_accumulate(rc, IDASetEtaMin(ida_mem, 0.1), "IDASetEtaMin");
    rc = ida_accumulate(rc, IDASetEtaMax(ida_mem, 0.9), "IDASetEtaMax");

    return rc;
  }

  class IDASolverData {
  public:
    IDASolverData();
    ~IDASolverData();

    int Initialise(UserData& user_data, std::string& error_message);
    double GetTime() const;
    int SetStopTime(double t_final);
    int OneStep(double t_final, double& t_curr);
    void Unpack(UserData& user_data);
    IntegrationStats GetStatistics() const;

  private:
    SUNContext sunctx_ = nullptr;
    void* ida_mem_ = nullptr;
    N_Vector y_ = nullptr;
    N_Vector yp_ = nullptr;
    N_Vector id_ = nullptr;
    N_Vector abstol_vec_ = nullptr;
    SUNMatrix A_ = nullptr;
    SUNLinearSolver LS_ = nullptr;

  };

  class IDASolver {
  public:
    explicit IDASolver(UserData& data) : user_data_(data) {}
    int Solve(bool verbose = false);
    void PrintStatistics() const;

  private:
    bool Initialise(std::string& error_message);

    UserData& user_data_;
    IDASolverData solver_data_;
    
  };

} // end px namespace
