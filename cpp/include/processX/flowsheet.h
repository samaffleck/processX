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
    for(auto* v: valves){ std::string why; if(!v->validate(&why)){ if(err)*err="Valve invalid: "+why; return false; } v->register_unknowns(reg); }
    for(auto* v: valves) v->add_equations(sys);
    if(reg.size()!=sys.size()){ if(err)*err="DOF mismatch: unknowns="+std::to_string(reg.size())+" equations="+std::to_string(sys.size()); return false; }
    return true;
  }

  NewtonReport solve(const NewtonOptions& opt={}) { return newton_solve(reg, sys, opt); }

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
