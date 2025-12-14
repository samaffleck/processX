#include "processX/fluids.h"

namespace px {

  // Common CoolProp backends
  const char* backend_names[] = {
    "HEOS"
  };

  const int kNumberOfBackends = sizeof(backend_names) / sizeof(backend_names[0]);

  // Common CoolProp component names
  // Based on CoolProp's supported fluids: https://coolprop.org/fluid_properties/PurePseudoPure.html
  const char* component_names[] = {
    "1-Butene", 
    "Acetone",
    "Air",
    "Ammonia",
    "Argon",
    "Benzene",
    "CarbonDioxide",
    "CarbonMonoxide",
    "CarbonylSulfide",
    "CycloHexane",
    "CycloPropane",
    "Cyclopentane",
    "D4",
    "D5",
    "D6",
    "Deuterium",  
    "Dichloroethane",
    "DiethylEther",
    "DimethylCarbonate",
    "DimethylEther",
    "Ethane",
    "Ethanol",
    "EthylBenzene",
    "Ethylene",
    "EthyleneOxide",
    "Fluorine",
    "HFE143m",
    "HeavyWater",
    "Helium",
    "Hydrogen",
    "HydrogenChloride",
    "HydrogenSulfide",
    "IsoButane",
    "IsoButene",
    "Isohexane",
    "Isopentane",
    "Krypton",
    "MD2M",
    "MD3M",
    "MD4M",
    "MDM",
    "MM",
    "Methane",
    "Methanol",
    "MethylLinoleate",
    "MethylLinolenate",
    "MethylOleate",
    "MethylPalmitate",
    "MethylStearate",
    "Neon",
    "Neopentane",
    "Nitrogen",
    "NitrousOxide",
    "Novec649",
    "OrthoDeuterium",
    "OrthoHydrogen",
    "Oxygen",
    "ParaDeuterium",
    "ParaHydrogen",
    "Propylene",
    "Propyne",
    "R11",
    "R113",
    "R114",
    "R115",  
    "R116",
    "R12",
    "R123",
    "R1233zd(E)",
    "R1234yf",
    "R1234ze(E)",
    "R1234ze(Z)",
    "R124",
    "R1243zf",
    "R125", 
    "R13",
    "R1336mzz(E)",
    "R134a",
    "R13I1",
    "R14",
    "R141b",
    "R142b",
    "R143a",
    "R152A",
    "R161",
    "R21",
    "R218", 
    "R22",
    "R227EA",
    "R23",
    "R236EA",
    "R236FA",
    "R245ca",
    "R245fa",
    "R32",
    "R365MFC",
    "R40",
    "R404A",
    "R407C",
    "R41",
    "R410A",
    "R507A",
    "RC318",
    "SES36",
    "SulfurDioxide",
    "SulfurHexafluoride",
    "Toluene",
    "Water",
    "Xenon",
    "cis-2-Butene",
    "m-Xylene",
    "n-Butane",
    "n-Decane",
    "n-Dodecane",
    "n-Heptane",
    "n-Hexane",
    "n-Nonane",
    "n-Octane",
    "n-Pentane",
    "n-Propane",
    "n-Undecane",
    "o-Xylene",
    "p-Xylene",
    "trans-2-Butene"
  };

  const int kNumberOfComponents = sizeof(component_names) / sizeof(component_names[0]);

  // Implementation of FluidRegistry helper methods for temperature calculations
  double FluidRegistry::CalculateTemperatureFromEnthalpyAndPressure(
    size_t pkg_id,
    double H_target,
    double P,
    double T_guess,
    const std::vector<double>& mole_fractions
  ) const {
    auto fluid = GetFluidPackage(pkg_id);
    if (!fluid) {
      return T_guess; // Return guess if fluid package not available
    }

    try {
      // Set mole fractions if provided (for mixtures)
      auto components = GetComponents(pkg_id);
      if (components.size() > 1 && !mole_fractions.empty() && mole_fractions.size() == components.size()) {
        std::vector<double> z = mole_fractions;
        // Normalize to sum to 1.0
        double sum = 0.0;
        for (double val : z) {
          if (val < 0.0) val = 0.0;
          sum += val;
        }
        if (sum > 1e-10) {
          for (auto& val : z) val /= sum;
        } else {
          double equal_frac = 1.0 / z.size();
          for (auto& val : z) val = equal_frac;
        }
        fluid->set_mole_fractions(z);
      }

      // Validate initial guess
      if (T_guess <= 0 || T_guess < 50.0) T_guess = 300.0;

      double T_low = 50.0;
      double T_high = 2000.0;

      // Try to bracket the solution (validate that bracketing is possible)
      try {
        fluid->update(CoolProp::PT_INPUTS, P, T_low);
        (void)fluid->hmolar(); // Validate low bound
        fluid->update(CoolProp::PT_INPUTS, P, T_high);
        (void)fluid->hmolar(); // Validate high bound
      } catch (...) {
        // If bracketing fails, use simple linear interpolation
        try {
          fluid->update(CoolProp::PT_INPUTS, P, T_guess);
          double H_guess = fluid->hmolar();
          // Simple linear extrapolation: dH/dT ≈ Cp, assume Cp ~ 30 J/mol/K for gases
          double dT = (H_target - H_guess) / 30.0;
          T_guess += dT;
          if (T_guess < 100.0) T_guess = 100.0;
          if (T_guess > 1000.0) T_guess = 1000.0;
          return T_guess;
        } catch (...) {
          return T_guess;
        }
      }

      // For pure fluids, try direct calculation first
      if (components.size() == 1) {
        try {
          fluid->update(CoolProp::HmolarP_INPUTS, H_target, P);
          return fluid->T();
        } catch (...) {
          // Fall through to iterative method
        }
      }

      // Bisection method to find T
      const int max_iter = 20;
      const double tol = 1e-3; // 1 mK tolerance
      for (int iter = 0; iter < max_iter; ++iter) {
        T_guess = 0.5 * (T_low + T_high);
        try {
          fluid->update(CoolProp::PT_INPUTS, P, T_guess);
          double H_guess = fluid->hmolar();
          double error = H_guess - H_target;

          if (std::abs(error) < tol * std::abs(H_target) + 1e-6) {
            break; // Converged
          }

          if (error < 0) {
            T_low = T_guess;
          } else {
            T_high = T_guess;
          }

          if (T_high - T_low < 1e-6) break;
        } catch (...) {
          if (T_guess < 200.0) T_low = T_guess + 10.0;
          else T_high = T_guess - 10.0;
          if (T_high <= T_low) {
            return T_guess;
          }
        }
      }

      return T_guess;
    } catch (...) {
      return T_guess;
    }
  }

  double FluidRegistry::CalculateTemperatureFromEntropyAndPressure(
    size_t pkg_id,
    double S_target,
    double P,
    double T_guess,
    const std::vector<double>& mole_fractions
  ) const {
    auto fluid = GetFluidPackage(pkg_id);
    if (!fluid) {
      return T_guess; // Return guess if fluid package not available
    }

    try {
      // Set mole fractions if provided (for mixtures)
      auto components = GetComponents(pkg_id);
      if (components.size() > 1 && !mole_fractions.empty() && mole_fractions.size() == components.size()) {
        std::vector<double> z = mole_fractions;
        // Normalize to sum to 1.0
        double sum = 0.0;
        for (double val : z) {
          if (val < 0.0) val = 0.0;
          sum += val;
        }
        if (sum > 1e-10) {
          for (auto& val : z) val /= sum;
        } else {
          double equal_frac = 1.0 / z.size();
          for (auto& val : z) val = equal_frac;
        }
        fluid->set_mole_fractions(z);
      }

      // Validate initial guess
      if (T_guess <= 0 || T_guess < 50.0) T_guess = 300.0;

      double T_low = 50.0;
      double T_high = 2000.0;

      // Try to bracket the solution (validate that bracketing is possible)
      try {
        fluid->update(CoolProp::PT_INPUTS, P, T_low);
        (void)fluid->smolar(); // Validate low bound
        fluid->update(CoolProp::PT_INPUTS, P, T_high);
        (void)fluid->smolar(); // Validate high bound
      } catch (...) {
        // If bracketing fails, use simple linear interpolation
        try {
          fluid->update(CoolProp::PT_INPUTS, P, T_guess);
          double S_guess = fluid->smolar();
          // Simple linear extrapolation: dS/dT ≈ Cp/T, assume Cp ~ 30 J/mol/K
          double dT = (S_target - S_guess) * T_guess / 30.0;
          T_guess += dT;
          if (T_guess < 100.0) T_guess = 100.0;
          if (T_guess > 1000.0) T_guess = 1000.0;
          return T_guess;
        } catch (...) {
          return T_guess;
        }
      }

      // Bisection method to find T
      const int max_iter = 20;
      const double tol = 1e-3; // 1 mK tolerance
      for (int iter = 0; iter < max_iter; ++iter) {
        T_guess = 0.5 * (T_low + T_high);
        try {
          fluid->update(CoolProp::PT_INPUTS, P, T_guess);
          double S_guess = fluid->smolar();
          double error = S_guess - S_target;

          if (std::abs(error) < tol * std::abs(S_target) + 1e-6) {
            break; // Converged
          }

          if (error < 0) {
            T_low = T_guess;
          } else {
            T_high = T_guess;
          }

          if (T_high - T_low < 1e-6) break;
        } catch (...) {
          if (T_guess < 200.0) T_low = T_guess + 10.0;
          else T_high = T_guess - 10.0;
          if (T_high <= T_low) {
            return T_guess;
          }
        }
      }

      return T_guess;
    } catch (...) {
      return T_guess;
    }
  }

} // namespace px

