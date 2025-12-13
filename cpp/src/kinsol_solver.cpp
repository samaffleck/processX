// src/kinsol_solver.cpp

#include "processX/kinsol_solver.h"


namespace px {

  // KINSOL residual function
  int KINSOLResidual(N_Vector u, N_Vector f, void* user_data) {
    KINSOLUserData* udata = static_cast<KINSOLUserData*>(user_data);
    UnknownsRegistry* reg = udata->reg;
    ResidualSystem* sys = udata->sys;
    
    // Unpack KINSOL vector to variables
    sunrealtype* u_data = N_VGetArrayPointer(u);
    size_t n = reg->GetNumberOfUnknowns();
    reg->UnpackVariables(std::vector<double>(u_data, u_data + n));
    
    // Evaluate residuals
    auto residuals = sys->EvaluateResiduals();
    sunrealtype* f_data = N_VGetArrayPointer(f);
    
    size_t m = sys->GetNumberOfEquations();
    
    // KINSOL requires f to have the same size as u (n)
    // For overdetermined systems (m > n), we use the first n residuals
    // For underdetermined systems (m < n), we pad with zeros (shouldn't happen)
    for (size_t i = 0; i < n; ++i) {
      if (i < m) {
        f_data[i] = residuals[i];
      } else {
        f_data[i] = 0.0; // Pad with zeros if m < n (shouldn't happen)
      }
    }
    
    return KIN_SUCCESS;
  }

  NewtonReport newton_solve(UnknownsRegistry& reg, ResidualSystem& sys, const NewtonOptions& opt){
    NewtonReport rep;
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

    // Create SUNDIALS context
    SUNContext sunctx;
    int flag = SUNContext_Create(SUN_COMM_NULL, &sunctx);
    if (flag != SUN_SUCCESS) {
      rep.msg = "Failed to create SUNDIALS context";
      return rep;
    }

    // Create KINSOL memory
    void* kin_mem = KINCreate(sunctx);
    if (kin_mem == nullptr) {
      rep.msg = "Failed to create KINSOL memory";
      SUNContext_Free(&sunctx);
      return rep;
    }

    // Create vectors
    N_Vector u = N_VNew_Serial(n, sunctx);
    N_Vector scale = N_VNew_Serial(n, sunctx);
    if (u == nullptr || scale == nullptr) {
      rep.msg = "Failed to create N_Vector";
      if (u) N_VDestroy_Serial(u);
      if (scale) N_VDestroy_Serial(scale);
      KINFree(&kin_mem);
      SUNContext_Free(&sunctx);
      return rep;
    }

    // Initialize solution vector from current variable values
    sunrealtype* u_data = N_VGetArrayPointer(u);
    auto x0 = reg.PackVariables();
    for (size_t i = 0; i < n; ++i) {
      u_data[i] = x0[i];
      N_VConst(1.0, scale); // Unit scaling
    }

    // Set user data
    KINSOLUserData user_data;
    user_data.reg = &reg;
    user_data.sys = &sys;
    user_data.opt = &opt;
    user_data.verbose = opt.verbose;

    flag = KINSetUserData(kin_mem, &user_data);
    if (flag != KIN_SUCCESS) {
      rep.msg = "KINSetUserData failed";
      N_VDestroy_Serial(u);
      N_VDestroy_Serial(scale);
      KINFree(&kin_mem);
      SUNContext_Free(&sunctx);
      return rep;
    }

    // Initialize KINSOL
    flag = KINInit(kin_mem, KINSOLResidual, u);
    if (flag != KIN_SUCCESS) {
      rep.msg = "KINInit failed with code: " + std::to_string(flag);
      N_VDestroy_Serial(u);
      N_VDestroy_Serial(scale);
      KINFree(&kin_mem);
      SUNContext_Free(&sunctx);
      return rep;
    }

    // Set tolerances
    sunrealtype fnormtol = opt.tol_res;
    sunrealtype scsteptol = opt.tol_step;
    flag = KINSetFuncNormTol(kin_mem, fnormtol);
    if (flag != KIN_SUCCESS) {
      rep.msg = "KINSetFuncNormTol failed";
      N_VDestroy_Serial(u);
      N_VDestroy_Serial(scale);
      KINFree(&kin_mem);
      SUNContext_Free(&sunctx);
      return rep;
    }

    flag = KINSetScaledStepTol(kin_mem, scsteptol);
    if (flag != KIN_SUCCESS) {
      rep.msg = "KINSetScaledStepTol failed";
      N_VDestroy_Serial(u);
      N_VDestroy_Serial(scale);
      KINFree(&kin_mem);
      SUNContext_Free(&sunctx);
      return rep;
    }

    // Set maximum iterations
    flag = KINSetMaxSetupCalls(kin_mem, opt.max_iters);
    if (flag != KIN_SUCCESS) {
      rep.msg = "KINSetMaxSetupCalls failed";
      N_VDestroy_Serial(u);
      N_VDestroy_Serial(scale);
      KINFree(&kin_mem);
      SUNContext_Free(&sunctx);
      return rep;
    }

    // Set finite difference parameters for Jacobian
    flag = KINSetNumMaxIters(kin_mem, opt.max_iters);
    if (flag != KIN_SUCCESS) {
      rep.msg = "KINSetNumMaxIters failed";
      N_VDestroy_Serial(u);
      N_VDestroy_Serial(scale);
      KINFree(&kin_mem);
      SUNContext_Free(&sunctx);
      return rep;
    }

    // Create dense matrix and linear solver for Newton iteration
    SUNMatrix A = SUNDenseMatrix(n, n, sunctx);
    if (A == nullptr) {
      rep.msg = "Failed to create SUNMatrix";
      N_VDestroy_Serial(u);
      N_VDestroy_Serial(scale);
      KINFree(&kin_mem);
      SUNContext_Free(&sunctx);
      return rep;
    }

    SUNLinearSolver LS = SUNLinSol_Dense(u, A, sunctx);
    if (LS == nullptr) {
      rep.msg = "Failed to create SUNLinearSolver";
      SUNMatDestroy(A);
      N_VDestroy_Serial(u);
      N_VDestroy_Serial(scale);
      KINFree(&kin_mem);
      SUNContext_Free(&sunctx);
      return rep;
    }

    // Attach linear solver to KINSOL
    flag = KINSetLinearSolver(kin_mem, LS, A);
    if (flag != KIN_SUCCESS) {
      rep.msg = "KINSetLinearSolver failed with code: " + std::to_string(flag);
      SUNLinSolFree(LS);
      SUNMatDestroy(A);
      N_VDestroy_Serial(u);
      N_VDestroy_Serial(scale);
      KINFree(&kin_mem);
      SUNContext_Free(&sunctx);
      return rep;
    }

    // Set scaling
    N_VConst(1.0, scale);

    // Solve
    flag = KINSol(kin_mem, u, KIN_NONE, scale, scale);
    
    // Get statistics
    long int nni, nfe;
    flag = KINGetNumNonlinSolvIters(kin_mem, &nni);
    flag = KINGetNumFuncEvals(kin_mem, &nfe);
    
    // Get final residual norm
    sunrealtype fnorm;
    flag = KINGetFuncNorm(kin_mem, &fnorm);
    rep.final_res = fnorm;
    rep.iters = static_cast<int>(nni);

    // Check convergence
    if (flag == KIN_SUCCESS) {
      rep.converged = true;
      rep.msg = "Converged.";
    } else if (flag == KIN_INITIAL_GUESS_OK) {
      rep.converged = true;
      rep.msg = "Initial guess satisfied tolerance.";
    } else if (flag == KIN_STEP_LT_STPTOL) {
      rep.converged = true;
      rep.msg = "Converged by step tolerance.";
    } else {
      rep.converged = false;
      rep.msg = "KINSOL failed with code: " + std::to_string(flag);
    }

    if (opt.verbose) {
      std::cerr << "[KINSOL] " << rep.msg << " (iters=" << rep.iters 
                << ", |r|=" << rep.final_res << ")\n";
    }

    // Unpack solution back to variables
    u_data = N_VGetArrayPointer(u);
    reg.UnpackVariables(std::vector<double>(u_data, u_data + n));

    // Cleanup
    SUNLinSolFree(LS);
    SUNMatDestroy(A);
    N_VDestroy_Serial(u);
    N_VDestroy_Serial(scale);
    KINFree(&kin_mem);
    SUNContext_Free(&sunctx);

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

  SystemAnalysis analyze_system(const UnknownsRegistry& reg,
                                      const ResidualSystem& sys,
                                      double fd_rel, double fd_abs,
                                      double tiny_row,
                                      double tiny_res) {
    SystemAnalysis a;
    const size_t m = sys.GetNumberOfEquations();
    const size_t n = reg.GetNumberOfUnknowns();
    a.J.assign(m, std::vector<double>(n, 0.0));
    a.r = sys.EvaluateResiduals();
    a.row_max.assign(m, 0.0);
    a.col_max.assign(n, 0.0);

    // Build FD Jacobian (like Newton), but non-intrusive: we need a non-const reg to mutate,
    // so cast away constness in a controlled way (we'll restore values afterwards).
    UnknownsRegistry& reg_nc = const_cast<UnknownsRegistry&>(reg);
    const auto x0 = reg.PackVariables();

    for (size_t j = 0; j < n; ++j) {
      Var* vj = reg_nc.vars_[j];
      double base = vj->value_;
      double h = std::max(fd_abs, std::abs(base) * fd_rel);
      vj->value_ = base + h;
      auto r_ph = sys.EvaluateResiduals();
      vj->value_ = base;
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
