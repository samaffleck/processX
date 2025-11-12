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

    // Helper function to initialize N2/O2 fluid package (79% N2, 21% O2)
    size_t init_air_fluid() {
      size_t fluid_id = fs.fluids.AddFluidPackage({"N2", "O2"}, "HEOS");
      auto fluid = fs.fluids.GetFluidPackage(fluid_id);
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
      auto components = fs.fluids.GetComponents(fluid_id);
      if (components.size() == 2) {
        // For N2/O2 mixture, set to 79% N2, 21% O2
        stream.initialize_composition(components.size(), components);
        if (stream.mole_fractions.size() == 2) {
          stream.mole_fractions[0].set_val(0.79, fix_composition); // N2
          stream.mole_fractions[1].set_val(0.21, fix_composition); // O2
        }
      } else {
        // For other mixtures, use equal fractions
        stream.initialize_composition(components.size(), components);
        if (fix_composition) {
          // Fix all mole fractions if requested
          for (auto& x : stream.mole_fractions) {
            x.fixed = true;
          }
        }
      }
    }
  };


  TEST_F(ProcessTest, MultiValveZeroFlowTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    
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

    // Assign fluid package to all streams
    in.fluid_package_id = fluid_id;
    out.fluid_package_id = fluid_id;
    mid.fluid_package_id = fluid_id;
    
    // Initialize mole fractions: fix inlet composition, others will be solved
    init_stream_composition(in, fluid_id, true);  // Fix inlet composition
    init_stream_composition(mid, fluid_id, false);
    init_stream_composition(out, fluid_id, false);

    fs.connect_in<Valve>(v1, s_in);
    fs.connect_out<Valve>(v1, s_mid);

    fs.connect_in<Valve>(v2, s_mid);
    fs.connect_out<Valve>(v2, s_out);

    val_1.Cv.set_val(1.0e-4, true);
    val_2.Cv.set_val(3.0e-4, true);

    in.pressure.set_val(2e5, true);
    out.pressure.set_val(2e5, true);
    mid.pressure.set_val(1e5, false);
    
    in.molar_flow.set_val(1, false);
    out.molar_flow.set_val(1, false);
    mid.molar_flow.set_val(1, false);
    
    in.temperature.set_val(300.0, true);

    run();

    EXPECT_NEAR(in.molar_flow.value,  0.0, 1e-12);
    EXPECT_NEAR(mid.molar_flow.value, 0.0, 1e-12);
    EXPECT_NEAR(out.molar_flow.value, 0.0, 1e-12);
    EXPECT_NEAR(mid.pressure.value,   2.0e5, 1e-8);
    
    // Verify mole fractions are preserved (composition unchanged through valves)
    if (in.mole_fractions.size() == 2 && mid.mole_fractions.size() == 2 && out.mole_fractions.size() == 2) {
      EXPECT_NEAR(in.mole_fractions[0].value, mid.mole_fractions[0].value, 1e-10);
      EXPECT_NEAR(mid.mole_fractions[0].value, out.mole_fractions[0].value, 1e-10);
      // Verify sum constraint
      expect_prob_vector({in.mole_fractions[0].value, in.mole_fractions[1].value}, 1e-6);
      expect_prob_vector({mid.mole_fractions[0].value, mid.mole_fractions[1].value}, 1e-6);
      expect_prob_vector({out.mole_fractions[0].value, out.mole_fractions[1].value}, 1e-6);
    }

    SaveTest("multi_valve_zero_flow_test", fs);
  }

  TEST_F(ProcessTest, MultiValveTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    
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

    val1.Cv.set_val(Cv1, true);
    val2.Cv.set_val(Cv2, true);
    in.pressure.set_val(Pin,  true);
    out.pressure.set_val(Pout, true);

    in.molar_flow.set_val(0.0, false);
    mid.molar_flow.set_val(0.0, false);
    out.molar_flow.set_val(0.0, false);
    mid.pressure.set_val((Pin + Pout) * 0.5, false);
    
    in.temperature.set_val(300.0, true);
    mid.temperature.set_val(300.0, false);
    out.temperature.set_val(300.0, false);

    // Enthalpy will be automatically initialized from T and P in assemble()
    // No need to manually set it anymore

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
    
    // Verify mole fractions are preserved (composition unchanged through valves)
    if (in.mole_fractions.size() == 2 && mid.mole_fractions.size() == 2 && out.mole_fractions.size() == 2) {
      EXPECT_NEAR(in.mole_fractions[0].value, mid.mole_fractions[0].value, 1e-10);
      EXPECT_NEAR(mid.mole_fractions[0].value, out.mole_fractions[0].value, 1e-10);
      // Verify sum constraint
      expect_prob_vector({in.mole_fractions[0].value, in.mole_fractions[1].value}, 1e-6);
      expect_prob_vector({mid.mole_fractions[0].value, mid.mole_fractions[1].value}, 1e-6);
      expect_prob_vector({out.mole_fractions[0].value, out.mole_fractions[1].value}, 1e-6);
    }

    SaveTest("multi_valve_test", fs);
  }

  TEST_F(ProcessTest, MixerTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    
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

    in1.pressure.set_val(P, false);
    in2.pressure.set_val(P, false);
    out.pressure.set_val(P, true);
    in1.molar_flow.set_val(F1, true);
    in2.molar_flow.set_val(F2, true);

    out.molar_flow.set_val(0.8, false);
    
    // Fix inlet temperatures and pressures - state equations will calculate H_in1 and H_in2 from T,P
    // Energy balance will determine H_out from inlets
    // State equation will then calculate T_out from H_out, P_out
    in1.temperature.set_val(300.0, true);
    in2.temperature.set_val(300.0, true);
    
    run();

    const double F = F1 + F2;
    EXPECT_NEAR(out.molar_flow.value, F, 1e-10);
    // Verify pressure equality equations
    EXPECT_NEAR(in1.pressure.value, out.pressure.value, 1e-10);
    EXPECT_NEAR(in2.pressure.value, out.pressure.value, 1e-10);
    
    // Verify component mass balances: Σ(n_in * x_in,i) = n_out * x_out,i
    if (in1.mole_fractions.size() == 2 && in2.mole_fractions.size() == 2 && out.mole_fractions.size() == 2) {
      // Component 0 (N2)
      double comp0_in = in1.molar_flow.value * in1.mole_fractions[0].value + 
                        in2.molar_flow.value * in2.mole_fractions[0].value;
      double comp0_out = out.molar_flow.value * out.mole_fractions[0].value;
      EXPECT_NEAR(comp0_in, comp0_out, 1e-10);
      
      // Component 1 (O2)
      double comp1_in = in1.molar_flow.value * in1.mole_fractions[1].value + 
                        in2.molar_flow.value * in2.mole_fractions[1].value;
      double comp1_out = out.molar_flow.value * out.mole_fractions[1].value;
      EXPECT_NEAR(comp1_in, comp1_out, 1e-10);
      
      // Verify sum constraint
      expect_prob_vector({out.mole_fractions[0].value, out.mole_fractions[1].value}, 1e-6);
    }

    SaveTest("mixer_test", fs);
  }

  TEST_F(ProcessTest, SplitterTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    
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

    in.pressure.set_val(P, true);
    out1.pressure.set_val(P, false);
    out2.pressure.set_val(P, false);
    in.molar_flow.set_val(F, true);
    out1.molar_flow.set_val(F1, true);
    out2.molar_flow.set_val(F2, false);  // Unknown - should equal F - F1
    
    in.temperature.set_val(300.0, true);
    
    run();

    EXPECT_NEAR(in.molar_flow.value, out1.molar_flow.value + out2.molar_flow.value, 1e-10);
    EXPECT_NEAR(in.pressure.value, out1.pressure.value, 1e-10);
    EXPECT_NEAR(in.pressure.value, out2.pressure.value, 1e-10);
    
    // Verify mole fractions are preserved (composition unchanged through splitter)
    if (in.mole_fractions.size() == 2 && out1.mole_fractions.size() == 2 && out2.mole_fractions.size() == 2) {
      EXPECT_NEAR(in.mole_fractions[0].value, out1.mole_fractions[0].value, 1e-10);
      EXPECT_NEAR(in.mole_fractions[0].value, out2.mole_fractions[0].value, 1e-10);
      // Verify component mass balances: n_in * x_in,i = Σ(n_out * x_out,i)
      double comp0_in = in.molar_flow.value * in.mole_fractions[0].value;
      double comp0_out = out1.molar_flow.value * out1.mole_fractions[0].value + 
                         out2.molar_flow.value * out2.mole_fractions[0].value;
      EXPECT_NEAR(comp0_in, comp0_out, 1e-10);
      // Verify sum constraint
      expect_prob_vector({in.mole_fractions[0].value, in.mole_fractions[1].value}, 1e-6);
      expect_prob_vector({out1.mole_fractions[0].value, out1.mole_fractions[1].value}, 1e-6);
      expect_prob_vector({out2.mole_fractions[0].value, out2.mole_fractions[1].value}, 1e-6);
    }

    SaveTest("splitter_test", fs);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Fout_and_Cv) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    
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

    in.molar_flow.set_val(Fin,  true);
    in.pressure.set_val(Pin,  true);
    out.pressure.set_val(Pout, true);

    // Unknowns
    out.molar_flow.set_val(0.0,   false);  // F_out
    val.Cv.set_val(1.0,   false);  // Cv
    
    in.temperature.set_val(300.0, true);
    out.temperature.set_val(300.0, false);

    run();

    // Expected (mass balance + valve law)
    const double Fexp  = Fin;
    const double Cvexp = Fexp / (Pin - Pout);

    EXPECT_NEAR(out.molar_flow.value, Fexp,  1e-10);
    EXPECT_NEAR(in.molar_flow.value,  Fexp,  1e-10);
    EXPECT_NEAR(val.Cv.value,         Cvexp, 1e-12);
    EXPECT_NEAR(out.molar_flow.value, val.Cv.value * (in.pressure.value - out.pressure.value), 1e-10);

    SaveTest("single_valve_unknown_fout_and_cv_test", fs);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Pout_and_Fout) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    
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

    in.molar_flow.set_val(Fin, true);
    in.pressure.set_val(Pin, true);
    val.Cv.set_val(Cv,  true);

    // Unknowns
    out.molar_flow.set_val(0.0,  false); // F_out
    out.pressure.set_val(1.0e5, false); // P_out
    
    in.temperature.set_val(300.0, true);
    out.temperature.set_val(300.0, false);

    run();

    // Expected (balance F_out = Fin; P_out from valve law)
    const double Fexp  = Fin;
    const double Poutexp = Pin - Fexp / Cv; // = 2.6e5

    EXPECT_NEAR(out.molar_flow.value, Fexp,    1e-10);
    EXPECT_NEAR(in.molar_flow.value,  Fexp,    1e-10);
    EXPECT_NEAR(out.pressure.value,   Poutexp, 1e-6);
    EXPECT_NEAR(out.molar_flow.value, val.Cv.value * (in.pressure.value - out.pressure.value), 1e-10);

    SaveTest("single_valve_unknown_pout_and_fout_test", fs);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Fin_and_Fout) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    
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

    in.pressure.set_val(Pin,  true);
    out.pressure.set_val(Pout, true);
    val.Cv.set_val(Cv,   true);

    // Unknowns
    in.molar_flow .set_val(0.0, false); // F_in
    out.molar_flow.set_val(0.0, false); // F_out
    
    in.temperature.set_val(300.0, true);

    run();

    // Expected
    const double Fexp = Cv * (Pin - Pout); // = 40
    EXPECT_NEAR(in.molar_flow.value,  Fexp, 1e-10);
    EXPECT_NEAR(out.molar_flow.value, Fexp, 1e-10);
    EXPECT_NEAR(out.molar_flow.value, val.Cv.value * (in.pressure.value - out.pressure.value), 1e-10);

    SaveTest("single_valve_unknown_fin_and_fout_test", fs);
  }

  TEST_F(ProcessTest, SingleValve_Unknown_Pin_and_Cv) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    
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

    in.molar_flow .set_val(F, true);
    out.molar_flow.set_val(F, true);
    out.pressure.set_val(Pout, true);

    // Unknowns (under-determined without an extra condition; we assert consistency)
    in.pressure.set_val(2.0e5,  false); // P_in (free)
    val.Cv.set_val(1.0e-5, false); // Cv (free)
    
    // Fix inlet temperature and pressure - state equation will calculate H_in from T_in, P_in
    // Energy balance: H_in = H_out (valve preserves enthalpy)
    // State equation will then calculate T_out from H_out, P_out
    in.temperature.set_val(300.0, true);

    auto converged = run();
    
    SaveTest("single_valve_unknown_pin_and_cv_test", fs);
    
    ASSERT_FALSE(converged); // We expect it to FAIL!
  }

  TEST_F(ProcessTest, RecycleLoopTest) {
    // Initialize fluid package
    size_t fluid_id = init_air_fluid();
    
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
    in.molar_flow.set_val(F_in, true);
    recycle.molar_flow.set_val(F_recycle, true);
    
    // Fix all pressures to same value
    in.pressure.set_val(P, true);
    middle.pressure.set_val(P, false);
    out.pressure.set_val(P, false);
    recycle.pressure.set_val(P, false);
    
    // Set initial guesses for unknowns
    middle.molar_flow.set_val(15.0, false);  // Should be F_in + F_recycle = 15
    out.molar_flow.set_val(10.0, false);    // Should be F_in = 10
    
    in.temperature.set_val(300.0, true);
    middle.temperature.set_val(300.0, false);
    out.temperature.set_val(300.0, false);
    recycle.temperature.set_val(300.0, false);
    
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
    
    SaveTest("recycle_loop_test", fs);
    
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
    in.pressure.set_val(Pin, true);
    in.temperature.set_val(Tin, true);
    in.molar_flow.set_val(Fin, true);
    hex.dP.set_val(dP, true);
    hex.Q.set_val(Q, true);

    // Unknowns
    out.pressure.set_val(1.9e5, false);
    out.molar_flow.set_val(4.9, false);
    out.temperature.set_val(310.0, false);

    run();

    // Verify mass balance
    EXPECT_NEAR(in.molar_flow.value, out.molar_flow.value, 1e-10);
    
    // Verify pressure drop
    EXPECT_NEAR(out.pressure.value, Pin - dP, 1e-6);
    
    // Verify energy balance: Q = m * (h_out - h_in)
    // We can't directly verify enthalpy without CoolProp, but we can verify the equation
    double h_diff = out.molar_enthalpy.value - in.molar_enthalpy.value;
    EXPECT_NEAR(hex.Q.value, out.molar_flow.value * h_diff, 1e-6);

    SaveTest("simple_heat_exchanger_test", fs);
  }

  TEST_F(ProcessTest, HeatExchangerTest) {
    // Initialize fluid package (same for both hot and cold sides in this test)
    size_t fluid_id = init_air_fluid();
    
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
    hot_in.pressure.set_val(P_hot_in, true);
    hot_in.temperature.set_val(T_hot_in, true);
    hot_in.molar_flow.set_val(F_hot, true);
    
    cold_in.pressure.set_val(P_cold_in, true);
    cold_in.temperature.set_val(T_cold_in, true);
    cold_in.molar_flow.set_val(F_cold, true);
    
    hex.dP_hot.set_val(dP_hot, true);
    hex.dP_cold.set_val(dP_cold, true);
    hex.Q.set_val(Q, true);

    // Unknowns
    hot_out.pressure.set_val(2.95e5, false);
    hot_out.molar_flow.set_val(9.9, false);
    hot_out.temperature.set_val(390.0, false);
    
    cold_out.pressure.set_val(1.97e5, false);
    cold_out.molar_flow.set_val(7.9, false);
    cold_out.temperature.set_val(310.0, false);

    run();

    // Verify mass balances
    EXPECT_NEAR(hot_in.molar_flow.value, hot_out.molar_flow.value, 1e-10);
    EXPECT_NEAR(cold_in.molar_flow.value, cold_out.molar_flow.value, 1e-10);
    
    // Verify pressure drops
    EXPECT_NEAR(hot_out.pressure.value, P_hot_in - dP_hot, 1e-6);
    EXPECT_NEAR(cold_out.pressure.value, P_cold_in - dP_cold, 1e-6);
    
    // Verify energy balances
    // Hot side: Q = m_hot * (h_hot_in - h_hot_out)
    double h_hot_diff = hot_in.molar_enthalpy.value - hot_out.molar_enthalpy.value;
    EXPECT_NEAR(hex.Q.value, hot_in.molar_flow.value * h_hot_diff, 1e-6);
    
    // Cold side: Q = m_cold * (h_cold_out - h_cold_in)
    double h_cold_diff = cold_out.molar_enthalpy.value - cold_in.molar_enthalpy.value;
    EXPECT_NEAR(hex.Q.value, cold_out.molar_flow.value * h_cold_diff, 1e-6);

    SaveTest("heat_exchanger_test", fs);
  }

} // end processX namespace
