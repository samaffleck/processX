// STL includes
#include <iostream>
#include <cmath>

// Gtest includes
#include <gtest/gtest.h>

// CoolProp includes
#include <CoolProp.h>

// STL includes
#include <cstring>


namespace processX {

  static inline void expect_near_rel(double actual, double expected, double rel = 1e-3) {
      double tol = std::max(1.0, std::abs(expected)) * rel;
      EXPECT_NEAR(actual, expected, tol);
  }

  TEST(CoolProp_Basics, Air_P_from_T_and_rho) {
      // P(T, rho, "Air") -> ~1 bar at ~1.2 kg/m3 and 298 K
      double P = CoolProp::PropsSI("P", "T", 298.0, "D", 1.2, "Air");
      EXPECT_NEAR(P, 101325.0, 3000.0); // within ~3 kPa
  }

  TEST(CoolProp_Saturation, Water_Tsat_at_1bar) {
      // Tsat from P & quality Q (0 or 1 -> same Tsat)
      double p = 101325.0;
      double Tsat0 = CoolProp::PropsSI("T", "P", p, "Q", 0, "Water");
      double Tsat1 = CoolProp::PropsSI("T", "P", p, "Q", 1, "Water");
      EXPECT_NEAR(Tsat0, 373.124, 0.05);
      EXPECT_NEAR(Tsat1, Tsat0, 1e-6);
  }

  TEST(CoolProp_TwoPhase, Water_rhoL_rhoV_at_Tsat) {
      double p = 101325.0;
      double Tsat = CoolProp::PropsSI("T", "P", p, "Q", 0, "Water");

      double rhoL = CoolProp::PropsSI("D", "T", Tsat, "Q", 0, "Water"); // saturated liquid
      double rhoV = CoolProp::PropsSI("D", "T", Tsat, "Q", 1, "Water"); // saturated vapor

      EXPECT_GT(rhoL, 900.0);          // ~958 kg/m3 near 100 C
      EXPECT_LT(rhoV, 1.0);            // ~0.6 kg/m3 near 100 C
      EXPECT_GT(rhoL, rhoV);
  }

  TEST(CoolProp_TwoPhase, Water_LatentHeat_at_Tsat) {
      double p = 101325.0;
      double Tsat = CoolProp::PropsSI("T", "P", p, "Q", 0, "Water");

      // Enthalpy per mass (J/kg) at quality 0 and 1
      double hL = CoolProp::PropsSI("Hmass", "T", Tsat, "Q", 0, "Water");
      double hV = CoolProp::PropsSI("Hmass", "T", Tsat, "Q", 1, "Water");
      double hfg = hV - hL;

      // Latent heat at 100 C ~ 2257 kJ/kg
      EXPECT_NEAR(hfg, 2.257e6, 1.2e5); // ±120 kJ/kg band is generous but reliable across builds
  }

} // end processX namespace
