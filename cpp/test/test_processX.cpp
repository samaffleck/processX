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

  TEST_F(ProcessTest, LoadTest) {
    const std::string file_path = "gpt_test.json";
    LoadFromJson(this->fs, file_path);
    run();
  }

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

  TEST_F(ProcessTest, MixerValveTest) {
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

    in1.pressure.set_val(P, true);
    in2.pressure.set_val(P, true);
    out.pressure.set_val(P, true);
    in1.molar_flow.set_val(F1, true);
    in2.molar_flow.set_val(F2, true);

    out.molar_flow.set_val(0.8, false);
    
    run();

    const double F = F1 + F2;
    EXPECT_NEAR(out.molar_flow.value, F, 1e-10);
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

    auto converged = run();
    ASSERT_FALSE(converged); // We expect it to FAIL!
  }

} // end processX namespace
