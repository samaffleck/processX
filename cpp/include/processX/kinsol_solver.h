// include/processX/kinsol_solver.h
#pragma once

// STL includes
#include <vector>
#include <string>

// SUNDIALS includes (minimal - only for type definitions)
#include <sundials/sundials_types.h>
#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>

// ProcessX includes (for type definitions)
#include "processX/core.h"

namespace px {
	
  class NewtonOptions {
  public:
    int max_iters{50};
    double tol_res{1e-10};
    double tol_step{1e-12};
    double fd_rel{1e-6};
    double fd_abs{1e-8};
    bool verbose{true};

  private:
    friend class ::cereal::access;
    template<class Archive>
    void serialize(Archive& ar, std::uint32_t const version) {
      ar(
        cereal::make_nvp("Maximum_Number_Of_Itterations", max_iters),
        cereal::make_nvp("Tol_res", tol_res),
        cereal::make_nvp("tol_step", tol_step),
        cereal::make_nvp("fd_rel", fd_rel),
        cereal::make_nvp("fd_abs", fd_abs),
        cereal::make_nvp("verbose", verbose)
      );
    }
  };

  struct NewtonReport { 
    bool converged{false}; 
    int iters{0}; 
    double final_res{0}; 
    std::string msg; 
  };

  // Forward declarations
  class UserData;
  class UnknownsRegistry;
  class ResidualSystem;
  
  int ResidualUpdate(N_Vector u, N_Vector f, void* user_data);
  NewtonReport newton_solve(UserData& user_data);

  class KINSOLSolverData {
  public:
    KINSOLSolverData();
    ~KINSOLSolverData();
    KINSOLSolverData(const KINSOLSolverData&) = delete;
    KINSOLSolverData& operator=(const KINSOLSolverData&) = delete;

    NewtonReport Initialise(UserData& user_data);
    NewtonReport Solve(UserData& user_data);
  
  private:
    void Cleanup();
    std::string ApplySettings(const NewtonOptions& opt);

    SUNContext sunctx;
    void* kin_mem;
    N_Vector u;
    N_Vector scale;
    SUNMatrix A;
    SUNLinearSolver LS;
    bool initialized_;
    size_t n_unknowns_;
    
  };

  // --- Helper: builds FD Jacobian once and analyzes structure ---
  class SystemAnalysis {
  public:
    std::vector<std::vector<double>> J;
    std::vector<double> r;
    std::vector<double> row_max;  // per-equation max |J_ij|
    std::vector<double> col_max;  // per-unknown  max |J_ij|
    int rank{0};
    std::vector<int> pivots;      // pivot column indices
    std::vector<int> redundant_eqs;     // rows with tiny Jacobian and tiny residual
    std::vector<int> inconsistent_eqs;  // rows with tiny Jacobian but non-tiny residual
    std::vector<int> unconstrained_unknowns; // columns with tiny Jacobian
  };

  // numeric rank via Gaussian elim with threshold
  int numeric_rank(
    std::vector<std::vector<double>> A, 
    double pivot_tol,
    std::vector<int>* pivots_out = nullptr
  );

  SystemAnalysis analyze_system(
    const UnknownsRegistry& reg,
    const ResidualSystem& sys,
    double fd_rel, 
    double fd_abs,
    double tiny_row = 1e-14,
    double tiny_res = 1e-12
  );

} // end px namespace
