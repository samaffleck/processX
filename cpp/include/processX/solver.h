// include/processX/core.h
#pragma once

// STL includes
#include <vector>
#include <string>
#include <cassert>
#include <functional>
#include <iostream>

#include "processX/core.h"


namespace px {

  static std::pair<bool,std::vector<double>> solve_linear(std::vector<std::vector<double>> A, std::vector<double> b){
    int n = (int)b.size(); 
    for(int i=0;i<n;++i) A[i].push_back(b[i]);
    for(int col=0; col<n; ++col){
      int piv=col; double best=std::abs(A[col][col]);
      for(int r=col+1;r<n;++r){ double v=std::abs(A[r][col]); if(v>best){best=v;piv=r;} }
      if(best<1e-16) return {false,{}};
      if(piv!=col) std::swap(A[piv],A[col]);
      double d=A[col][col]; for(int j=col;j<=n;++j) A[col][j]/=d;
      for(int r=col+1;r<n;++r){ double f=A[r][col]; if(std::abs(f)<1e-20) continue; for(int j=col;j<=n;++j) A[r][j]-=f*A[col][j]; }
    }
    std::vector<double> x(n,0.0);
    for(int i=n-1;i>=0;--i){ double s=A[i][n]; for(int j=i+1;j<n;++j) s-=A[i][j]*x[j]; x[i]=s; }
    return {true,x};
  }

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

  static NewtonReport newton_solve(UnknownsRegistry& reg, ResidualSystem& sys, const NewtonOptions& opt={}){
    NewtonReport rep;
    size_t n=reg.size(), m=sys.size();
    if(n!=m){ rep.msg="DOF mismatch: unknowns="+std::to_string(n)+" equations="+std::to_string(m); if(opt.verbose){ std::cerr<<"[Newton] "<<rep.msg<<"\nUnknowns: "<<reg.unknowns_list()<<"\n"; for(size_t i=0;i<sys.names.size();++i) std::cerr<<"  Eq"<<i<<": "<<sys.names[i]<<"\n"; } return rep; }
    if(n==0){ rep.converged=true; rep.msg="Nothing to solve."; return rep; }

    for(int it=0; it<opt.max_iters; ++it){
      auto r = sys.eval();
      double nr = ResidualSystem::norm_inf(r);
      if(opt.verbose) std::cerr<<"[Newton] it "<<it<<" |r|_inf="<<nr<<"\n";
      if(nr<opt.tol_res){ rep.converged=true; rep.iters=it; rep.final_res=nr; rep.msg="Converged by residual."; return rep; }

      auto x0 = reg.pack_x();
      std::vector<std::vector<double>> J(m, std::vector<double>(n,0.0));
      for(size_t j=0;j<n;++j){
        Var* vj = reg.vars[j];
        double base=vj->value;
        double h=std::max(opt.fd_abs,std::abs(base)*opt.fd_rel);
        vj->value=base+h;
        auto rph=sys.eval();
        vj->value=base;
        for(size_t i=0;i<m;++i) J[i][j]=(rph[i]-r[i])/h;
      }

      std::vector<double> rhs(n,0.0);
      for(size_t i=0;i<n;++i) rhs[i]=-r[i];
      auto [ok, dx] = solve_linear(J, rhs);
      if(!ok){ rep.msg="Singular Jacobian."; if(opt.verbose) std::cerr<<"[Newton] "<<rep.msg<<"\n"; return rep; }

      double alpha=1.0, c1=1e-4; bool accepted=false;
      for(int ls=0; ls<15; ++ls){
        std::vector<double> xtry(n);
        for(size_t j=0;j<n;++j) xtry[j]=x0[j]+alpha*dx[j];
        reg.scatter_x(xtry);
        double nr_try = ResidualSystem::norm_inf(sys.eval());
        if(nr_try <= (1.0 - c1*alpha)*nr){ accepted=true; break; }
        alpha*=0.5;
      }
      if(!accepted){ reg.scatter_x(x0); for(size_t j=0;j<n;++j) x0[j]+=dx[j]; reg.scatter_x(x0); }

      double step_inf=0.0;
      for(size_t j=0;j<n;++j) step_inf=std::max(step_inf,std::abs(alpha*dx[j]));
      if(step_inf<opt.tol_step){ rep.converged=true; rep.iters=it+1; rep.final_res=ResidualSystem::norm_inf(sys.eval()); rep.msg="Converged by step."; return rep; }
    }
    rep.final_res = ResidualSystem::norm_inf(sys.eval());
    rep.msg="Max iterations reached.";
    return rep;
  }

  // --- Add this helper: builds FD Jacobian once and analyzes structure ---
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
  static int numeric_rank(std::vector<std::vector<double>> A, double pivot_tol,
                          std::vector<int>* pivots_out=nullptr) {
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

  static SystemAnalysis analyze_system(const UnknownsRegistry& reg,
                                      const ResidualSystem& sys,
                                      double fd_rel, double fd_abs,
                                      double tiny_row = 1e-14,
                                      double tiny_res = 1e-12) {
    SystemAnalysis a;
    const size_t m = sys.size();
    const size_t n = reg.size();
    a.J.assign(m, std::vector<double>(n, 0.0));
    a.r = sys.eval();
    a.row_max.assign(m, 0.0);
    a.col_max.assign(n, 0.0);

    // Build FD Jacobian (like Newton), but non-intrusive: we need a non-const reg to mutate,
    // so cast away constness in a controlled way (we'll restore values afterwards).
    UnknownsRegistry& reg_nc = const_cast<UnknownsRegistry&>(reg);
    const auto x0 = reg.pack_x();

    for (size_t j = 0; j < n; ++j) {
      Var* vj = reg_nc.vars[j];
      double base = vj->value;
      double h = std::max(fd_abs, std::abs(base) * fd_rel);
      vj->value = base + h;
      auto r_ph = sys.eval();
      vj->value = base;
      for (size_t i = 0; i < m; ++i) {
        double Jij = (r_ph[i] - a.r[i]) / h;
        a.J[i][j] = Jij;
        a.row_max[i] = std::max(a.row_max[i], std::abs(Jij));
        a.col_max[j] = std::max(a.col_max[j], std::abs(Jij));
      }
    }
    reg_nc.scatter_x(x0); // restore

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
