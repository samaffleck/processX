#pragma once

#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// ---------- Core variable / registry ----------
struct Var {
  std::string name;
  double value{0.0};
  bool fixed{false};
  Var() = default;
  Var(std::string n, double v, bool f=false) : name(std::move(n)), value(v), fixed(f) {}
};

struct UnknownsRegistry {
  std::vector<Var*> vars;
  void clear() { vars.clear(); }
  void register_var(Var& v) { if (!v.fixed) vars.push_back(&v); }
  std::size_t size() const { return vars.size(); }
  std::vector<double> pack_x() const { std::vector<double> x; x.reserve(vars.size()); for (auto* v: vars) x.push_back(v->value); return x; }
  void scatter_x(const std::vector<double>& x){ assert(x.size()==vars.size()); for (size_t i=0;i<vars.size();++i) vars[i]->value = x[i]; }
  std::string unknowns_list() const { std::string s; for (size_t i=0;i<vars.size();++i){ if(i)s+=", "; s+=vars[i]->name;} return s.empty()?"(none)":s; }
};

struct ResidualSystem {
  std::vector<std::function<double()>> residuals;
  std::vector<std::string> names;
  void clear(){ residuals.clear(); names.clear(); }
  void add(std::string nm, std::function<double()> r){ names.push_back(std::move(nm)); residuals.push_back(std::move(r)); }
  std::size_t size() const { return residuals.size(); }
  std::vector<double> eval() const { std::vector<double> r; r.reserve(residuals.size()); for (auto& f: residuals) r.push_back(f()); return r; }
  static double norm_inf(const std::vector<double>& r){ double m=0; for(double v:r) m=std::max(m,std::abs(v)); return m; }
};

// ---------- Dense linear solve (tiny Gaussian elim) ----------
static std::pair<bool,std::vector<double>>
solve_linear(std::vector<std::vector<double>> A, std::vector<double> b){
  int n=(int)b.size(); for(int i=0;i<n;++i) A[i].push_back(b[i]);
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

// ---------- Domain ----------
struct Stream {
  Var molar_flow{"stream.F", 0.0, false}; // mol/s
  Var pressure{"stream.P", 0.0, false};   // Pa
  Var temperature{"stream.T", 300.0, true}; // not used here
};

struct Valve {
  Stream* inlet{nullptr};
  Stream* outlet{nullptr};
  Var Cv{"valve.Cv", 1.0, false};

  bool validate(std::string* why=nullptr) const {
    if(!inlet){ if(why)*why="inlet not set"; return false; }
    if(!outlet){ if(why)*why="outlet not set"; return false; }
    return true;
  }

  void register_unknowns(UnknownsRegistry& reg){
    reg.register_var(inlet->molar_flow);
    reg.register_var(outlet->molar_flow);
    reg.register_var(inlet->pressure);
    reg.register_var(outlet->pressure);
    reg.register_var(Cv);
  }

  void add_equations(ResidualSystem& sys){
    sys.add("Valve balance: Fin - Fout = 0", [&](){
      return inlet->molar_flow.value - outlet->molar_flow.value;
    });
    sys.add("Valve PF: Fout - Cv*(Pin - Pout) = 0", [&](){
      return outlet->molar_flow.value - Cv.value * (inlet->pressure.value - outlet->pressure.value);
    });
  }
};

// ---------- Flowsheet ----------
struct Flowsheet {
  std::vector<Valve*> valves;
  UnknownsRegistry reg;
  ResidualSystem sys;

  bool assemble(std::string* err=nullptr){
    reg.clear(); sys.clear();
    for (auto* v: valves) {
      std::string why;
      if (!v->validate(&why)) { if (err) *err = "Valve invalid: " + why; return false; }
      v->register_unknowns(reg);
      v->add_equations(sys); // equations are always present
    }
    // Basic DOF
    if (reg.size() != sys.size()) {
      if (err) *err = "DOF mismatch: unknowns=" + std::to_string(reg.size()) +
                      " equations=" + std::to_string(sys.size());
      return false;
    }

    // Structural analysis BEFORE solving
    auto a = analyze_system(reg, sys, /*fd_rel=*/1e-6, /*fd_abs=*/1e-8);

    // Inconsistent equations (no dependence on any unknown, but nonzero residual)
    if (!a.inconsistent_eqs.empty()) {
      if (err) {
        *err = "Inconsistent equation(s) given chosen unknowns:\n";
        for (int idx : a.inconsistent_eqs)
          *err += "  - " + sys.names[(size_t)idx] + " (residual=" + std::to_string(a.r[(size_t)idx]) + ")\n";
      }
      return false;
    }

    // Redundant equations (identities w.r.t. unknowns)
    if (!a.redundant_eqs.empty()) {
      if (err) {
        *err = "Redundant equation(s) detected (independent of unknowns and already satisfied):\n";
        for (int idx : a.redundant_eqs)
          *err += "  - " + sys.names[(size_t)idx] + "\n";
        *err += "Result: Jacobian rank " + std::to_string(a.rank) +
                " < " + std::to_string(reg.size()) + " (rank-deficient). Adjust unknowns.";
      }
      return false;
    }

    // Rank deficiency (e.g., two different equations but linearly dependent)
    if (a.rank < (int)reg.size()) {
      if (err) {
        *err = "System is rank-deficient: rank(J)=" + std::to_string(a.rank) +
               " < unknowns=" + std::to_string(reg.size()) + ".";
        // Optional: list unconstrained unknowns
        if (!a.unconstrained_unknowns.empty()) {
          *err += "\nUnconstrained unknown(s):";
          for (int j : a.unconstrained_unknowns)
            *err += " " + reg.vars[(size_t)j]->name;
        }
      }
      return false;
    }

    return true; // structurally OK
  }

  NewtonReport solve(const NewtonOptions& opt={}) {
    return newton_solve(reg, sys, opt);
  }

  static void print_valve(const Valve& v){
    auto pf=[&](const Var& x){ return x.fixed?"(fixed)":"(free)"; };
    std::cout<<"  inlet.F = "<<v.inlet->molar_flow.value<<" "<<pf(v.inlet->molar_flow)<<"\n";
    std::cout<<"  outlet.F = "<<v.outlet->molar_flow.value<<" "<<pf(v.outlet->molar_flow)<<"\n";
    std::cout<<"  inlet.P = "<<v.inlet->pressure.value<<" "<<pf(v.inlet->pressure)<<"\n";
    std::cout<<"  outlet.P = "<<v.outlet->pressure.value<<" "<<pf(v.outlet->pressure)<<"\n";
    std::cout<<"  Cv = "<<v.Cv.value<<" "<<pf(v.Cv)<<"\n";
  }
};

// ---------- Demo ----------
static void run_case(const std::string& title,
                     std::function<void(Valve&, Stream&, Stream&)> config){
  std::cout<<"\n=== "<<title<<" ===\n";
  Stream in;  in.molar_flow.name="in.F";  in.pressure.name="in.P";
  Stream out; out.molar_flow.name="out.F"; out.pressure.name="out.P";
  Valve v; v.inlet=&in; v.outlet=&out; v.Cv.name="Cv";

  // User config (choose unknowns by marking fixed=false)
  config(v, in, out);

  Flowsheet fs; fs.valves.push_back(&v);
  std::string err;
  if(!fs.assemble(&err)){ std::cerr<<"Assemble error: "<<err<<"\n"; Flowsheet::print_valve(v); return; }

  std::cout<<"Unknowns: "<<fs.reg.unknowns_list()<<"\n";
  auto rep = fs.solve(NewtonOptions{50,1e-12,1e-14,1e-6,1e-8,true});
  std::cout<<(rep.converged?"Converged":"Not converged")<<" in "<<rep.iters<<" iters, |r|_inf="<<rep.final_res<<" : "<<rep.msg<<"\n";
  Flowsheet::print_valve(v);
}
