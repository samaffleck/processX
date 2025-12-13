// src/solver.cpp

#include "processX/ida_solver.h"
#include "processX/flowsheet.h"


namespace px {

  IDASolverData::IDASolverData() {
    SUNContext_Create(SUN_COMM_NULL, &sunctx_);
  }

  IDASolverData::~IDASolverData() {
    if (ida_mem_) {
      IDAFree(&ida_mem_);
      ida_mem_ = nullptr;
    }
    if (LS_) {
      SUNLinSolFree(LS_);
      LS_ = nullptr;
    }
    if (A_) {
      SUNMatDestroy(A_);
      A_ = nullptr;
    }
    if (y_) {
      N_VDestroy_Serial(y_);
      y_ = nullptr;
    }
    if (yp_) {
      N_VDestroy_Serial(yp_);
      yp_ = nullptr;
    }
    if (id_) {
      N_VDestroy_Serial(id_);
      id_ = nullptr;
    }
    if (abstol_vec_) {
      N_VDestroy_Serial(abstol_vec_);
      abstol_vec_ = nullptr;
    }
    if (sunctx_) {
      SUNContext_Free(&sunctx_);
      sunctx_ = nullptr;
    }
  }

  int IDASolverData::Initialise(UserData& user_data, std::string& error_message) {
    auto& fs = user_data.fs;
    auto& settings = user_data.settings;

    int sys_size = 0; // TODO: remove when I implement the this time-dependent solver 
    // int sys_size = fs.GetSystemSize(); 
    // if (sys_size <= 0) {
    //   error_message += ("Invalid system size: " + std::to_string(sys_size));
    //   return false;
    // }

    y_ = N_VNew_Serial(sys_size, sunctx_);
    yp_ = N_VNew_Serial(sys_size, sunctx_);
    id_ = N_VNew_Serial(sys_size, sunctx_);
    abstol_vec_ = N_VNew_Serial(sys_size, sunctx_);
    
    // Pack initial conditions
    sunrealtype* y_data = N_VGetArrayPointer(y_);
    sunrealtype* yp_data = N_VGetArrayPointer(yp_);
    sunrealtype* id_data = N_VGetArrayPointer(id_);
    sunrealtype* abstol_data = N_VGetArrayPointer(abstol_vec_);
    
    // state.PackBanded(y_data, yp_data);
    // state.SetIDBanded(id_data);
    // state.SetToleranceVectorBanded(abstol_data);
    
    // Create and configure IDA
    ida_mem_ = IDACreate(sunctx_);
    if (!ida_mem_) {
      error_message += ("Failed to create IDA memory");
      return IDA_MEM_NULL;
    }

    // int retval = IDAInit(ida_mem_, UpdateResiduals, 0.0, y_, yp_);
    int retval = -1; // TODO: remove once implemented
    if (retval != IDA_SUCCESS) {
      error_message += ("IDAInit failed with code: " + std::to_string(retval));
      return retval;
    }

    retval = IDASetUserData(ida_mem_, &user_data);
    if (retval != IDA_SUCCESS) {
      error_message += ("IDASetUserData failed with code: " + std::to_string(retval));
      return retval;
    }

    retval = IDASVtolerances(ida_mem_, settings.reltol, abstol_vec_);
    if (retval != IDA_SUCCESS) {
      error_message += ("IDASVtolerances failed with code: " + std::to_string(retval));
      return retval;
    }

    retval = IDASetId(ida_mem_, id_);
    if (retval != IDA_SUCCESS) {
      error_message += ("IDASetId failed with code: " + std::to_string(retval));
      return retval;
    }

    // size_t num_equations = state.GetEquations().size();
    // sunindextype mu = static_cast<sunindextype>(num_equations * 2); // Upper bandwidth
    // sunindextype ml = static_cast<sunindextype>(num_equations * 2); // Lower bandwidth
    
    // A_ = SUNBandMatrix(sys_size, mu, ml, sunctx_);
    // if (!A_) {
    //   error_message += ("Failed to create banded matrix");
    //   return IDA_ILL_INPUT;
    // }
    
    LS_ = SUNLinSol_Band(y_, A_, sunctx_);
    if (!LS_) {
      error_message += ("Failed to create banded linear solver");
      return IDA_ILL_INPUT;
    }

    retval = IDASetLinearSolver(ida_mem_, LS_, A_);
    if (retval != IDA_SUCCESS) {
      error_message += ("IDASetLinearSolver failed with code: " + std::to_string(retval));
      return retval;
    }

    // Apply IDA settings
    ApplyIDASettings(ida_mem_, settings);

    // Compute consistent initial conditions
    retval = IDACalcIC(ida_mem_, IDA_YA_YDP_INIT, 1e-8);
    if (retval != IDA_SUCCESS) {
      error_message += ("Warning: IDACalcIC returned: " + std::to_string(retval));
      return retval;
    }

    return IDA_SUCCESS;
  }

  double IDASolverData::GetTime() const {
    double time = 0.0;
    int flag = IDAGetCurrentTime(ida_mem_, &time);
    return time;
  }

  int IDASolverData::SetStopTime(double t_final) {
    int flag = IDASetStopTime(ida_mem_, t_final);
    return flag;  
  }

  int IDASolverData::OneStep(double t_final, double& t_curr) {
    int flag = IDASolve(ida_mem_, t_final, &t_curr, y_, yp_, IDA_ONE_STEP);
    return flag;
  }

  void IDASolverData::Unpack(UserData& user_data) {
    sunrealtype* y_data = N_VGetArrayPointer(y_);
    sunrealtype* yp_data = N_VGetArrayPointer(yp_);
    // user_data.state.UnpackBanded(y_data, yp_data);
    // user_data.state.UpdateIndependentEquations(&user_data);
  }

  IntegrationStats IDASolverData::GetStatistics() const {
    IntegrationStats stats;
    if (ida_mem_) {
      IDAGetNumSteps(ida_mem_, &stats.num_steps);
      IDAGetNumResEvals(ida_mem_, &stats.num_res_evals);
      IDAGetNumNonlinSolvIters(ida_mem_, &stats.num_nonlin_iters);
      IDAGetNumErrTestFails(ida_mem_, &stats.num_err_test_fails);
      IDAGetNumNonlinSolvConvFails(ida_mem_, &stats.num_conv_fails);
    }
    return stats;
  }


  bool IDASolver::Initialise(std::string& error_message) {
    // TODO
  }

  int IDASolver::Solve(bool verbose) {
    // TODO
  }

  void IDASolver::PrintStatistics() const {
    auto stats = solver_data_.GetStatistics();
    std::cout << "Statistics:" << std::endl;
    std::cout << "  Number of steps: " << stats.num_steps << std::endl;
    std::cout << "  Number of residual evals: " << stats.num_res_evals << std::endl;
    std::cout << "  Number of nonlinear iterations: " << stats.num_nonlin_iters << std::endl;
    std::cout << "  Number of error test failures: " << stats.num_err_test_fails << std::endl;
    std::cout << "  Number of convergence failures: " << stats.num_conv_fails << std::endl;
  }

} // namespace px


CEREAL_CLASS_VERSION(px::IDASolver, 0)
CEREAL_CLASS_VERSION(px::IDASolverData, 0)
