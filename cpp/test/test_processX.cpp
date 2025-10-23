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

  TEST(ProcessX, SimpleSetup) {
    // Example 1: Unknowns {F_out, Cv}. Fix Fin, Pin, Pout.
    run_valve_case("Unknowns: F_out, Cv (Fin, Pin, Pout fixed)", [](Valve& v, Stream& in, Stream& out){
      in.molar_flow = {"in.F", 10.0, true};
      in.pressure   = {"in.P", 5.0e5, true};
      out.pressure  = {"out.P", 4.0e5, true};
      out.molar_flow= {"out.F", 0.0,  false}; // unknown
      v.Cv          = {"Cv",    1.0,false}; // unknown
      // Balance will enforce out.F == in.F at convergence; PF sets Cv accordingly.
    });

    // Example 2: Unknowns {P_out, F_out}. Fix Fin, Pin, Cv.
    run_valve_case("Unknowns: P_out, F_out (Fin, Pin, Cv fixed)", [](Valve& v, Stream& in, Stream& out){
      in.molar_flow = {"in.F", 8.0,   true};
      in.pressure   = {"in.P", 3.0e5, true};
      v.Cv          = {"Cv",   2.0e-4,true};
      out.molar_flow= {"out.F",0.0,   false}; // unknown
      out.pressure  = {"out.P",1.0e5, false}; // unknown
    });

    // Example 3: Unknowns {P_in, Cv}. Fix F (both ends equal by balance) and P_out.
    run_valve_case("Unknowns: P_in, Cv (F fixed, P_out fixed)", [](Valve& v, Stream& in, Stream& out){
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
