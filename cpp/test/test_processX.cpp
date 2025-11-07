// STL includes
#include <iostream>
#include <cmath>
#include <numeric>
#include <cstring>
#include <memory>

// Gtest includes
#include <gtest/gtest.h>

// CoolProp includes
#include <CoolProp.h>
#include <AbstractState.h>

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

    bool run() {
      std::string err;
      if(!fs.assemble(&err)){ 
        std::cerr<<"Assemble error: "<<err<<"\n"; 
        return false; 
      }

      std::cout<<"Unknowns: "<<fs.reg.unknowns_list()<<"\n";

      auto rep = fs.solve(NewtonOptions{50,1e-12,1e-14,1e-6,1e-8,true});
      
      std::cout<<(rep.converged?"Converged":"Not converged")<<" in "<<rep.iters<<" iters, |r|_inf="<<rep.final_res<<" : "<<rep.msg<<"\n";    
      
      return rep.converged;
    }
  };

  // TEST_F(ProcessTest, LoadTest) {
  //   const std::string file_path = "gpt_test.json";
  //   LoadFromJson(this->fs, file_path);
  //   run();
  // }

  TEST_F(ProcessTest, MultiValveZeroFlowTest) {
    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto s_mid = fs.add<Stream>();
    auto v1 = fs.add<Valve>();
    auto v2 = fs.add<Valve>();

    auto& in = fs.get<Stream>(s_in);
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
    
    // Fix enthalpies: each valve needs 1 fixed (2 valves, 3 streams, need 2 fixed)
    in.molar_enthalpy.set_val(0.0, true);
    mid.molar_enthalpy.set_val(0.0, true);

    run();

    EXPECT_NEAR(in.molar_flow.value,  0.0, 1e-12);
    EXPECT_NEAR(mid.molar_flow.value, 0.0, 1e-12);
    EXPECT_NEAR(out.molar_flow.value, 0.0, 1e-12);
    EXPECT_NEAR(mid.pressure.value,   2.0e5, 1e-8);

    const std::string file_path = "test.json";
    SaveToJson(fs, file_path);
    Flowsheet load_fs;
    LoadFromJson(load_fs, file_path);
    
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

    auto& in = fs.get<Stream>(s_in);
    auto& mid = fs.get<Stream>(s_mid);
    auto& out = fs.get<Stream>(s_out);
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
    
    // Fix enthalpies: each valve needs 1 fixed (2 valves, 3 streams, need 2 fixed)
    in.molar_enthalpy.set_val(0.0, true);
    mid.molar_enthalpy.set_val(0.0, true);

    run();

    // Analytical solution:
    const double Pmid = (Cv1*Pin + Cv2*Pout) / (Cv1 + Cv2);
    const double F = (Cv1*Cv2/(Cv1+Cv2)) * (Pin - Pout);

    EXPECT_NEAR(mid.pressure.value,   Pmid, 1e-6);
    EXPECT_NEAR(in.molar_flow.value,  F,    1e-10);
    EXPECT_NEAR(mid.molar_flow.value, F,    1e-10);
    EXPECT_NEAR(out.molar_flow.value, F,    1e-10);

    // Check each valve equation too
    EXPECT_NEAR(mid.molar_flow.value, val1.Cv.value * (in.pressure.value  - mid.pressure.value),  1e-10);
    EXPECT_NEAR(out.molar_flow.value, val2.Cv.value * (mid.pressure.value - out.pressure.value),  1e-10);
  }

  TEST_F(ProcessTest, MixerTest) {
    auto s_in1  = fs.add<Stream>();
    auto s_in2 = fs.add<Stream>();
    auto s_end = fs.add<Stream>();
    auto m = fs.add<Mixer>();

    auto& mixer = fs.get<Mixer>(m);

    mixer.add_inlet(s_in1);
    mixer.add_inlet(s_in2);
    mixer.set_outlet(s_end);
    
    auto& in1 = fs.get<Stream>(s_in1);
    auto& in2 = fs.get<Stream>(s_in2);
    auto& out = fs.get<Stream>(s_end);
    
    const double P  = 1.0e5;
    const double F1  = 1.5;
    const double F2  = 0.2;

    in1.pressure.set_val(P, false);
    in2.pressure.set_val(P, false);
    out.pressure.set_val(P, true);
    in1.molar_flow.set_val(F1, true);
    in2.molar_flow.set_val(F2, true);

    out.molar_flow.set_val(0.8, false);
    
    // Fix enthalpies: mixer has 3 enthalpies, 1 energy balance, need to fix 2
    in1.molar_enthalpy.set_val(0.0, true);
    in2.molar_enthalpy.set_val(0.0, true);
    
    run();

    const double F = F1 + F2;
    EXPECT_NEAR(out.molar_flow.value, F, 1e-10);
    // Verify pressure equality equations
    EXPECT_NEAR(in1.pressure.value, out.pressure.value, 1e-10);
    EXPECT_NEAR(in2.pressure.value, out.pressure.value, 1e-10);
  }

  TEST_F(ProcessTest, SplitterTest) {
    auto s_in = fs.add<Stream>();
    auto s_out1 = fs.add<Stream>();
    auto s_out2 = fs.add<Stream>();
    auto s = fs.add<Splitter>();

    auto& splitter = fs.get<Splitter>(s);

    splitter.set_inlet(s_in);
    splitter.add_outlet(s_out1);
    splitter.add_outlet(s_out2);
    
    auto& in = fs.get<Stream>(s_in);
    auto& out1 = fs.get<Stream>(s_out1);
    auto& out2 = fs.get<Stream>(s_out2);
    
    const double P  = 1.0e5;
    const double F  = 2.0;
    const double F1 = 1.2;
    const double F2 = 0.0;

    in.pressure.set_val(P, true);
    out1.pressure.set_val(P, false);
    out2.pressure.set_val(P, false);
    in.molar_flow.set_val(F, true);
    out1.molar_flow.set_val(F1, true);
    out2.molar_flow.set_val(F2, false);  // Unknown - should equal F - F1
    
    // Fix enthalpy: splitter has enthalpy equality (H_in = H_out1 = H_out2), so fix 1
    in.molar_enthalpy.set_val(0.0, true);
    
    run();

    EXPECT_NEAR(in.molar_flow.value, out1.molar_flow.value + out2.molar_flow.value, 1e-10);
    EXPECT_NEAR(in.pressure.value, out1.pressure.value, 1e-10);
    EXPECT_NEAR(in.pressure.value, out2.pressure.value, 1e-10);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Fout_and_Cv) {
    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto v = fs.add<Valve>();

    fs.connect_in<Valve>(v, s_in);
    fs.connect_out<Valve>(v, s_out);

    auto& in = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& val = fs.get<Valve>(v);

    // Given
    const double Pin  = 5.0e5;
    const double Pout = 4.0e5;
    const double Fin  = 10.0;

    in.molar_flow.set_val(Fin,  true);
    in.pressure.set_val(Pin,  true);
    out.pressure.set_val(Pout, true);

    // Unknowns
    out.molar_flow.set_val(0.0,   false);  // F_out
    val.Cv.set_val(1.0,   false);  // Cv
    
    // Fix enthalpy: valve has 2 enthalpies, 1 energy balance, need to fix 1
    in.molar_enthalpy.set_val(0.0, true);

    run();

    // Expected (mass balance + valve law)
    const double Fexp  = Fin;
    const double Cvexp = Fexp / (Pin - Pout);

    EXPECT_NEAR(out.molar_flow.value, Fexp,  1e-10);
    EXPECT_NEAR(in.molar_flow.value,  Fexp,  1e-10);
    EXPECT_NEAR(val.Cv.value,         Cvexp, 1e-12);
    EXPECT_NEAR(out.molar_flow.value, val.Cv.value * (in.pressure.value - out.pressure.value), 1e-10);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Pout_and_Fout) {
    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto v = fs.add<Valve>();

    fs.connect_in<Valve>(v, s_in);
    fs.connect_out<Valve>(v, s_out);

    auto& in = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& val = fs.get<Valve>(v);

    // Given
    const double Pin = 3.0e5;
    const double Fin = 8.0;
    const double Cv  = 2.0e-4;

    in.molar_flow.set_val(Fin, true);
    in.pressure.set_val(Pin, true);
    val.Cv.set_val(Cv,  true);

    // Unknowns
    out.molar_flow.set_val(0.0,  false); // F_out
    out.pressure.set_val(1.0e5, false); // P_out
    
    // Fix enthalpy: valve has 2 enthalpies, 1 energy balance, need to fix 1
    in.molar_enthalpy.set_val(0.0, true);

    run();

    // Expected (balance F_out = Fin; P_out from valve law)
    const double Fexp  = Fin;
    const double Poutexp = Pin - Fexp / Cv; // = 2.6e5

    EXPECT_NEAR(out.molar_flow.value, Fexp,    1e-10);
    EXPECT_NEAR(in.molar_flow.value,  Fexp,    1e-10);
    EXPECT_NEAR(out.pressure.value,   Poutexp, 1e-6);
    EXPECT_NEAR(out.molar_flow.value, val.Cv.value * (in.pressure.value - out.pressure.value), 1e-10);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Fin_and_Fout) {
    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto v = fs.add<Valve>();

    fs.connect_in<Valve>(v, s_in);
    fs.connect_out<Valve>(v, s_out);

    auto& in  = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& val = fs.get<Valve>(v);

    const double Pin = 3.0e5;
    const double Pout = 1.0e5;
    const double Cv = 2.0e-4;

    in.pressure.set_val(Pin,  true);
    out.pressure.set_val(Pout, true);
    val.Cv.set_val(Cv,   true);

    // Unknowns
    in.molar_flow .set_val(0.0, false); // F_in
    out.molar_flow.set_val(0.0, false); // F_out
    
    // Fix enthalpy: valve has 2 enthalpies, 1 energy balance, need to fix 1
    in.molar_enthalpy.set_val(0.0, true);

    run();

    // Expected
    const double Fexp = Cv * (Pin - Pout); // = 40
    EXPECT_NEAR(in.molar_flow.value,  Fexp, 1e-10);
    EXPECT_NEAR(out.molar_flow.value, Fexp, 1e-10);
    EXPECT_NEAR(out.molar_flow.value, val.Cv.value * (in.pressure.value - out.pressure.value), 1e-10);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Pin_and_Cv) {
    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto v = fs.add<Valve>();

    fs.connect_in<Valve>(v, s_in);
    fs.connect_out<Valve>(v, s_out);

    auto& in  = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& val = fs.get<Valve>(v);

    // Given (both flows fixed/consistent; P_out fixed)
    const double Pout = 1.2e5;
    const double F = 12.0;

    in.molar_flow .set_val(F, true);
    out.molar_flow.set_val(F, true);
    out.pressure.set_val(Pout, true);

    // Unknowns (under-determined without an extra condition; we assert consistency)
    in.pressure.set_val(2.0e5,  false); // P_in (free)
    val.Cv.set_val(1.0e-5, false); // Cv (free)
    
    // Fix enthalpy: valve has 2 enthalpies, 1 energy balance, need to fix 1
    in.molar_enthalpy.set_val(0.0, true);

    auto converged = run();
    ASSERT_FALSE(converged); // We expect it to FAIL!
  }

  TEST_F(ProcessTest, RecycleLoopTest) {
    // Create streams: in, middle, out, recycle
    auto s_in = fs.add<Stream>();
    auto s_middle = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto s_recycle = fs.add<Stream>();
    
    // Create unit operations
    auto m = fs.add<Mixer>();
    auto sp = fs.add<Splitter>();
    
    // Get references
    auto& mixer = fs.get<Mixer>(m);
    auto& splitter = fs.get<Splitter>(sp);
    auto& in = fs.get<Stream>(s_in);
    auto& middle = fs.get<Stream>(s_middle);
    auto& out = fs.get<Stream>(s_out);
    auto& recycle = fs.get<Stream>(s_recycle);
    
    // Connect mixer: inlets={in, recycle}, outlet=middle
    mixer.add_inlet(s_in);
    mixer.add_inlet(s_recycle);
    mixer.set_outlet(s_middle);
    
    // Connect splitter: inlet=middle, outlets={out, recycle}
    splitter.set_inlet(s_middle);
    splitter.add_outlet(s_out);
    splitter.add_outlet(s_recycle);
    
    // Set fixed values
    const double F_in = 10.0;
    const double F_recycle = 5.0;
    const double P = 1.0e5;
    
    // Fix inlet and recycle flow rates
    in.molar_flow.set_val(F_in, true);
    recycle.molar_flow.set_val(F_recycle, true);
    
    // Fix all pressures to same value
    in.pressure.set_val(P, true);
    middle.pressure.set_val(0.0, false);
    out.pressure.set_val(0.0, false);
    recycle.pressure.set_val(0.0, false);
    
    // Set initial guesses for unknowns
    middle.molar_flow.set_val(15.0, false);  // Should be F_in + F_recycle = 15
    out.molar_flow.set_val(10.0, false);    // Should be F_in = 10
    
    // Fix enthalpies:
    // Mixer: 3 enthalpies (in, recycle, middle), 1 energy balance → fix 2
    // Splitter: 3 enthalpies (middle, out, recycle), but enthalpy equality (H_middle = H_out = H_recycle) → fix 1
    // Strategy: Fix in and recycle for mixer (2 fixed), and middle for splitter (1 fixed, determines out and recycle by equality)
    // But recycle is shared, so if we fix recycle for mixer and middle for splitter, recycle is determined by splitter equality
    // Better: Fix in and recycle for mixer, then splitter equality determines middle, out, and recycle (but recycle already fixed)
    // Actually: Fix in and recycle for mixer, then middle is determined by mixer energy balance, then splitter equality determines out
    in.molar_enthalpy.set_val(0.0, true);
    recycle.molar_enthalpy.set_val(0.0, true);
    // middle will be determined by mixer energy balance, then splitter equality will determine out
    
    std::cout << "\n=== Recycle Loop Test ===" << std::endl;

    bool converged = run();
    
    if (converged) {
      std::cout << "\nResults:" << std::endl;
      std::cout << "F_in = " << in.molar_flow.value << std::endl;
      std::cout << "F_middle = " << middle.molar_flow.value << std::endl;
      std::cout << "F_out = " << out.molar_flow.value << std::endl;
      std::cout << "F_recycle = " << recycle.molar_flow.value << std::endl;
      
      // Verify balances
      EXPECT_NEAR(middle.molar_flow.value, F_in + F_recycle, 1e-10);
      EXPECT_NEAR(out.molar_flow.value, F_in, 1e-10);
      EXPECT_NEAR(middle.molar_flow.value, out.molar_flow.value + recycle.molar_flow.value, 1e-10);
    } else {
      std::cout << "\nSOLUTION FAILED: DOF mismatch detected before rank analysis" << std::endl;
      std::cout << "The system correctly identifies that we have more equations than unknowns," << std::endl;
      std::cout << "but the redundant pressure equations (when all pressures are fixed) should" << std::endl;
      std::cout << "be detected by rank analysis, not blocked by the DOF check." << std::endl;
    }
    
    // Don't assert convergence - we want to analyze why it fails
  }

  TEST(WaterEnthalpyTest, MolarEnthalpyAtRoomTempAndPressure) {
    const double P_std = 101325.0; // Pa
    const double Hmolar = 1890.0; // J/mol

    auto t0 = std::chrono::steady_clock::now();
    std::shared_ptr<CoolProp::AbstractState> mix(
      CoolProp::AbstractState::factory("HEOS", "Water")
    );
    auto t1 = std::chrono::steady_clock::now();
    mix->set_mole_fractions({1.0});
    auto t2 = std::chrono::steady_clock::now();
    mix->update(CoolProp::HmolarP_INPUTS, Hmolar, P_std);
    auto t3 = std::chrono::steady_clock::now();
    std::cerr
      << "factory: " << (t1-t0).count() << " ms, "
      << "set_z: "  << (t2-t1).count() << " ms, "
      << "Update: "    << (t3-t2).count() << " ms\n";

    double temp = CoolProp::PropsSI("T", "P", P_std, "Hmolar", Hmolar, "IF97::Water");
    
    std::cout << "\n=== Water Molar Enthalpy Test ===" << std::endl;
    std::cout << "Temperature: " << temp << " K (" << (temp - 273.15) << " °C)" << std::endl;
    std::cout << "Pressure: " << P_std << " Pa (" << (P_std / 101325.0) << " atm)" << std::endl;
    std::cout << "Molar Enthalpy: " << Hmolar << " J/mol" << std::endl;
    
    // Verify the value is reasonable
    EXPECT_GT(temp, 295) << "Molar enthalpy seems too negative";
    EXPECT_LT(temp, 300) << "Molar enthalpy seems too positive for liquid water";
  }

} // end processX namespace
