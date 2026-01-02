// STL includes
#include <iostream>
#include <cmath>
#include <numeric>
#include <cstring>
#include <memory>
#include <filesystem>

// Gtest includes
#include <gtest/gtest.h>

// CoolProp includes
#include <CoolProp.h>
#include <AbstractState.h>

// ProcessX includes
#include <processX/flowsheet.h>
#include <processX/user_data.h>
#include <processX/kinsol_solver.h>


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

  void SaveTest(const std::string& test_name, Flowsheet& flowsheet) {
    // Create directory structure: public/Examples/
    std::filesystem::path examples_dir = "../../../public/Examples";
    
    std::filesystem::create_directories(examples_dir);
    
    // Save JSON file: public/Examples/test_name.json
    std::string json_filename = test_name + ".json";
    std::filesystem::path json_path = examples_dir / json_filename;
    
    std::ofstream json_file(json_path);
    if (json_file.is_open()) {
      cereal::JSONOutputArchive archive(json_file);
      archive(cereal::make_nvp("Flowsheet_Data", flowsheet));
    }
  }

  class ProcessTest : public ::testing::Test {
    protected:
      UserData data;

    bool run() {
      std::string err;
      if(!data.fs.assemble(&err)){ 
        std::cerr<<"Assemble error: "<<err<<"\n"; 
        return false; 
      }

      std::cout<<"Unknowns: "<<data.fs.reg.GetFeeVariableNames()<<"\n";

      // Set solver options
      data.newton_options = NewtonOptions{200,1e-12,1e-14,1e-6,1e-8,true};
      auto rep = newton_solve(data);
      
      std::cout<<(rep.converged?"Converged":"Not converged")<<" in "<<rep.iters<<" iters, |r|_inf="<<rep.final_res<<" : "<<rep.msg<<"\n";    
      
      return rep.converged;
    }

    // Helper function to initialize N2/O2 fluid package (79% N2, 21% O2)
    size_t init_air_fluid() {
      size_t fluid_id = data.fs.fluids.AddFluidPackage({"N2", "O2"}, "HEOS");
      auto fluid = data.fs.fluids.GetFluidPackage(fluid_id);
      if (fluid) {
        fluid->set_mole_fractions({0.79, 0.21}); // 79% N2, 21% O2
        // Build phase envelope after setting mole fractions (required for mixtures)
        try {
          fluid->build_phase_envelope("none");
        } catch (...) {
          // If phase envelope build fails, continue anyway
        }
      }
      return fluid_id;
    }
    
    // Helper function to initialize stream mole fractions to match fluid package composition
    // If fix_composition is true, the mole fractions will be fixed (not solved for)
    void init_stream_composition(Stream& stream, size_t fluid_id, bool fix_composition = false) {
      auto components = data.fs.fluids.GetComponents(fluid_id);
      if (components.size() == 2) {
        // For N2/O2 mixture, set to 79% N2, 21% O2
        stream.initialize_composition(components.size(), components);
        if (stream.mole_fractions.size() == 2) {
          stream.mole_fractions[0].SetValue(0.79, fix_composition); // N2
          stream.mole_fractions[1].SetValue(0.21, fix_composition); // O2
        }
      } else {
        // For other mixtures, use equal fractions
        stream.initialize_composition(components.size(), components);
        if (fix_composition) {
          // Fix all mole fractions if requested
          for (auto& x : stream.mole_fractions) {
            x.is_fixed_ = true;
          }
        }
      }
    }
  };


  TEST_F(ProcessTest, MultiValveZeroFlowTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    
    auto s_in = data.fs.add<Stream>();
    auto s_out = data.fs.add<Stream>();
    auto s_mid = data.fs.add<Stream>();
    auto v1 = data.fs.add<Valve>();
    auto v2 = data.fs.add<Valve>();

    auto& in = data.fs.get<Stream>(s_in);
    auto& out = data.fs.get<Stream>(s_out);
    auto& mid = data.fs.get<Stream>(s_mid);
    auto& val_1 = data.fs.get<Valve>(v1);
    auto& val_2 = data.fs.get<Valve>(v2);

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;
    mid.fluid_package_id = fluid_id;
    
    // Initialize mole fractions: fix inlet composition, others will be solved
    init_stream_composition(in, fluid_id, true);  // Fix inlet composition
    init_stream_composition(mid, fluid_id, false);
    init_stream_composition(out, fluid_id, false);

    data.fs.connect_in<Valve>(v1, s_in);
    data.fs.connect_out<Valve>(v1, s_mid);

    data.fs.connect_in<Valve>(v2, s_mid);
    data.fs.connect_out<Valve>(v2, s_out);

    val_1.Cv.SetValue(1.0e-4, true);
    val_2.Cv.SetValue(3.0e-4, true);

    in.pressure.SetValue(2e5, true);
    out.pressure.SetValue(2e5, true);
    mid.pressure.SetValue(1e5, false);
    
    in.molar_flow.SetValue(1, false);
    out.molar_flow.SetValue(1, false);
    mid.molar_flow.SetValue(1, false);

    in.temperature.SetValue(300.0, true);

    run();

    EXPECT_NEAR(in.molar_flow.value_,  0.0, 1e-12);
    EXPECT_NEAR(mid.molar_flow.value_, 0.0, 1e-12);
    EXPECT_NEAR(out.molar_flow.value_, 0.0, 1e-12);
    EXPECT_NEAR(mid.pressure.value_,   2.0e5, 1e-8);
    
    // Verify mole fractions are preserved (composition unchanged through valves)
    if (in.mole_fractions.size() == 2 && mid.mole_fractions.size() == 2 && out.mole_fractions.size() == 2) {
      EXPECT_NEAR(in.mole_fractions[0].value_, mid.mole_fractions[0].value_, 1e-10);
      EXPECT_NEAR(mid.mole_fractions[0].value_, out.mole_fractions[0].value_, 1e-10);
      // Verify sum constraint
      expect_prob_vector({in.mole_fractions[0].value_, in.mole_fractions[1].value_}, 1e-6);
      expect_prob_vector({mid.mole_fractions[0].value_, mid.mole_fractions[1].value_}, 1e-6);
      expect_prob_vector({out.mole_fractions[0].value_, out.mole_fractions[1].value_}, 1e-6);
    }

    SaveTest("multi_valve_zero_flow_test", data.fs);
  }

  TEST_F(ProcessTest, MultiValveTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs;
    
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

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    mid.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;
    
    // Initialize mole fractions: fix inlet composition, others will be solved
    init_stream_composition(in, fluid_id, true);  // Fix inlet composition
    init_stream_composition(mid, fluid_id, false);
    init_stream_composition(out, fluid_id, false);

    const double Pin  = 1.1e5;
    const double Pout = 1.0e5;
    const double Cv1  = 1.0e-4;
    const double Cv2  = 3.0e-4;

    val1.Cv.SetValue(Cv1, true);
    val2.Cv.SetValue(Cv2, true);
    in.pressure.SetValue(Pin,  true);
    out.pressure.SetValue(Pout, true);

    in.molar_flow.SetValue(0.0, false);
    mid.molar_flow.SetValue(0.0, false);
    out.molar_flow.SetValue(0.0, false);
    mid.pressure.SetValue((Pin + Pout) * 0.5, false);
    
    in.temperature.SetValue(300.0, true);
    mid.temperature.SetValue(300.0, false);
    out.temperature.SetValue(300.0, false);

    // Enthalpy will be automatically initialized from T and P in assemble()
    // No need to manually set it anymore

    run();

    // Analytical solution:
    const double Pmid = (Cv1*Pin + Cv2*Pout) / (Cv1 + Cv2);
    const double F = (Cv1*Cv2/(Cv1+Cv2)) * (Pin - Pout);

    EXPECT_NEAR(mid.pressure.value_,   Pmid, 1e-6);
    EXPECT_NEAR(in.molar_flow.value_,  F,    1e-10);
    EXPECT_NEAR(mid.molar_flow.value_, F,    1e-10);
    EXPECT_NEAR(out.molar_flow.value_, F,    1e-10);

    // Check each valve equation too
    EXPECT_NEAR(mid.molar_flow.value_, val1.Cv.value_ * (in.pressure.value_  - mid.pressure.value_),  1e-10);
    EXPECT_NEAR(out.molar_flow.value_, val2.Cv.value_ * (mid.pressure.value_ - out.pressure.value_),  1e-10);
    
    // Verify mole fractions are preserved (composition unchanged through valves)
    if (in.mole_fractions.size() == 2 && mid.mole_fractions.size() == 2 && out.mole_fractions.size() == 2) {
      EXPECT_NEAR(in.mole_fractions[0].value_, mid.mole_fractions[0].value_, 1e-10);
      EXPECT_NEAR(mid.mole_fractions[0].value_, out.mole_fractions[0].value_, 1e-10);
      // Verify sum constraint
      expect_prob_vector({in.mole_fractions[0].value_, in.mole_fractions[1].value_ }, 1e-6);
      expect_prob_vector({mid.mole_fractions[0].value_, mid.mole_fractions[1].value_}, 1e-6);
      expect_prob_vector({out.mole_fractions[0].value_, out.mole_fractions[1].value_ }, 1e-6);
    }

    SaveTest("multi_valve_test", data.fs);
  }

  TEST_F(ProcessTest, MixerTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs;

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

    // Assign fluid package to all streams
    in1.fluid_package_id = fluid_id;
    in2.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;
    
    // Initialize mole fractions: fix inlet compositions, outlet will be solved
    init_stream_composition(in1, fluid_id, true);  // Fix inlet 1 composition
    init_stream_composition(in2, fluid_id, true);  // Fix inlet 2 composition
    init_stream_composition(out, fluid_id, false);
    
    const double P  = 1.0e5;
    const double F1  = 1.5;
    const double F2  = 0.2;

    in1.pressure.SetValue(P, false);
    in2.pressure.SetValue(P, false);
    out.pressure.SetValue(P, true);
    in1.molar_flow.SetValue(F1, true);
    in2.molar_flow.SetValue(F2, true);

    out.molar_flow.SetValue(0.8, false);
    
    // Fix inlet temperatures and pressures - state equations will calculate H_in1 and H_in2 from T,P
    // Energy balance will determine H_out from inlets
    // State equation will then calculate T_out from H_out, P_out
    in1.temperature.SetValue(300.0, true);
    in2.temperature.SetValue(300.0, true);
    
    run();

    const double F = F1 + F2;
    EXPECT_NEAR(out.molar_flow.value_, F, 1e-10);
    // Verify pressure equality equations
    EXPECT_NEAR(in1.pressure.value_, out.pressure.value_, 1e-10);
    EXPECT_NEAR(in2.pressure.value_, out.pressure.value_, 1e-10);
    
    // Verify component mass balances: Σ(n_in * x_in,i) = n_out * x_out,i
    if (in1.mole_fractions.size() == 2 && in2.mole_fractions.size() == 2 && out.mole_fractions.size() == 2) {
      // Component 0 (N2)
      double comp0_in = in1.molar_flow.value_ * in1.mole_fractions[0].value_ + 
                        in2.molar_flow.value_ * in2.mole_fractions[0].value_;
      double comp0_out = out.molar_flow.value_ * out.mole_fractions[0].value_;
      EXPECT_NEAR(comp0_in, comp0_out, 1e-10);
      
      // Component 1 (O2)
      double comp1_in = in1.molar_flow.value_ * in1.mole_fractions[1].value_ + 
                        in2.molar_flow.value_ * in2.mole_fractions[1].value_;
      double comp1_out = out.molar_flow.value_ * out.mole_fractions[1].value_;
      EXPECT_NEAR(comp1_in, comp1_out, 1e-10);
      
      // Verify sum constraint
      expect_prob_vector({out.mole_fractions[0].value_, out.mole_fractions[1].value_}, 1e-6);
    }

    SaveTest("mixer_test", data.fs);
  }

  TEST_F(ProcessTest, SplitterTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs;

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

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    out1.fluid_package_id = fluid_id;
    out2.fluid_package_id = fluid_id;
    
    // Initialize mole fractions: fix inlet composition, outlets will be solved
    init_stream_composition(in, fluid_id, true);  // Fix inlet composition
    init_stream_composition(out1, fluid_id, false);
    init_stream_composition(out2, fluid_id, false);
    
    const double P  = 1.0e5;
    const double F  = 2.0;
    const double F1 = 1.2;
    const double F2 = 0.0;

    in.pressure.SetValue(P, true);
    out1.pressure.SetValue(P, false);
    out2.pressure.SetValue(P, false);
    in.molar_flow.SetValue(F, true);
    out1.molar_flow.SetValue(F1, true);
    out2.molar_flow.SetValue(F2, false);  // Unknown - should equal F - F1
    
    in.temperature.SetValue(300.0, true);
    
    run();

    EXPECT_NEAR(in.molar_flow.value_, out1.molar_flow.value_ + out2.molar_flow.value_, 1e-10);
    EXPECT_NEAR(in.pressure.value_, out1.pressure.value_, 1e-10);
    EXPECT_NEAR(in.pressure.value_, out2.pressure.value_, 1e-10);
    
    // Verify mole fractions are preserved (composition unchanged through splitter)
    if (in.mole_fractions.size() == 2 && out1.mole_fractions.size() == 2 && out2.mole_fractions.size() == 2) {
      EXPECT_NEAR(in.mole_fractions[0].value_, out1.mole_fractions[0].value_, 1e-10);
      EXPECT_NEAR(in.mole_fractions[0].value_, out2.mole_fractions[0].value_, 1e-10);
      // Verify component mass balances: n_in * x_in,i = Σ(n_out * x_out,i)
      double comp0_in = in.molar_flow.value_ * in.mole_fractions[0].value_;
      double comp0_out = out1.molar_flow.value_ * out1.mole_fractions[0].value_ + 
                         out2.molar_flow.value_ * out2.mole_fractions[0].value_;
      EXPECT_NEAR(comp0_in, comp0_out, 1e-10);
      // Verify sum constraint
      expect_prob_vector({in.mole_fractions[0].value_, in.mole_fractions[1].value_}, 1e-6);
      expect_prob_vector({out1.mole_fractions[0].value_, out1.mole_fractions[1].value_}, 1e-6);
      expect_prob_vector({out2.mole_fractions[0].value_, out2.mole_fractions[1].value_}, 1e-6);
    }

    SaveTest("splitter_test", data.fs);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Fout_and_Cv) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs; 

    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto v = fs.add<Valve>();

    fs.connect_in<Valve>(v, s_in);
    fs.connect_out<Valve>(v, s_out);

    auto& in = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& val = fs.get<Valve>(v);

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;

    // Given
    const double Pin  = 5.0e5;
    const double Pout = 4.0e5;
    const double Fin  = 10.0;

    in.molar_flow.SetValue(Fin,  true);
    in.pressure.SetValue(Pin,  true);
    out.pressure.SetValue(Pout, true);

    // Unknowns
    out.molar_flow.SetValue(0.0,   false);  // F_out
    val.Cv.SetValue(1.0,   false);  // Cv
    
    in.temperature.SetValue(300.0, true);
    out.temperature.SetValue(300.0, false);

    run();

    // Expected (mass balance + valve law)
    const double Fexp  = Fin;
    const double Cvexp = Fexp / (Pin - Pout);

    EXPECT_NEAR(out.molar_flow.value_, Fexp,  1e-10);
    EXPECT_NEAR(in.molar_flow.value_,  Fexp,  1e-10);
    EXPECT_NEAR(val.Cv.value_,         Cvexp, 1e-12);
    EXPECT_NEAR(out.molar_flow.value_, val.Cv.value_ * (in.pressure.value_ - out.pressure.value_), 1e-10);

    SaveTest("single_valve_unknown_fout_and_cv_test", data.fs);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Pout_and_Fout) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs; 

    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto v = fs.add<Valve>();

    fs.connect_in<Valve>(v, s_in);
    fs.connect_out<Valve>(v, s_out);

    auto& in = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& val = fs.get<Valve>(v);

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;

    // Given
    const double Pin = 3.0e5;
    const double Fin = 8.0;
    const double Cv  = 2.0e-4;

    in.molar_flow.SetValue(Fin, true);
    in.pressure.SetValue(Pin, true);
    val.Cv.SetValue(Cv,  true);

    // Unknowns
    out.molar_flow.SetValue(0.0,  false); // F_out
    out.pressure.SetValue(1.0e5, false); // P_out
    
    in.temperature.SetValue(300.0, true);
    out.temperature.SetValue(300.0, false);

    run();

    // Expected (balance F_out = Fin; P_out from valve law)
    const double Fexp  = Fin;
    const double Poutexp = Pin - Fexp / Cv; // = 2.6e5

    EXPECT_NEAR(out.molar_flow.value_, Fexp,    1e-10);
    EXPECT_NEAR(in.molar_flow.value_,  Fexp,    1e-10);
    EXPECT_NEAR(out.pressure.value_,   Poutexp, 1e-6);
    EXPECT_NEAR(out.molar_flow.value_, val.Cv.value_ * (in.pressure.value_ - out.pressure.value_), 1e-10);

    SaveTest("single_valve_unknown_pout_and_fout_test", data.fs);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Fin_and_Fout) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs; 

    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto v = fs.add<Valve>();

    fs.connect_in<Valve>(v, s_in);
    fs.connect_out<Valve>(v, s_out);

    auto& in  = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& val = fs.get<Valve>(v);

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;

    const double Pin = 3.0e5;
    const double Pout = 1.0e5;
    const double Cv = 2.0e-4;

    in.pressure.SetValue(Pin,  true);
    out.pressure.SetValue(Pout, true);
    val.Cv.SetValue(Cv,   true);

    // Unknowns
    in.molar_flow .SetValue(0.0, false); // F_in
    out.molar_flow.SetValue(0.0, false); // F_out
    
    in.temperature.SetValue(300.0, true);

    run();

    // Expected
    const double Fexp = Cv * (Pin - Pout); // = 40
    EXPECT_NEAR(in.molar_flow.value_,  Fexp, 1e-10);
    EXPECT_NEAR(out.molar_flow.value_, Fexp, 1e-10);
    EXPECT_NEAR(out.molar_flow.value_, val.Cv.value_ * (in.pressure.value_ - out.pressure.value_), 1e-10);

    SaveTest("single_valve_unknown_fin_and_fout_test", data.fs);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Pin_and_Cv) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs; 

    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto v = fs.add<Valve>();

    fs.connect_in<Valve>(v, s_in);
    fs.connect_out<Valve>(v, s_out);

    auto& in  = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& val = fs.get<Valve>(v);

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;

    // Given (both flows fixed/consistent; P_out fixed)
    const double Pout = 1.2e5;
    const double F = 12.0;

    in.molar_flow .SetValue(F, true);
    out.molar_flow.SetValue(F, true);
    out.pressure.SetValue(Pout, true);

    // Unknowns (under-determined without an extra condition; we assert consistency)
    in.pressure.SetValue(2.0e5,  false); // P_in (free)
    val.Cv.SetValue(1.0e-5, false); // Cv (free)
    
    // Fix inlet temperature and pressure - state equation will calculate H_in from T_in, P_in
    // Energy balance: H_in = H_out (valve preserves enthalpy)
    // State equation will then calculate T_out from H_out, P_out
    in.temperature.SetValue(300.0, true);

    auto converged = run();
    
    SaveTest("single_valve_unknown_pin_and_cv_test", data.fs);
    
    ASSERT_FALSE(converged); // We expect it to FAIL!
  }

  TEST_F(ProcessTest, RecycleLoopTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs; 

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

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    middle.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;
    recycle.fluid_package_id = fluid_id;
    
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
    in.molar_flow.SetValue(F_in, true);
    recycle.molar_flow.SetValue(F_recycle, true);
    
    // Fix all pressures to same value
    in.pressure.SetValue(P, true);
    middle.pressure.SetValue(P, false);
    out.pressure.SetValue(P, false);
    recycle.pressure.SetValue(P, false);
    
    // Set initial guesses for unknowns
    middle.molar_flow.SetValue(15.0, false);  // Should be F_in + F_recycle = 15
    out.molar_flow.SetValue(10.0, false);    // Should be F_in = 10
    
    in.temperature.SetValue(300.0, true);
    middle.temperature.SetValue(300.0, false);
    out.temperature.SetValue(300.0, false);
    recycle.temperature.SetValue(300.0, false);
    
    std::cout << "\n=== Recycle Loop Test ===" << std::endl;

    bool converged = run();
    
    if (converged) {
      std::cout << "\nResults:" << std::endl;
      std::cout << "F_in = " << in.molar_flow.value_ << std::endl;
      std::cout << "F_middle = " << middle.molar_flow.value_ << std::endl;
      std::cout << "F_out = " << out.molar_flow.value_ << std::endl;
      std::cout << "F_recycle = " << recycle.molar_flow.value_ << std::endl;
      
      // Verify balances
      EXPECT_NEAR(middle.molar_flow.value_, F_in + F_recycle, 1e-10);
      EXPECT_NEAR(out.molar_flow.value_, F_in, 1e-10);
      EXPECT_NEAR(middle.molar_flow.value_, out.molar_flow.value_ + recycle.molar_flow.value_, 1e-10);
    } else {
      std::cout << "\nSOLUTION FAILED: DOF mismatch detected before rank analysis" << std::endl;
      std::cout << "The system correctly identifies that we have more equations than unknowns," << std::endl;
      std::cout << "but the redundant pressure equations (when all pressures are fixed) should" << std::endl;
      std::cout << "be detected by rank analysis, not blocked by the DOF check." << std::endl;
    }
    
    SaveTest("recycle_loop_test", data.fs);
    
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

  TEST_F(ProcessTest, SimpleHeatExchangerTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs; 

    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto hx = fs.add<SimpleHeatExchanger>();

    fs.connect_in<SimpleHeatExchanger>(hx, s_in);
    fs.connect_out<SimpleHeatExchanger>(hx, s_out);

    auto& in = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& hex = fs.get<SimpleHeatExchanger>(hx);

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;

    // Given: inlet conditions
    const double Pin = 2.0e5;  // Pa
    const double Tin = 300.0;  // K
    const double Fin = 5.0;    // mol/s
    const double dP = 1.0e4;   // Pa (pressure drop)
    const double Q = 1.0e5;    // W (heat duty - positive means heat added)

    // Set fixed values
    in.pressure.SetValue(Pin, true);
    in.temperature.SetValue(Tin, true);
    in.molar_flow.SetValue(Fin, true);
    hex.dP.SetValue(dP, true);
    hex.Q.SetValue(Q, true);

    // Unknowns
    out.pressure.SetValue(1.9e5, false);
    out.molar_flow.SetValue(4.9, false);
    out.temperature.SetValue(310.0, false);

    run();

    // Verify mass balance
    EXPECT_NEAR(in.molar_flow.value_, out.molar_flow.value_, 1e-10);
    
    // Verify pressure drop
    EXPECT_NEAR(out.pressure.value_, Pin - dP, 1e-6);
    
    // Verify energy balance: Q = m * (h_out - h_in)
    // We can't directly verify enthalpy without CoolProp, but we can verify the equation
    double h_diff = out.molar_enthalpy.value_ - in.molar_enthalpy.value_;
    EXPECT_NEAR(hex.Q.value_, out.molar_flow.value_ * h_diff, 1e-6);

    SaveTest("simple_heat_exchanger_test", data.fs);
  }

  TEST_F(ProcessTest, HeatExchangerTest) {
    // Initialize fluid package (same for both hot and cold sides in this test)
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs; 

    // Create streams for hot and cold sides
    auto s_hot_in = fs.add<Stream>();
    auto s_hot_out = fs.add<Stream>();
    auto s_cold_in = fs.add<Stream>();
    auto s_cold_out = fs.add<Stream>();
    auto hx = fs.add<HeatExchanger>();

    // Connect streams to heat exchanger
    auto& hex = fs.get<HeatExchanger>(hx);
    hex.in_hot = s_hot_in;
    hex.out_hot = s_hot_out;
    hex.in_cold = s_cold_in;
    hex.out_cold = s_cold_out;

    auto& hot_in = fs.get<Stream>(s_hot_in);
    auto& hot_out = fs.get<Stream>(s_hot_out);
    auto& cold_in = fs.get<Stream>(s_cold_in);
    auto& cold_out = fs.get<Stream>(s_cold_out);

    // Assign fluid package to all streams
    hot_in.fluid_package_id = fluid_id;
    hot_out.fluid_package_id = fluid_id;
    cold_in.fluid_package_id = fluid_id;
    cold_out.fluid_package_id = fluid_id;

    // Given: hot side inlet conditions
    const double P_hot_in = 3.0e5;   // Pa
    const double T_hot_in = 400.0;   // K
    const double F_hot = 10.0;       // mol/s
    const double dP_hot = 50.0;     // Pa (pressure drop on hot side)

    // Given: cold side inlet conditions
    const double P_cold_in = 2.0e5;  // Pa
    const double T_cold_in = 300.0;  // K
    const double F_cold = 8.0;       // mol/s
    const double dP_cold = 30.0;    // Pa (pressure drop on cold side)

    // Heat duty (positive means heat flows from hot to cold)
    const double Q = 0.0;          // W

    // Set fixed values
    hot_in.pressure.SetValue(P_hot_in, true);
    hot_in.temperature.SetValue(T_hot_in, true);
    hot_in.molar_flow.SetValue(F_hot, true);
    
    cold_in.pressure.SetValue(P_cold_in, true);
    cold_in.temperature.SetValue(T_cold_in, true);
    cold_in.molar_flow.SetValue(F_cold, true);
    
    hex.dP_hot.SetValue(dP_hot, true);
    hex.dP_cold.SetValue(dP_cold, true);
    hex.Q.SetValue(Q, true);

    // Unknowns
    hot_out.pressure.SetValue(2.95e5, false);
    hot_out.molar_flow.SetValue(9.9, false);
    hot_out.temperature.SetValue(390.0, false);
    
    cold_out.pressure.SetValue(1.97e5, false);
    cold_out.molar_flow.SetValue(7.9, false);
    cold_out.temperature.SetValue(310.0, false);

    run();

    // Verify mass balances
    EXPECT_NEAR(hot_in.molar_flow.value_, hot_out.molar_flow.value_, 1e-10);
    EXPECT_NEAR(cold_in.molar_flow.value_, cold_out.molar_flow.value_, 1e-10);
    
    // Verify pressure drops
    EXPECT_NEAR(hot_out.pressure.value_, P_hot_in - dP_hot, 1e-6);
    EXPECT_NEAR(cold_out.pressure.value_, P_cold_in - dP_cold, 1e-6);
    
    // Verify energy balances
    // Hot side: Q = m_hot * (h_hot_in - h_hot_out)
    double h_hot_diff = hot_in.molar_enthalpy.value_ - hot_out.molar_enthalpy.value_;
    EXPECT_NEAR(hex.Q.value_, hot_in.molar_flow.value_ * h_hot_diff, 1e-6);
    
    // Cold side: Q = m_cold * (h_cold_out - h_cold_in)
    double h_cold_diff = cold_out.molar_enthalpy.value_ - cold_in.molar_enthalpy.value_;
    EXPECT_NEAR(hex.Q.value_, cold_out.molar_flow.value_ * h_cold_diff, 1e-6);

    SaveTest("heat_exchanger_test", data.fs);
  }

  TEST_F(ProcessTest, PumpTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs; 
    // Create streams
    auto s_in = fs.add<Stream>();
    auto s_out = fs.add<Stream>();
    auto p = fs.add<Pump>();

    // Connect streams to pump
    fs.connect_in<Pump>(p, s_in);
    fs.connect_out<Pump>(p, s_out);

    auto& in = fs.get<Stream>(s_in);
    auto& out = fs.get<Stream>(s_out);
    auto& pump = fs.get<Pump>(p);

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;
    
    // Initialize mole fractions: fix inlet composition, outlet will be solved
    init_stream_composition(in, fluid_id, true);  // Fix inlet composition
    init_stream_composition(out, fluid_id, false);

    // Given: inlet conditions
    const double Pin = 1.0e5;   // Pa
    const double Tin = 300.0;   // K
    const double Fin = 10.0;    // mol/s
    const double dP = 1.0e5;    // Pa (pressure rise)
    const double eta = 0.8;     // Efficiency

    // Set fixed values
    in.pressure.SetValue(Pin, true);
    in.temperature.SetValue(Tin, true);
    in.molar_flow.SetValue(Fin, true);
    pump.dP.SetValue(dP, true);
    pump.eta.SetValue(eta, true);

    // Unknowns
    out.pressure.SetValue(1.9e5, false);  // Should be Pin + dP = 2.0e5
    out.molar_flow.SetValue(9.9, false);  // Should equal Fin
    out.temperature.SetValue(300.0, false);  // Will be calculated from state equation
    pump.W.SetValue(1000.0, false);  // Work will be calculated from isentropic work equation (based on dP)

    run();

    // Verify mass balance
    EXPECT_NEAR(in.molar_flow.value_, out.molar_flow.value_, 1e-10);
    
    // Verify pressure rise: P_out = P_in + dP
    EXPECT_NEAR(out.pressure.value_, Pin + dP, 1e-6);
    
    // Verify energy balance: W * eta = m * (h_out - h_in)
    double delta_h = out.molar_enthalpy.value_ - in.molar_enthalpy.value_;
    // Use inlet flow for consistency with equation (mass balance ensures they're equal)
    EXPECT_NEAR(pump.W.value_ * pump.eta.value_, in.molar_flow.value_ * delta_h, 1e-6);

    SaveTest("pump_test", data.fs);
  }

  TEST_F(ProcessTest, ComponentSplitterTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs; 

    // Create streams
    auto s_in = fs.add<Stream>();
    auto s_overhead = fs.add<Stream>();
    auto s_bottom = fs.add<Stream>();
    auto cs = fs.add<ComponentSplitter>();

    // Connect streams to component splitter
    auto& splitter = fs.get<ComponentSplitter>(cs);
    splitter.in = s_in;
    splitter.overhead = s_overhead;
    splitter.bottom = s_bottom;

    auto& in = fs.get<Stream>(s_in);
    auto& overhead = fs.get<Stream>(s_overhead);
    auto& bottom = fs.get<Stream>(s_bottom);

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    overhead.fluid_package_id = fluid_id;
    bottom.fluid_package_id = fluid_id;
    
    // Initialize mole fractions: fix inlet composition, outlets will be solved
    init_stream_composition(in, fluid_id, true);  // Fix inlet composition
    init_stream_composition(overhead, fluid_id, false);
    init_stream_composition(bottom, fluid_id, false);

    // Set up split ratios (one per component)
    // For N2/O2 mixture: 2 components
    auto components = fs.fluids.GetComponents(fluid_id);
    splitter.overhead_split_ratios.resize(components.size());
    
    // Component 0 (N2): 70% goes to overhead, 30% to bottom
    splitter.overhead_split_ratios[0] = Var("split_ratio_N2", 1.0, true);
    // Component 1 (O2): 30% goes to overhead, 70% to bottom
    splitter.overhead_split_ratios[1] = Var("split_ratio_O2", 0.0, true);

    // Given: inlet conditions
    const double Pin = 1.0e5;   // Pa
    const double Tin = 300.0;   // K
    const double Fin = 10.0;    // mol/s
    const double Q = 0.0;       // W (heat duty - zero for adiabatic)

    // Set fixed values
    in.pressure.SetValue(Pin, true);
    in.temperature.SetValue(Tin, true);
    in.molar_flow.SetValue(Fin, true);
    splitter.Q.SetValue(Q, false);

    // Unknowns
    overhead.pressure.SetValue(0.95e5, false);  // Should equal bottom pressure
    bottom.pressure.SetValue(0.95e5, false);    // Should equal Pin
    overhead.temperature.SetValue(300.0, true);
    bottom.temperature.SetValue(300.0, true);
    
    overhead.molar_flow.SetValue(0.5 * Fin, false);  // Fix overhead flow
    bottom.molar_flow.SetValue(0.5 * Fin, false);  // Will be determined by mass balance
    
    run();

    // Verify mass balance: m_in = m_overhead + m_bottom
    EXPECT_NEAR(in.molar_flow.value_, overhead.molar_flow.value_ + bottom.molar_flow.value_, 1e-10);
    
    // Verify pressure relationships
    EXPECT_NEAR(bottom.pressure.value_, Pin, 1e-6);  // P_bottom = P_in
    EXPECT_NEAR(overhead.pressure.value_, bottom.pressure.value_, 1e-6);  // P_overhead = P_bottom
    
    // Verify component balances for each component
    if (in.mole_fractions.size() == 2 && overhead.mole_fractions.size() == 2 && bottom.mole_fractions.size() == 2) {
      // Component 0 (N2): n_in * x_in,0 * split_ratio_0 = n_overhead * x_overhead,0
      double comp0_in_overhead = in.molar_flow.value_ * in.mole_fractions[0].value_ * splitter.overhead_split_ratios[0].value_;
      double comp0_overhead = overhead.molar_flow.value_ * overhead.mole_fractions[0].value_;
      EXPECT_NEAR(comp0_in_overhead, comp0_overhead, 1e-10);
      
      // Component 0 (N2): n_in * x_in,0 * (1 - split_ratio_0) = n_bottom * x_bottom,0
      double comp0_in_bottom = in.molar_flow.value_ * in.mole_fractions[0].value_ * (1.0 - splitter.overhead_split_ratios[0].value_);
      double comp0_bottom = bottom.molar_flow.value_ * bottom.mole_fractions[0].value_;
      EXPECT_NEAR(comp0_in_bottom, comp0_bottom, 1e-10);
      
      // Component 1 (O2): n_in * x_in,1 * split_ratio_1 = n_overhead * x_overhead,1
      double comp1_in_overhead = in.molar_flow.value_ * in.mole_fractions[1].value_ * splitter.overhead_split_ratios[1].value_;
      double comp1_overhead = overhead.molar_flow.value_ * overhead.mole_fractions[1].value_;
      EXPECT_NEAR(comp1_in_overhead, comp1_overhead, 1e-10);
      
      // Component 1 (O2): n_in * x_in,1 * (1 - split_ratio_1) = n_bottom * x_bottom,1
      double comp1_in_bottom = in.molar_flow.value_ * in.mole_fractions[1].value_ * (1.0 - splitter.overhead_split_ratios[1].value_);
      double comp1_bottom = bottom.molar_flow.value_ * bottom.mole_fractions[1].value_;
      EXPECT_NEAR(comp1_in_bottom, comp1_bottom, 1e-10);
      
      // Verify general component balance: n_in * x_in,i = n_overhead * x_overhead,i + n_bottom * x_bottom,i
      for (size_t i = 0; i < 2; ++i) {
        double comp_in = in.molar_flow.value_ * in.mole_fractions[i].value_;
        double comp_out = overhead.molar_flow.value_ * overhead.mole_fractions[i].value_ + 
                         bottom.molar_flow.value_ * bottom.mole_fractions[i].value_;
        EXPECT_NEAR(comp_in, comp_out, 1e-10);
      }
      
      // Verify sum constraints
      expect_prob_vector({in.mole_fractions[0].value_, in.mole_fractions[1].value_}, 1e-6);
      expect_prob_vector({overhead.mole_fractions[0].value_, overhead.mole_fractions[1].value_}, 1e-6);
      expect_prob_vector({bottom.mole_fractions[0].value_, bottom.mole_fractions[1].value_}, 1e-6);
      
      // Verify that compositions are different (splitter changes composition)
      // Overhead should have more N2 (higher split ratio), bottom should have more O2
      EXPECT_GT(overhead.mole_fractions[0].value_, in.mole_fractions[0].value_);  // More N2 in overhead
      EXPECT_LT(bottom.mole_fractions[0].value_, in.mole_fractions[0].value_);   // Less N2 in bottom
    }
    
    // Verify energy balance: Q = m_overhead * h_overhead + m_bottom * h_bottom - m_in * h_in
    double energy_out = overhead.molar_flow.value_ * overhead.molar_enthalpy.value_ + 
                       bottom.molar_flow.value_ * bottom.molar_enthalpy.value_;
    double energy_in = in.molar_flow.value_ * in.molar_enthalpy.value_;
    EXPECT_NEAR(splitter.Q.value_, energy_out - energy_in, 1e-6);

    SaveTest("component_splitter_test", data.fs);
  }

  TEST_F(ProcessTest, AmbientAirCompressionAndCoolingTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    auto& fs = data.fs; 

    // Create streams: ambient inlet, after compression, final cooled outlet
    auto s_ambient = fs.add<Stream>();
    auto s_compressed = fs.add<Stream>();
    auto s_cooled = fs.add<Stream>();
    
    // Create unit operations: pump and heat exchanger
    auto pump = fs.add<Pump>();
    auto hx = fs.add<SimpleHeatExchanger>();

    // Connect pump: ambient -> compressed
    fs.connect_in<Pump>(pump, s_ambient);
    fs.connect_out<Pump>(pump, s_compressed);
    
    // Connect heat exchanger: compressed -> cooled
    fs.connect_in<SimpleHeatExchanger>(hx, s_compressed);
    fs.connect_out<SimpleHeatExchanger>(hx, s_cooled);

    auto& ambient = fs.get<Stream>(s_ambient);
    auto& compressed = fs.get<Stream>(s_compressed);
    auto& cooled = fs.get<Stream>(s_cooled);
    auto& p = fs.get<Pump>(pump);
    auto& hex = fs.get<SimpleHeatExchanger>(hx);

    // Assign fluid package to all streams
    ambient.fluid_package_id = fluid_id;
    compressed.fluid_package_id = fluid_id;
    cooled.fluid_package_id = fluid_id;
    
    // Initialize mole fractions: fix ambient composition, others will be solved
    init_stream_composition(ambient, fluid_id, true);  // Fix ambient composition
    init_stream_composition(compressed, fluid_id, false);
    init_stream_composition(cooled, fluid_id, false);

    // Given: ambient conditions
    const double P_ambient = 1.0e5;   // Pa (1 bar)
    const double T_ambient = 300.0;  // K (room temperature)
    const double F = 10.0;            // mol/s
    
    // Compression: from 1 bar to 5 bar
    const double P_compressed = 5.0e5;  // Pa (5 bar)
    const double dP_pump = P_compressed - P_ambient;  // 4.0e5 Pa
    const double eta = 0.75;            // Pump efficiency
    
    // Heat exchanger: cool back to ambient temperature
    const double T_cooled = 300.0;      // K (back to ambient)
    const double dP_hx = 1.0e4;         // Pa (pressure drop in heat exchanger)

    // Set fixed values for ambient inlet
    ambient.pressure.SetValue(P_ambient, true);
    ambient.temperature.SetValue(T_ambient, true);
    ambient.molar_flow.SetValue(F, true);
    
    // Set pump parameters
    p.dP.SetValue(dP_pump, true);
    p.eta.SetValue(eta, true);
    
    // Set heat exchanger parameters
    hex.dP.SetValue(dP_hx, true);
    cooled.temperature.SetValue(T_cooled, true);  // Fix outlet temperature, solve for Q
    
    // Unknowns
    compressed.pressure.SetValue(4.9e5, false);    // Should be P_ambient + dP_pump
    compressed.molar_flow.SetValue(9.9, false);   // Should equal F
    compressed.temperature.SetValue(350.0, false); // Will be calculated from compression
    
    cooled.pressure.SetValue(4.8e5, false);       // Should be P_compressed - dP_hx
    cooled.molar_flow.SetValue(9.8, false);      // Should equal F
    hex.Q.SetValue(0.0, false);                   // Will be calculated (negative for cooling)
    p.W.SetValue(1000.0, false);                 // Work will be calculated

    run();

    // Verify mass balances
    EXPECT_NEAR(ambient.molar_flow.value_, compressed.molar_flow.value_, 1e-10);
    EXPECT_NEAR(compressed.molar_flow.value_, cooled.molar_flow.value_, 1e-10);
    EXPECT_NEAR(ambient.molar_flow.value_, cooled.molar_flow.value_, 1e-10);
    
    // Verify compression: P_compressed = P_ambient + dP_pump
    EXPECT_NEAR(compressed.pressure.value_, P_compressed, 1e-6);
    
    // Verify heat exchanger pressure drop: P_cooled = P_compressed - dP_hx
    EXPECT_NEAR(cooled.pressure.value_, P_compressed - dP_hx, 1e-6);
    
    // Verify final temperature is back to ambient
    EXPECT_NEAR(cooled.temperature.value_, T_ambient, 1e-6);
    
    // Verify compression increases temperature (adiabatic compression heats the gas)
    EXPECT_GT(compressed.temperature.value_, T_ambient);
    
    // Verify pump energy balance: W * eta = m * (h_compressed - h_ambient)
    double delta_h_pump = compressed.molar_enthalpy.value_ - ambient.molar_enthalpy.value_;
    EXPECT_NEAR(p.W.value_ * p.eta.value_, ambient.molar_flow.value_ * delta_h_pump, 1e-6);
    
    // Verify heat exchanger energy balance: Q = m * (h_cooled - h_compressed)
    // Q should be negative (heat removed) since we're cooling
    double delta_h_hx = cooled.molar_enthalpy.value_ - compressed.molar_enthalpy.value_;
    EXPECT_NEAR(hex.Q.value_, compressed.molar_flow.value_ * delta_h_hx, 1e-6);
    EXPECT_LT(hex.Q.value_, 0.0);  // Negative Q means cooling
    
    SaveTest("ambient_air_compression_and_cooling_test", data.fs);
  }

} // end processX namespace
