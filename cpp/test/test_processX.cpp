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


namespace processX {

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

  TEST(CoolProp_VLE_Mixture, MethaneEthane_TPFlash_xy_and_quality) {
    using namespace CoolProp;

    const double T = 190.0;
    const std::vector<double> z = {0.7, 0.3}; 
    const char* mix_props = "HEOS::Methane[0.7]&Ethane[0.3]";
    const double P = 16.4765e5; // [pa]

    auto AS = AbstractState::factory("HEOS", "Methane&Ethane");
    AS->set_mole_fractions(z);
    AS->update(PT_INPUTS, P, T);

    const double Q = AS->Q();
    ASSERT_TRUE(std::isfinite(Q));
    EXPECT_GT(Q, 0.0);
    EXPECT_LT(Q, 1.0);

    const std::vector<double> x = AS->mole_fractions_liquid();
    const std::vector<double> y = AS->mole_fractions_vapor();

    ASSERT_EQ(x.size(), z.size());
    ASSERT_EQ(y.size(), z.size());
    expect_prob_vector(x);
    expect_prob_vector(y);

    EXPECT_GT(y[0], x[0]);
    EXPECT_LT(y[1], x[1]);

    std::cout << "\nTP flash @ T=" << T << " K, P=" << P/1e5 << " bar\n"
              << "Q (vapor mass fraction) = " << Q << "\n"
              << "x_liq = [" << x[0] << ", " << x[1] << "]\n"
              << "y_vap = [" << y[0] << ", " << y[1] << "]\n";
  }

  TEST(CoolProp_Saturation, Water_Tsat_at_1bar) {
      // Tsat from P & quality Q (0 or 1 -> same Tsat)
      double p = 101325.0;
      double Tsat0 = CoolProp::PropsSI("T", "P", p, "Q", 0, "IF97::Water");
      double Tsat1 = CoolProp::PropsSI("T", "P", p, "Q", 1, "IF97::Water");
      EXPECT_NEAR(Tsat0, 373.124, 0.05);
      EXPECT_NEAR(Tsat1, Tsat0, 1e-6);
  }

  TEST(CoolProp_TwoPhase, Water_rhoL_rhoV_at_Tsat) {
      double p = 101325.0;
      double Tsat = CoolProp::PropsSI("T", "P", p, "Q", 0, "IF97::Water");

      double rhoL = CoolProp::PropsSI("D", "T", Tsat, "Q", 0, "IF97::Water"); // saturated liquid
      double rhoV = CoolProp::PropsSI("D", "T", Tsat, "Q", 1, "IF97::Water"); // saturated vapor

      EXPECT_GT(rhoL, 900.0);          // ~958 kg/m3 near 100 C
      EXPECT_LT(rhoV, 1.0);            // ~0.6 kg/m3 near 100 C
      EXPECT_GT(rhoL, rhoV);
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
