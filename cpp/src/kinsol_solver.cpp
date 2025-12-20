// src/kinsol_solver.cpp

#include "processX/kinsol_solver.h"
#include "processX/user_data.h"
#include "processX/core.h"

// SUNDIALS includes
#include <sundials/sundials_core.h>
#include <kinsol/kinsol.h>
#include <iostream>


namespace px {

  // KINSOL residual function
  int ResidualUpdate(N_Vector u, N_Vector f, void* user_data) {
    
    // static cast into my user_data object
    UserData* udata = static_cast<UserData*>(user_data);
    
    auto& fs = udata->fs;
    auto& reg = fs.reg;
    auto& sys = fs.sys;
    
    // Unpack KINSOL vector to variables (direct, no copy)
    sunrealtype* u_data = N_VGetArrayPointer(u);
    size_t n = reg.GetNumberOfUnknowns();
    // reg.UnpackVariables(u_data, n);
    reg.UnpackVariables(u_data, n);
    
    // Evaluate residuals directly into f (no intermediate vector)
    sunrealtype* f_data = N_VGetArrayPointer(f);
    size_t m = sys.GetNumberOfEquations();
    
    // KINSOL requires f to have the same size as u (n)
    // For overdetermined systems (m > n), we use the first n residuals
    // For underdetermined systems (m < n), we pad with zeros (shouldn't happen)
    if (m >= n) {
      // Overdetermined or square: evaluate first n residuals directly into f
      sys.EvaluateResiduals(f_data, n);
    } else {
      // Underdetermined: evaluate all m residuals, then pad with zeros
      sys.EvaluateResiduals(f_data, m);
      for (size_t i = m; i < n; ++i) {
        f_data[i] = 0.0;
      }
    }
    
    return KIN_SUCCESS;
  }
  
  KINSOLSolverData::KINSOLSolverData() 
    : sunctx(nullptr), kin_mem(nullptr), u(nullptr), scale(nullptr), 
      A(nullptr), LS(nullptr), initialized_(false), n_unknowns_(0) 
  {
    SUNErrCode err = SUNContext_Create(SUN_COMM_NULL, &sunctx);
    if (err != SUN_SUCCESS || sunctx == nullptr) {
      sunctx = nullptr; // Mark as failed
    }
  }

  KINSOLSolverData::~KINSOLSolverData() {
    Cleanup();
  }

  void KINSOLSolverData::Cleanup() {
    if (LS) {
      SUNLinSolFree(LS);
      LS = nullptr;
    }
    if (A) {
      SUNMatDestroy(A);
      A = nullptr;
    }
    if (u) {
      N_VDestroy_Serial(u);
      u = nullptr;
    }
    if (scale) {
      N_VDestroy_Serial(scale);
      scale = nullptr;
    }
    if (kin_mem) {
      KINFree(&kin_mem);
      kin_mem = nullptr;
    }
    if (sunctx) {
      SUNContext_Free(&sunctx);
      sunctx = nullptr;
    }
    initialized_ = false;
  }

  std::string KINSOLSolverData::ApplySettings(const NewtonOptions& opt) {
    // Set tolerances
    sunrealtype fnormtol = opt.tol_res;
    sunrealtype scsteptol = opt.tol_step;
    int flag = KINSetFuncNormTol(kin_mem, fnormtol);
    if (flag != KIN_SUCCESS) {
      return "KINSetFuncNormTol failed";
    }

    flag = KINSetScaledStepTol(kin_mem, scsteptol);
    if (flag != KIN_SUCCESS) {
      return "KINSetScaledStepTol failed";
    }

    // Set maximum iterations
    flag = KINSetMaxSetupCalls(kin_mem, opt.max_iters);
    if (flag != KIN_SUCCESS) {
      return "KINSetMaxSetupCalls failed";
    }

    flag = KINSetNumMaxIters(kin_mem, opt.max_iters);
    if (flag != KIN_SUCCESS) {
      return "KINSetNumMaxIters failed";
    }

    return ""; // Success
  }

  NewtonReport KINSOLSolverData::Initialise(UserData& user_data) {
    NewtonReport rep;
    auto& fs = user_data.fs;
    auto& reg = fs.reg;
    auto& sys = fs.sys;
    auto& opt = user_data.newton_options;

    size_t n = reg.GetNumberOfUnknowns();
    size_t m = sys.GetNumberOfEquations();
    
    // Allow equations >= unknowns (redundancy handled by rank analysis in assemble)
    if(n > m) { 
      rep.msg = "DOF mismatch: unknowns=" + std::to_string(n) + " > equations=" + std::to_string(m) + " (under-specified)"; 
      if(opt.verbose) { 
        std::cerr << "[KINSOL] " << rep.msg << "\nUnknowns: " << reg.GetFeeVariableNames() << "\n"; 
      } 
      return rep; 
    }
    
    if(n == 0) { 
      rep.converged = true; 
      rep.msg = "Nothing to solve."; 
      return rep; 
    }

    // Check if context was created successfully
    if (sunctx == nullptr) {
      rep.msg = "Failed to create SUNContext";
      return rep;
    }

    // Create KINSOL memory
    kin_mem = KINCreate(sunctx);
    if (kin_mem == nullptr) {
      rep.msg = "Failed to create KINSOL memory";
      return rep;
    }

    // Create vectors
    u = N_VNew_Serial(n, sunctx);
    scale = N_VNew_Serial(n, sunctx);
    
    if (u == nullptr || scale == nullptr) {
      rep.msg = "Failed to create N_Vector";
      Cleanup();
      return rep;
    }

    // Initialize solution vector
    sunrealtype* u_data = N_VGetArrayPointer(u);
    reg.PackVariables(u_data, n);
    N_VConst(1.0, scale);

    int flag = KINSetUserData(kin_mem, &user_data);
    if (flag != KIN_SUCCESS) {
      rep.msg = "KINSetUserData failed";
      Cleanup();
      return rep;
    }

    // Initialize KINSOL
    flag = KINInit(kin_mem, ResidualUpdate, u);
    if (flag != KIN_SUCCESS) {
      rep.msg = "KINInit failed with code: " + std::to_string(flag);
      Cleanup();
      return rep;
    }

    // Apply solver settings
    rep.msg = ApplySettings(opt);
    if (!rep.msg.empty()) {
      Cleanup();
      return rep;
    }

    // Create dense matrix and linear solver for Newton iteration
    A = SUNDenseMatrix(n, n, sunctx);
    if (A == nullptr) {
      rep.msg = "Failed to create SUNMatrix";
      Cleanup();
      return rep;
    }

    LS = SUNLinSol_Dense(u, A, sunctx);
    if (LS == nullptr) {
      rep.msg = "Failed to create SUNLinearSolver";
      Cleanup();
      return rep;
    }

    // Attach linear solver to KINSOL
    flag = KINSetLinearSolver(kin_mem, LS, A);
    if (flag != KIN_SUCCESS) {
      rep.msg = "KINSetLinearSolver failed with code: " + std::to_string(flag);
      Cleanup();
      return rep;
    }

    // Initialization successful
    initialized_ = true;
    n_unknowns_ = n;
    return rep;
  }

  NewtonReport KINSOLSolverData::Solve(UserData& user_data) {
    NewtonReport rep;
    
    if (!initialized_) {
      rep.msg = "Solver not initialized. Call Initialise() first.";
      return rep;
    }

    auto& fs = user_data.fs;
    auto& reg = fs.reg;
    auto& opt = user_data.newton_options;

    // Solve
    int flag = KINSol(kin_mem, u, KIN_NONE, scale, scale);
    
    // Get statistics (save the solve flag before overwriting)
    int solve_flag = flag;
    long int nni, nfe;
    KINGetNumNonlinSolvIters(kin_mem, &nni);
    KINGetNumFuncEvals(kin_mem, &nfe);
    
    // Get final residual norm
    sunrealtype fnorm;
    KINGetFuncNorm(kin_mem, &fnorm);
    rep.final_res = fnorm;
    rep.iters = static_cast<int>(nni);

    // Check convergence using the solve flag
    if (solve_flag == KIN_SUCCESS) {
      rep.converged = true;
      rep.msg = "Converged.";
    } else if (solve_flag == KIN_INITIAL_GUESS_OK) {
      rep.converged = true;
      rep.msg = "Initial guess satisfied tolerance.";
    } else if (solve_flag == KIN_STEP_LT_STPTOL) {
      rep.converged = true;
      rep.msg = "Converged by step tolerance.";
    } else {
      rep.converged = false;
      rep.msg = "KINSOL failed with code: " + std::to_string(solve_flag);
    }

    if (opt.verbose) {
      std::cerr << "[KINSOL] " << rep.msg << " (iters=" << rep.iters 
                << ", |r|=" << rep.final_res << ")\n";
    }

    // Unpack solution back to variables (direct, no copy)
    sunrealtype* u_data = N_VGetArrayPointer(u);
    reg.UnpackVariables(u_data, n_unknowns_);
    
    return rep;
  }

  NewtonReport newton_solve(UserData& user_data){
    KINSOLSolverData solver_data;

    NewtonReport rep = solver_data.Initialise(user_data);
    
    // If initialization failed (error message present) or nothing to solve, return early
    if (!rep.msg.empty()) {
      return rep;
    }

    // Initialization successful - proceed to solve
    rep = solver_data.Solve(user_data);
    return rep;
  }

  // numeric rank via Gaussian elim with threshold
  int numeric_rank(std::vector<std::vector<double>> A, double pivot_tol,
                          std::vector<int>* pivots_out) {
    int m = (int)A.size();
    int n = m ? (int)A[0].size() : 0;
    int r = 0;
    std::vector<int> piv;
    for (int k = 0; k < std::min(m,n); ++k) {
      // find pivot in col k from row k..m-1
      int piv_row = -1;
      double best = 0.0;
      for (int i = k; i < m; ++i) {
        double val = std::abs(A[i][k]);
        if (val > best) { best = val; piv_row = i; }
      }
      if (best <= pivot_tol) continue; // no pivot in this column
      if (piv_row != k) std::swap(A[piv_row], A[k]);
      double d = A[k][k];
      for (int j = k; j < n; ++j) A[k][j] /= d;
      // eliminate below
      for (int i = k + 1; i < m; ++i) {
        double f = A[i][k];
        if (std::abs(f) < 1e-20) continue;
        for (int j = k; j < n; ++j) A[i][j] -= f * A[k][j];
      }
      ++r;
      piv.push_back(k);
    }
    if (pivots_out) *pivots_out = std::move(piv);
    return r;
  }

  SystemAnalysis analyze_system(
    const UnknownsRegistry& reg,
    const ResidualSystem& sys,
    double fd_rel, double fd_abs,
    double tiny_row,
    double tiny_res
  ) {
    SystemAnalysis a;
    const size_t m = sys.GetNumberOfEquations();
    const size_t n = reg.GetNumberOfUnknowns();
    a.J.assign(m, std::vector<double>(n, 0.0));
    a.r.resize(m, 0.0);  
    sys.EvaluateResiduals(a.r.data(), m);
    a.row_max.assign(m, 0.0);
    a.col_max.assign(n, 0.0);

    // Build FD Jacobian (like Newton), but non-intrusive: we need a non-const reg to mutate,
    // so cast away constness in a controlled way (we'll restore values afterwards).
    UnknownsRegistry& reg_nc = const_cast<UnknownsRegistry&>(reg);
    const auto x0 = reg.PackVariables();

    for (size_t j = 0; j < n; ++j) {
      double base = reg_nc.GetVariableValue(j);
      double h = std::max(fd_abs, std::abs(base) * fd_rel);
      reg_nc.SetVariableValue(j, base + h);
      std::vector<double> r_ph(m);
      sys.EvaluateResiduals(r_ph.data(), m);
      reg_nc.SetVariableValue(j, base);
      for (size_t i = 0; i < m; ++i) {
        double Jij = (r_ph[i] - a.r[i]) / h;
        a.J[i][j] = Jij;
        a.row_max[i] = std::max(a.row_max[i], std::abs(Jij));
        a.col_max[j] = std::max(a.col_max[j], std::abs(Jij));
      }
    }
    reg_nc.UnpackVariables(x0); // restore

    // Row diagnostics
    for (size_t i = 0; i < m; ++i) {
      if (a.row_max[i] <= tiny_row) {
        if (std::abs(a.r[i]) <= tiny_res) a.redundant_eqs.push_back((int)i);
        else a.inconsistent_eqs.push_back((int)i);
      }
    }
    // Column diagnostics
    for (size_t j = 0; j < n; ++j) {
      if (a.col_max[j] <= tiny_row) a.unconstrained_unknowns.push_back((int)j);
    }

    // Numeric rank of J
    // Use a pivot tolerance scaled by max row/col; here a simple absolute tol works for small systems
    a.rank = numeric_rank(a.J, /*pivot_tol=*/1e-12, &a.pivots);
    return a;
  }

} // end px namespace
