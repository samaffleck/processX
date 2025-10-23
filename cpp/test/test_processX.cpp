// STL includes
#include <iostream>
#include <cmath>

// Gtest includes
#include <gtest/gtest.h>

// CoolProp includes
#include <CoolProp.h>
#include <AbstractState.h>

// STL includes
#include <cstring>

// ProcessX includes
#include <processX/flowsheet.h>


namespace px {

  static inline double sum(const std::vector<double>& v) {
    return std::accumulate(v.begin(), v.end(), 0.0);
  }

  static inline void expect_prob_vector(const std::vector<double>& v, double tol = 1e-6) {
    ASSERT_FALSE(v.empty());
    for (double xi : v) {
      EXPECT_GE(xi, -1e-12);
      EXPECT_LE(xi, 1.0 + 1e-12);
    }
    EXPECT_NEAR(sum(v), 1.0, tol);
  }

  class ProcessTest : public ::testing::Test {
    protected:
      Flowsheet fs;

    void run() {
      std::string err;
      if(!fs.assemble(&err)){ 
        std::cerr<<"Assemble error: "<<err<<"\n"; 
        return; 
      }

      std::cout<<"Unknowns: "<<fs.reg.unknowns_list()<<"\n";

      auto rep = fs.solve(NewtonOptions{50,1e-12,1e-14,1e-6,1e-8,true});
      
      std::cout<<(rep.converged?"Converged":"Not converged")<<" in "<<rep.iters<<" iters, |r|_inf="<<rep.final_res<<" : "<<rep.msg<<"\n";    
    }
  };

  TEST_F(ProcessTest, MultiValveZeroFlowTest) {
    auto s_in  = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto s_mid = fs.add<Stream>();
    auto v1 = fs.add<Valve>();
    auto v2 = fs.add<Valve>();

    auto& in  = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& mid = fs.get<Stream>(s_mid);
    auto& val_1 = fs.get<Valve>(v1);
    auto& val_2 = fs.get<Valve>(v2);

    fs.connect_in<Valve>(v1, s_in);
    fs.connect_out<Valve>(v1, s_mid);

    fs.connect_in<Valve>(v2, s_mid);
    fs.connect_out<Valve>(v2, s_out);

    val_1.Cv.set_val(1.0e-4, true);
    val_2.Cv.set_val(3.0e-4, true);
    in.pressure.set_val(2e5, true);
    out.pressure.set_val(2e5, true);
    
    in.molar_flow.set_val(1, false);
    out.molar_flow.set_val(1, false);
    mid.molar_flow.set_val(1, false);
    mid.pressure.set_val(1, false);

    run();

    EXPECT_NEAR(in.molar_flow.value,  0.0, 1e-12);
    EXPECT_NEAR(mid.molar_flow.value, 0.0, 1e-12);
    EXPECT_NEAR(out.molar_flow.value, 0.0, 1e-12);
    EXPECT_NEAR(mid.pressure.value,   2.0e5, 1e-8);
  }

  TEST_F(ProcessTest, MultiValveTest) {
    auto s_in  = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto s_mid = fs.add<Stream>();
    auto v1 = fs.add<Valve>();
    auto v2 = fs.add<Valve>();

    fs.connect_in<Valve>(v1, s_in);
    fs.connect_out<Valve>(v1, s_mid);
    fs.connect_in<Valve>(v2, s_mid);
    fs.connect_out<Valve>(v2, s_out);

    auto& in   = fs.get<Stream>(s_in);
    auto& mid  = fs.get<Stream>(s_mid);
    auto& out  = fs.get<Stream>(s_out);
    auto& val1 = fs.get<Valve>(v1);
    auto& val2 = fs.get<Valve>(v2);

    const double Pin  = 5.0e5;
    const double Pout = 1.0e5;
    const double Cv1  = 1.0e-4;
    const double Cv2  = 3.0e-4;

    val1.Cv.set_val(Cv1, true);
    val2.Cv.set_val(Cv2, true);
    in.pressure.set_val(Pin,  true);
    out.pressure.set_val(Pout, true);

    in.molar_flow.set_val(0.0, false);
    mid.molar_flow.set_val(0.0, false);
    out.molar_flow.set_val(0.0, false);
    mid.pressure.set_val((Pin + Pout) * 0.5, false);

    run();

    // Analytical solution:
    const double Pmid = (Cv1*Pin + Cv2*Pout) / (Cv1 + Cv2);
    const double F    = (Cv1*Cv2/(Cv1+Cv2)) * (Pin - Pout);

    EXPECT_NEAR(mid.pressure.value,   Pmid, 1e-6);
    EXPECT_NEAR(in.molar_flow.value,  F,    1e-10);
    EXPECT_NEAR(mid.molar_flow.value, F,    1e-10);
    EXPECT_NEAR(out.molar_flow.value, F,    1e-10);

    // Check each valve equation too
    EXPECT_NEAR(mid.molar_flow.value, val1.Cv.value * (in.pressure.value  - mid.pressure.value),  1e-10);
    EXPECT_NEAR(out.molar_flow.value, val2.Cv.value * (mid.pressure.value - out.pressure.value),  1e-10);
  }

  TEST(ProcessX, SimpleSetup) {
    run_valve_case("Unknowns: F_out, Cv\nFixed: Fin, Pin, Pout", [](Valve& v, Stream& in, Stream& out){
      in.molar_flow = {"in.F", 10.0, true};
      in.pressure   = {"in.P", 5.0e5, true};
      out.pressure  = {"out.P", 4.0e5, true};
      out.molar_flow= {"out.F", 0.0,  false}; // unknown
      v.Cv          = {"Cv",    1.0,false}; // unknown
      // Balance will enforce out.F == in.F at convergence; PF sets Cv accordingly.
    });

    run_valve_case("Unknowns: P_out, F_out\nFixed: Fin, Pin, Cv", [](Valve& v, Stream& in, Stream& out){
      in.molar_flow = {"in.F", 8.0,   true};
      in.pressure   = {"in.P", 3.0e5, true};
      v.Cv          = {"Cv",   2.0e-4,true};
      out.molar_flow= {"out.F",0.0,   false}; // unknown
      out.pressure  = {"out.P",1.0e5, false}; // unknown
    });

    run_valve_case("Unknowns: F_in, F_out \nFixed: Pout, Pin, Cv", [](Valve& v, Stream& in, Stream& out){
      out.pressure  = {"out.P",1.0e5, true}; // unknown
      in.pressure   = {"in.P", 3.0e5, true};
      v.Cv          = {"Cv",   2.0e-4,true};
      out.molar_flow= {"out.F",0.0,   false}; // unknown
      in.molar_flow = {"in.F", 8.0,   false};
    });

    run_valve_case("Unknowns: P_in, Cv\nFixed: F_in, F_out, P_out)", [](Valve& v, Stream& in, Stream& out){
      in.molar_flow = {"in.F", 12.0,  true};
      out.molar_flow= {"out.F",12.0,  true};  // redundant with balance target; consistent
      out.pressure  = {"out.P",1.2e5, true};
      in.pressure   = {"in.P", 2.0e5, false}; // unknown
      v.Cv          = {"Cv",   1.0e-5,false}; // unknown
    });
  }

  TEST(CoolProp_TwoPhase, Water_LatentHeat_at_Tsat) {
    double p = 101325.0;
    double Tsat = CoolProp::PropsSI("T", "P", p, "Q", 0, "IF97::Water");

    // Enthalpy per mass (J/kg) at quality 0 and 1
    double hL = CoolProp::PropsSI("Hmass", "T", Tsat, "Q", 0, "IF97::Water");
    double hV = CoolProp::PropsSI("Hmass", "T", Tsat, "Q", 1, "IF97::Water");
    double hfg = hV - hL;

    // Latent heat at 100 C ~ 2257 kJ/kg
    EXPECT_NEAR(hfg, 2.257e6, 1.2e5); // ±120 kJ/kg band is generous but reliable across builds
  }

} // end processX namespace
