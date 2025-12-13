// include/processX/kinsol_solver.h
#pragma once

// STL includes
#include <vector>
#include <string>
#include <cassert>
#include <functional>
#include <iostream>

#include "processX/core.h"

// SUNDIALS includes
#include <sundials/sundials_core.h>
#include <kinsol/kinsol.h>
#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunnonlinsol/sunnonlinsol_newton.h>


namespace px {
	
  // ---------- Newton options / driver ----------
  struct NewtonOptions {
    int max_iters{50};
    double tol_res{1e-10};
    double tol_step{1e-12};
    double fd_rel{1e-6};
    double fd_abs{1e-8};
    bool verbose{true};
  };
  struct NewtonReport { bool converged{false}; int iters{0}; double final_res{0}; std::string msg; };

  // User data structure for KINSOL
  struct KINSOLUserData {
    UnknownsRegistry* reg;
    ResidualSystem* sys;
    const NewtonOptions* opt;
    bool verbose;
  };

  // Forward declarations
  int KINSOLResidual(N_Vector u, N_Vector f, void* user_data);
  NewtonReport newton_solve(UnknownsRegistry& reg, ResidualSystem& sys, const NewtonOptions& opt={});

  // --- Helper: builds FD Jacobian once and analyzes structure ---
  struct SystemAnalysis {
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
  int numeric_rank(std::vector<std::vector<double>> A, double pivot_tol,
                          std::vector<int>* pivots_out=nullptr);

  SystemAnalysis analyze_system(const UnknownsRegistry& reg,
                                      const ResidualSystem& sys,
                                      double fd_rel, double fd_abs,
                                      double tiny_row = 1e-14,
                                      double tiny_res = 1e-12);

} // end px namespace
