#pragma once


struct UnitXform {
    // base = ui * mul + add
    double mul;   // scale to base unit
    double add;   // offset to base unit
    double ToBase(double ui)   const { return ui * mul + add; }
    double FromBase(double base) const { return (base - add) / mul; }
};

struct UnitDef {
    const char* name;
    const char* symbol;
    UnitXform   xf;
};

struct QuantityEditState {
    int unit_index = 0;
    enum class Fmt { Auto, Fixed, Sci };
    Fmt sticky_fmt = Fmt::Auto;
};

// ----------------------------------------------------------------------------
// LENGTH (base: m)
// ----------------------------------------------------------------------------
static constexpr UnitDef kLengthUnits[] = {
    {"meter",      "m",   {1.0,    0.0}},
    {"centimeter", "cm",  {1e-2,   0.0}},
    {"millimeter", "mm",  {1e-3,   0.0}},
    {"micrometer", "um",  {1e-6,   0.0}},
    {"inch",       "in",  {0.0254, 0.0}},
    {"foot",       "ft",  {0.3048, 0.0}},
};

// ----------------------------------------------------------------------------
// MASS (base: kg)
// ----------------------------------------------------------------------------
static constexpr UnitDef kMassUnits[] = {
    {"kg",        "kg",   {1.0,               0.0}},
    {"g",         "g",    {1e-3,              0.0}},
    {"mg",        "mg",   {1e-6,              0.0}},
    {"tonne",     "t",    {1000.0,            0.0}},          // metric tonne
    {"pound",     "lb",   {0.45359237,        0.0}},          // international avoirdupois
};

// ----------------------------------------------------------------------------
// AREA (base: m²)
// ----------------------------------------------------------------------------
static constexpr UnitDef kAreaUnits[] = {
    {"square meter",     "m²",   {1.0,      0.0}},
    {"square centimeter","cm²",  {1e-4,     0.0}},   // 1 cm² = 1e-4 m²
    {"square millimeter","mm²",  {1e-6,     0.0}},   // 1 mm² = 1e-6 m²
};

// ----------------------------------------------------------------------------
// VOLUME (base: m3)
// ----------------------------------------------------------------------------
static constexpr UnitDef kVolumeUnits[] = {
{"m^3",   "m³",  {1.0,        0.0}},
{"liter", "L",   {1e-3,       0.0}},
{"mL",    "mL",  {1e-6,       0.0}},
{"ft^3",  "ft³", {0.028316846592, 0.0}},
};

// ----------------------------------------------------------------------------
// TIME (base: s)
// ----------------------------------------------------------------------------
static constexpr UnitDef kTimeUnits[] = {
    {"second",        "s",     {1.0,       0.0}},
    {"millisecond",   "ms",    {1e-3,      0.0}},    // 1 ms = 1e-3 s
    {"microsecond",   "µs",    {1e-6,      0.0}},    // 1 µs = 1e-6 s
    {"minute",        "min",   {60.0,      0.0}},    // 1 min = 60 s
    {"hour",          "h",     {3600.0,    0.0}},    // 1 h = 3600 s
    {"day",           "d",     {86400.0,   0.0}},    // 1 day = 86400 s
};

// ----------------------------------------------------------------------------
// PRESSURE (base: Pa)
// ----------------------------------------------------------------------------
static constexpr UnitDef kPressureUnits[] = {
    {"pascal",     "Pa",   {1.0,        0.0}},
    {"kilopascal", "kPa",  {1e3,        0.0}},
    {"megapascal", "MPa",  {1e6,        0.0}},
    {"bar",        "bar",  {1e5,        0.0}},
    {"atmosphere", "atm",  {101325.0,   0.0}},
    {"psi",        "psi",  {6894.757293168, 0.0}},
    {"torr",       "Torr", {133.32236842105263, 0.0}},
};

// ----------------------------------------------------------------------------
// ENERGY (base: J)
// ----------------------------------------------------------------------------
static constexpr UnitDef kEnergyUnits[] = {
{"joule",     "J",   {1.0,        0.0}},
{"kilojoule", "kJ",  {1e3,        0.0}},
{"megajoule", "MJ",  {1e6,        0.0}},
{"calorie",   "cal", {4.184,      0.0}},
{"kilocalorie", "kcal",  {4184.0,   0.0}},
{"Btu",       "Btu",   {1055.06,    0.0}},
};

// ----------------------------------------------------------------------------
// POWER (base: W)
// ----------------------------------------------------------------------------
static constexpr UnitDef kPowerUnits[] = {
    {"watt",         "W",      {1.0,          0.0}},     // base
    {"kilowatt",     "kW",     {1e3,          0.0}},
    {"megawatt",     "MW",     {1e6,          0.0}},
    {"gigawatt",     "GW",     {1e9,          0.0}},
    {"milliwatt",    "mW",     {1e-3,         0.0}},
};

// ----------------------------------------------------------------------------
// MOLAR AMOUNT (base: mol)
// ----------------------------------------------------------------------------
static constexpr UnitDef kMolarUnits[] = {
    {"mol",   "mol",   {1.0,    0.0}},   // base
    {"mmol",  "mmol",  {1e-3,   0.0}},
    {"kmol",  "kmol",  {1e3,    0.0}},  
};

// ----------------------------------------------------------------------------
// SPECIFIC MOLAR ENERGY (base: J/mol)
// ----------------------------------------------------------------------------
static constexpr UnitDef kSpecificMolarEnergyUnits[] = {
    {"J/mol",    "J/mol",    {1.0,     0.0}},
    {"kJ/mol",   "kJ/mol",   {1e3,     0.0}},
    {"MJ/mol",   "MJ/mol",   {1e6,     0.0}},
    {"cal/mol",  "cal/mol",  {4.184,   0.0}},   // 1 cal = 4.184 J
    {"kcal/mol", "kcal/mol", {4184.0,  0.0}},   // 1 kcal = 4184 J
    {"Btu/mol",  "Btu/mol",  {1055.06, 0.0}},   // 1 Btu ≈ 1055.06 J
};

// ----------------------------------------------------------------------------
// SPECIFIC ENERGY (mass-based) (base: J/kg)
// ----------------------------------------------------------------------------
static constexpr UnitDef kSpecificEnergyUnits[] = {
{"J/kg",     "J/kg",     {1.0,    0.0}},
{"kJ/kg",    "kJ/kg",    {1e3,    0.0}},     // 1 kJ = 1000 J
{"MJ/kg",    "MJ/kg",    {1e6,    0.0}},     // 1 MJ = 1,000,000 J
{"cal/g",    "cal/g",    {4184.0, 0.0}},     // 1 cal/g = 4184 J/kg
{"kcal/kg",  "kcal/kg",  {4184.0, 0.0}},     // 1 kcal/kg = 4184 J/kg
{"Btu/lb",   "Btu/lb",   {2326.0, 0.0}},     // 1 Btu/lb ≈ 2326 J/kg
};

// ----------------------------------------------------------------------------
// PRESSURE INVERSE (base: 1/Pa)
// ----------------------------------------------------------------------------
static constexpr UnitDef kPressureInverseUnits[] = {
    {"1/pascal",     "1/Pa",   {1.0,                      0.0}},
    {"1/kilopascal", "1/kPa",  {1.0 / 1e3,                0.0}},  // 1/kPa = 1 / 1000 Pa
    {"1/megapascal", "1/MPa",  {1.0 / 1e6,                0.0}},  // 1/MPa = 1 / 1e6 Pa
    {"1/bar",        "1/bar",  {1.0 / 1e5,                0.0}},  // 1/bar = 1 / 1e5 Pa
    {"1/atmosphere", "1/atm",  {1.0 / 101325.0,           0.0}},  // 1/atm = 1 / 101325 Pa
    {"1/psi",        "1/psi",  {1.0 / 6894.757293168,     0.0}},  // 1/psi = 1 / 6894.76 Pa
    {"1/torr",       "1/Torr", {1.0 / 133.32236842105263, 0.0}},  // 1/Torr = 1 / 133.32 Pa
};

// ----------------------------------------------------------------------------
// MOLAR PER PRESSURE (base: mol/Pa)
// ----------------------------------------------------------------------------
static constexpr UnitDef kMolarPerPressureUnits[] = {
{"mol/Pa",   "mol/Pa",   {1.0,                    0.0}},   // base
{"mol/kPa",  "mol/kPa",  {1e-3,                   0.0}},   // 1 kPa = 1000 Pa
{"mol/MPa",  "mol/MPa",  {1e-6,                   0.0}},   // 1 MPa = 1,000,000 Pa
{"mol/bar",  "mol/bar",  {1.0 / 1e5,              0.0}},   // 1 bar = 100,000 Pa
{"mol/atm",  "mol/atm",  {1.0 / 101325.0,         0.0}},   // 1 atm = 101325 Pa
{"mol/psi",  "mol/psi",  {1.0 / 6894.757293168,   0.0}},   // 1 psi ≈ 6894.76 Pa
{"mol/Torr", "mol/Torr", {1.0 / 133.322368421053, 0.0}},   // 1 Torr ≈ 133.32 Pa
};

// ----------------------------------------------------------------------------
// MOLAR PER MASS PER PRESSURE (base: mol / (kg·Pa))
// ----------------------------------------------------------------------------
static constexpr UnitDef kMolarPerMassPerPressureUnits[] = {
{"mol/(kg·Pa)",   "mol/(kg·Pa)",   {1.0,                    0.0}},  // base
{"mol/(kg·kPa)",  "mol/(kg·kPa)",  {1e-3,                   0.0}},  // 1 kPa = 1000 Pa
{"mol/(kg·MPa)",  "mol/(kg·MPa)",  {1e-6,                   0.0}},  // 1 MPa = 1e6 Pa
{"mol/(kg·bar)",  "mol/(kg·bar)",  {1.0 / 1e5,              0.0}},  // 1 bar = 100,000 Pa
{"mol/(kg·atm)",  "mol/(kg·atm)",  {1.0 / 101325.0,         0.0}},  // 1 atm = 101325 Pa
{"mol/(kg·psi)",  "mol/(kg·psi)",  {1.0 / 6894.757293168,   0.0}},  // 1 psi ≈ 6894.76 Pa
{"mol/(kg·Torr)", "mol/(kg·Torr)", {1.0 / 133.322368421053, 0.0}},  // 1 Torr ≈ 133.32 Pa
};

// ----------------------------------------------------------------------------
// TEMPERATURE (base: K) — affine conversions
// ----------------------------------------------------------------------------
static constexpr UnitDef kTemperatureUnits[] = {
    {"Kelvin",      "K",  {1.0,       0.0}},
    {"Celsius",     "°C", {1.0,     273.15}},                     // K = C + 273.15
    {"Fahrenheit",  "°F", {5.0/9.0, 255.37222222222223}},         // K = F*5/9 + 255.372...
};

// ----------------------------------------------------------------------------
// VELOCITY (base: m/s)
// ----------------------------------------------------------------------------
static constexpr UnitDef kVelocityUnits[] = {
    {"meter/second",        "m/s",   {1.0,        0.0}},
    {"centimeter/second",   "cm/s",  {1e-2,       0.0}},
    {"millimeter/second",   "mm/s",  {1e-3,       0.0}},
    {"meter/minute",        "m/min", {1.0/60.0,   0.0}},
    {"kilometer/hour",      "km/h",  {1000.0/3600.0, 0.0}}, 
    {"foot/second",         "ft/s",  {0.3048,     0.0}},
    {"mile/hour",           "mph",   {0.44704,    0.0}},
};

// ----------------------------------------------------------------------------
// VOLUMETRIC FLOW RATE (base: m^3/s)
// ----------------------------------------------------------------------------
static constexpr UnitDef kVolumetricFlowUnits[] = {
    {"m^3/s",           "m^3/s",  {1.0,                  0.0}},
    {"liter/second",    "L/s",    {1e-3,                 0.0}},
    {"liter/second",    "cm^3/s", {1e-6,                 0.0}},
    {"m^3/h",           "m^3/h",  {1.0/3600.0,           0.0}},
    {"liter/minute",    "L/min",  {1e-3/60.0,            0.0}},
    {"ccm",             "ccm",    {1e-6/60.0,            0.0}},
    {"milliliter/min",  "mL/min", {1e-6/60.0,            0.0}},
    {"ft^3/min (CFM)",  "ft^3/min",{0.028316846592/60.0, 0.0}},
    {"US gal/min",      "GPM",    {0.003785411784/60.0,  0.0}},
};

// ----------------------------------------------------------------------------
// MOLAR FLOW RATE (base: mol/s)
// ----------------------------------------------------------------------------
static constexpr UnitDef kMolarFlowUnits[] = {
    {"mol/s",    "mol/s",   {1.0,        0.0}},
    {"mol/min",  "mol/min", {1.0/60.0,   0.0}},
    {"mol/h",    "mol/h",   {1.0/3600.0, 0.0}},
    {"mmol/s",   "mmol/s",  {1e-3,       0.0}},
    {"umol/s",   "umol/s",  {1e-6,       0.0}},
    {"kmol/h",   "kmol/h",  {1000.0/3600.0, 0.0}},
    {"kmol/s",   "kmol/s",  {1000.0,     0.0}},
};

// ----------------------------------------------------------------------------
// DIMENSIONLESS (base: 1) — generic “no unit”
// ----------------------------------------------------------------------------
static constexpr UnitDef kDimensionlessUnits[] = {
    {"unitless", "-", {1.0, 0.0}},
};

// ----------------------------------------------------------------------------
// MOLE FRACTION (base: mol/mol) — offer %, ppm
// ----------------------------------------------------------------------------
static constexpr UnitDef kMoleFractionUnits[] = {
    {"mol/mol", "mol/mol", {1.0,    0.0}},
    {"percent", "%",       {0.01,   0.0}},
    {"ppm",     "ppm",     {1e-6,   0.0}},
};

// ----------------------------------------------------------------------------
// VISCOSITY (dynamic) (base: Pa·s)
// ----------------------------------------------------------------------------
static constexpr UnitDef kViscosityUnits[] = {
    {"Pa·s",   "Pa·s",  {1.0,    0.0}},
    {"mPa·s",  "mPa·s", {1e-3,   0.0}},      // = cP
    {"centipoise", "cP",{1e-3,   0.0}},
    {"poise",  "P",     {0.1,    0.0}},
    {"uPa·s",  "uPa·s", {1e-6,   0.0}},
};

// ----------------------------------------------------------------------------
// DENSITY (base: kg/m^3)
// ----------------------------------------------------------------------------
static constexpr UnitDef kDensityUnits[] = {
    {"kg/m^3",  "kg/m^3", {1.0,      0.0}},
    {"g/L",     "g/L",    {1.0,      0.0}},    // 1 g/L = 1 kg/m^3
    {"kg/L",    "kg/L",   {1000.0,   0.0}},
    {"g/mL",    "g/mL",   {1000.0,   0.0}},
    {"g/cm^3",  "g/cm^3", {1000.0,   0.0}},
    {"lb/ft^3", "lb/ft^3",{16.018463,0.0}},
};

// ----------------------------------------------------------------------------
// MOLAR CONCENTRATION (base: mol/m^3)
// ----------------------------------------------------------------------------
static constexpr UnitDef kMolarConcentrationUnits[] = {
    {"mol/m^3",  "mol/m^3", {1.0,   0.0}},
    {"mol/L (M)","mol/L",   {1000.0,0.0}},
    {"mmol/L",   "mmol/L",  {1.0,   0.0}},   // 1 mmol/L = 1 mol/m^3
    {"umol/L",   "umol/L",  {1e-3,  0.0}},
    {"kmol/m^3", "kmol/m^3",{1000.0,0.0}},
};

// ----------------------------------------------------------------------------
// INVERSE MOLAR CONCENTRATION (base: m³/mol)
// ----------------------------------------------------------------------------
static constexpr UnitDef kInverseMolarConcentrationUnits[] = {
    {"m³/mol",    "m³/mol",    {1.0,   0.0}},   // base
    {"L/mol",     "L/mol",     {1e-3,  0.0}},   // 1 L = 1e-3 m³
    {"L/mmol",    "L/mmol",    {1.0,   0.0}},   // 1 L/mmol = 1 m³/mol (mmol cancels)
    {"L/µmol",    "L/µmol",    {1e3,  0.0}},    // 1 µmol = 1e-6 mol
    {"m³/kmol",   "m³/kmol",   {1e-3,  0.0}},   // 1 kmol = 1000 mol
};

// ----------------------------------------------------------------------------
// ENERGY SOURCE (base: J/(m^3·s) == W/m^3)
// ----------------------------------------------------------------------------
static constexpr UnitDef kEnergySourceUnits[] = {
    {"W/m^3",   "W/m^3",  {1.0,     0.0}},   // same as J/(m^3·s)
    {"kW/m^3",  "kW/m^3", {1e3,     0.0}},
    {"W/L",     "W/L",    {1000.0,  0.0}},
    {"kW/L",    "kW/L",   {1e6,     0.0}},
};

// ----------------------------------------------------------------------------
// MOLAR SOURCE (your MASS_SOURCE) (base: mol/(m^3·s))
// ----------------------------------------------------------------------------
static constexpr UnitDef kMolarSourceUnits[] = {
    {"mol/(m^3·s)",   "mol/(m^3·s)", {1.0,        0.0}},
    {"mol/(m^3·h)",   "mol/(m^3·h)", {1.0/3600.0, 0.0}},
    {"mmol/(L·s)",    "mmol/(L·s)",  {1.0,        0.0}},   // 1 mmol/L = 1 mol/m^3
    {"mmol/(L·h)",    "mmol/(L·h)",  {1.0/3600.0, 0.0}},
    {"mol/(L·s)",     "mol/(L·s)",   {1000.0,     0.0}},
    {"mol/(L·h)",     "mol/(L·h)",   {1000.0/3600.0, 0.0}},
};

// ----------------------------------------------------------------------------
// HEAT CAPACITY (mass-based) (base: J/(kg·K))
// ----------------------------------------------------------------------------
static constexpr UnitDef kHeatCapacityUnits[] = {
    {"J/(kg·K)",        "J/(kg·K)",   {1.0,    0.0}},
    {"kJ/(kg·K)",       "kJ/(kg·K)",  {1e3,    0.0}},
    {"cal/(g·K)",       "cal/(g·K)",  {4184.0, 0.0}},     // 1 cal/gK = 4184 J/kgK
    {"Btu/(lb·°F)",     "Btu/(lb·°F)",{4186.8, 0.0}},     // ≈ 4186.8 J/kgK
};

// ----------------------------------------------------------------------------
// MOLAR HEAT CAPACITY (base: J/(mol·K))
// ----------------------------------------------------------------------------
static constexpr UnitDef kMolarHeatCapacityUnits[] = {
    {"J/(mol·K)",    "J/(mol·K)",   {1.0,     0.0}},
    {"kJ/(mol·K)",   "kJ/(mol·K)",  {1e3,     0.0}},     // 1 kJ = 1000 J
    {"cal/(mol·K)",  "cal/(mol·K)", {4.184,   0.0}},     // 1 cal = 4.184 J
    {"kcal/(mol·K)", "kcal/(mol·K)",{4184.0,  0.0}},     // 1 kcal = 4184 J
    {"Btu/(mol·K)",  "Btu/(mol·K)", {1055.06, 0.0}},     // 1 Btu ≈ 1055.06 J
};

// ----------------------------------------------------------------------------
// MOLAR MASS (base: kg/mol)
// ----------------------------------------------------------------------------
static constexpr UnitDef kMolarMassUnits[] = {
    {"kg/mol",     "kg/mol",    {1.0,    0.0}},
    {"g/mol",      "g/mol",     {1e-3,   0.0}},
    {"kg/kmol",    "kg/kmol",   {1e-3,   0.0}},   // same numeric as g/mol
    {"lbm/lbmol",  "lb/lbmol",  {1e-3,   0.0}},   // also same numeric as g/mol
};

// ----------------------------------------------------------------------------
// THERMAL CONDUCTIVITY & HEAT DISPERSION (base: W/(m·K))
// ----------------------------------------------------------------------------
static constexpr UnitDef kThermalConductivityUnits[] = {
    {"W/(m·K)",              "W/(m·K)",        {1.0,    0.0}},
    {"mW/(m·K)",             "mW/(m·K)",       {1e-3,   0.0}},
    {"W/(cm·K)",             "W/(cm·K)",       {100.0,  0.0}},
    {"Btu/(hr·ft·°F)",       "Btu/(h·ft·°F)",  {1.730735, 0.0}},
};

// ----------------------------------------------------------------------------
// HEAT TRANSFER COEFFICIENT (base: W/(m^2·K))
// ----------------------------------------------------------------------------
static constexpr UnitDef kHeatTransferCoeffUnits[] = {
    {"W/(m^2·K)",               "W/(m^2·K)",       {1.0,      0.0}},
    {"kW/(m^2·K)",              "kW/(m^2·K)",      {1e3,      0.0}},
    {"W/(cm^2·K)",              "W/(cm^2·K)",      {1e4,      0.0}},
    {"Btu/(hr·ft^2·°F)",        "Btu/(h·ft^2·°F)", {5.678263, 0.0}},
};

// ----------------------------------------------------------------------------
// Productivity (base: mol/(kg- s)
// ----------------------------------------------------------------------------
static constexpr UnitDef kProductivityUnits[] = {
{"kg_gas/(kg_solid·s)",     "kg_gas/(kg_solid·s)",     {1.0,           0.0}},      // base
{"kg_gas/(kg_solid·h)",     "kg_gas/(kg_solid·h)",     {1.0 / 3600.0,  0.0}},
{"tonne_gas/(kg_solid·s)",    "tonne_gas/(kg_solid·s)",    {1e3,           0.0}},
{"tonne_gas/(kg_solid·h)",    "tonne_gas/(kg_solid·h)",    {1e3 / 3600.0,  0.0}},
};

// ----------------------------------------------------------------------------
// PRESSURE GRADIENT (base: Pa/m)
// ----------------------------------------------------------------------------
static constexpr UnitDef kPressureGradientUnits[] = {
    {"Pa/m",     "Pa/m",     {1.0,      0.0}},
    {"kPa/m",    "kPa/m",    {1e3,      0.0}},
    {"MPa/m",    "MPa/m",    {1e6,      0.0}},
    {"bar/m",    "bar/m",    {1e5,      0.0}},
    {"atm/m",    "atm/m",    {101325.0, 0.0}},
    {"psi/m",    "psi/m",    {6894.757293168, 0.0}},
    {"psi/ft",   "psi/ft",   {6894.757293168/0.3048, 0.0}}, // ≈ 22620.632
};

// ----------------------------------------------------------------------------
// MASS TRANSFER COEFFICIENT (base: 1/s)
// ----------------------------------------------------------------------------
static constexpr UnitDef kInverseTimeUnits[] = {
    {"1/s",  "1/s",  {1.0,       0.0}},
    {"1/min","1/min",{1.0/60.0,  0.0}},
    {"1/h",  "1/h",  {1.0/3600.0,0.0}},
};

// ----------------------------------------------------------------------------
// DIFFUSIVITY / DISPERSION (base: m^2/s)
// ----------------------------------------------------------------------------
static constexpr UnitDef kDiffusivityUnits[] = {
    {"m^2/s",  "m^2/s", {1.0,   0.0}},
    {"cm^2/s", "cm^2/s",{1e-4,  0.0}},
    {"mm^2/s", "mm^2/s",{1e-6,  0.0}},
};

// ----------------------------------------------------------------------------
// ADSORBED CONCENTRATION (base: mol/kg)
// ----------------------------------------------------------------------------
static constexpr UnitDef kAdsorbedConcentrationUnits[] = {
    {"mol/kg",  "mol/kg",  {1.0,   0.0}},
    {"mmol/g",  "mmol/g",  {1.0,   0.0}},     // 1 mmol/g = 1 mol/kg
    {"mol/g",   "mol/g",   {1000.0,0.0}},
    {"mmol/kg", "mmol/kg", {1e-3,  0.0}},
    {"umol/g",  "umol/g",  {1e-3,  0.0}},
};

// ----------------------------------------------------------------------------
// COMPONENT MASS SOURCE (base: kg/(m^3·s))
// ----------------------------------------------------------------------------
static constexpr UnitDef kMassSourceUnits[] = {
    {"kg/(m^3·s)", "kg/(m^3·s)", {1.0,        0.0}},
    {"kg/(m^3·h)", "kg/(m^3·h)", {1.0/3600.0, 0.0}},
    {"g/(L·s)",    "g/(L·s)",    {1.0,        0.0}},   // 1 g/L = 1 kg/m^3
    {"g/(L·h)",    "g/(L·h)",    {1.0/3600.0, 0.0}},
    {"kg/(L·s)",   "kg/(L·s)",   {1000.0,     0.0}},
    {"mg/(L·s)",   "mg/(L·s)",   {1e-3,       0.0}},   // 1 mg/L = 1e-3 kg/m^3
};

struct UnitSet {
const UnitDef* data;
int size;
};

template <int N>
constexpr UnitSet MakeUnitSet(const UnitDef (&a)[N]) { return {a, N}; }
