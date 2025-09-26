# Units {#Units}

YggdrasilRapidJSON adds classes for adding physical units to scalars and arrays. Instances of these classes can be used to perform arithmetic operations that track the units or perform unit conversions.

This section uses excerpts from `example/units/units.cpp`.

## Scalars

The class for scalars with units is `units::Quantity` which can be constructed from a value and unit string. The value can be any number type (e.g. float, double, int, uint64_t) with the type passed to `units::Quantity` as a template parameter. Unit strings contain common unit abbreviations/names and SI prefixes (e.g. "meter", "kg", "°C"). Units can be combined in unit strings via operators (e.g. "cm/s", "kg/cm**3").

~~~~~~~~~~cpp
#include "rapidjson/units.h"         // Unit classes
#include "rapidjson/internal/meta.h" // values_eq for floating point comparison

units::Quantity<float> x(5.0, "cm");
assert(internal::values_eq(x.value(), (float)5.0));
assert(x.unitsStr() == "cm");
~~~~~~~~~~

## Arrays

The class for arrays with units is `units::QuantityArray` which can be constructed from a C-style stack array and unit string

~~~~~~~~~~cpp
// 1D stack array
units::QuantityArray<int64_t> x({1, 2, 3, 4}, "cm");
assert(x.value()[2] == (int64_t)3);
assert(x.unitsStr() == "cm");
assert(x.nelements() == 4);
assert(x.ndim() == 1);
assert(x.shape()[0] == 4);

// 2D stack array
units::QuantityArray<uint32_t> y({{1, 2, 3, 4},
                                  {5, 6, 7, 8}}, "grams");
assert(y.value()[5] == (uint32_t)6); // Elements stored in column major order
assert(y.unitsStr() == "g");
assert(y.nelements() == 8);
assert(y.ndim() == 2);
assert(y.shape()[1] == 4);
~~~~~~~~~~

or a heap allocated array, unit string, shape, and number of dimensions

~~~~~~~~~~cpp
RAPIDJSON_DEFAULT_ALLOCATOR allocator;

// 1D heap array
SizeType length = 4;
int64_t* xarr = (int64_t*)allocator.Malloc(length * sizeof(int64_t));
for (int64_t i = 0; i < length; i++)
  xarr[i] = i + 1;
units::QuantityArray<int64_t> x(xarr, length, "cm");
allocator.Free(xarr);
assert(x.value()[2] == (int64_t)3);
assert(x.unitsStr() == "cm");
assert(x.nelements() == 4);
assert(x.ndim() == 1);
assert(x.shape()[0] == 4);

// 2D heap array
SizeType ndim = 2;
SizeType shape[2] = {2, 4};
uint32_t* yarr = (uint32_t*)allocator.Malloc(shape[0] * shape[1] * sizeof(uint32_t));
for (uint32_t i = 0; i < shape[0] * shape[1]; i++)
  yarr[i] = i + 1;
units::QuantityArray<uint32_t> y(yarr, ndim, shape, "grams");
assert(y.value()[5] == (uint32_t)6); // Elements stored in column major order
assert(y.unitsStr() == "g");
assert(y.nelements() == 8);
assert(y.ndim() == 2);
assert(y.shape()[1] == 4);
~~~~~~~~~~

## Operations

Operation can be performed between scalars, Quantity, & QuantityArray instances that keep track of the resulting units.

### Quantity + Scalar

~~~~~~~~~~cpp
~~~~~~~~~~

### Quantity + Quantity

~~~~~~~~~~cpp
~~~~~~~~~~

### Quantity + QuantityArray

~~~~~~~~~~cpp
~~~~~~~~~~


## Supported Units {#Supported}

All units are expressed in terms of their relationship to the base unit system which is the MKS (meter, kilogram, second) unit system.

### Base Units

| Name(s) | Abbreviation(s) |
| ------- | --------------- |
| meter; metre | m |
| gram; gramme | g |
| second | s |
| ampere; amp; Amp | A |
| kelvin; degree_kelvin | K; degK |
| mole | mol |
| candela | cd |
| radian | rad |

### Composite (SI Prefixable)

| Name(s) | Abbreviation(s) |
| ------- | --------------- |
| **cgs** | |
| dyne | dyn |
| erg | erg |
| barye | Ba |
| gauss | G |
| statcoulomb; esu; ESU; electrostatic_unit | statC |
| statampere | statA |
| statvolt | statV |
| statohm | statohm |
| maxwell | Mx |
| **MKS**    |     |
| joule | J |
| watt | W |
| hertz | Hz |
| newton | N |
| coulomb | C |
| tesla | T |
| pascal | Pa |
| bar | bar |
| volt | V |
| farad | F |
| henry | H |
| ohm; Ohm | Ω |
| weber | Wb |
| lumen | lm |
| lux | lx |
| celcius; degree_celsius; degree_Celsius; celsius | degC; °C |
| **Other** | |
| calorie | cal |
| year | yr |
| parsec | pc |
| electronvolt | eV |
| jansky | J; j |
| sievert | Sv |
| molar | M |

### Composite (Not SI Prefixable)

| Name(s) | Abbreviation(s) |
| ------- | --------------- |
| **Imperial** | |
| mil; thou; thousandth | mil
| incl | incl |
| feet | ft |
| yard | yd |
| mile | mi |
| furlong | fur |
| farenheit; degree_fahrenheit; degree_Fahrenheit | degF; °F
| rankine; degree_rankine | degR |
| pound_force | lbf |
| pound; pound_mass | lb; lbm |
| atmosphere | atm |
| horsepower | hp |
| ounce | oz |
| ton | ton |
| slug | slug |
| BTU; british_thermal_unit | BTU |
| pounds_per_square_inch | psi |
| smoot | smoot |
| percent | % |
| fraction | 100% |
| minute | min |
| hour | hr; h; hrs |
| day | day; d |
| **Astronomy** | |
| c | c |
| solar_mass; solMass; mass_sun | Msun; Msol; msun; m_sun; M_sun; m_Sun |
| solar_radius; solRadius | Rsun; Rsol; rsun; r_sun; R_sun; r_Sun |
| solar_luminosity; solLumin | Lsun; Lsol; lsun; l_sun; L_sun; l_Sun |
| solar_temperature; solTemperature | Tsun; Tsol; tsun; t_sun; T_sun; t_Sun |
| solar_metallicity; solMetallicity | Zsun; Zsol; zsun; z_sun; Z_sun; z_Sun |
| jupiter_mass | Mjup; m_jup |
| jupiter_radius | Rjup; r_jup |
| earth_mass | Mearth; m_earth |
| earth_radius | Rearth; r_earth |
| astronomical_unit | AU; au |
| light_year | ly |
| degree | deg |
| arcmin | arcmin |
| arcsec | arcsec |
| miliarcsec | mas |
| hourangle | hourangle |
| steradian | sr |
| latitude; degree_latitude | lat |
| longitude; degree_longitude | long |
| **Physics** |     |
| amu | amu |
| angstrom | Å |
| counts | counts |
| photons | photons |
| me | me |
| mp | mp |
| rayleigh | rayleigh |
| lambert | lambert |
| nit | nt |
| **Planck** | |
| m_pl | m_pl |
| l_pl | l_pl |
| t_pl | t_pl |
| T_pl | T_pl |
| q_pl | q_pl |
| E_pl | E_pl |
| **Geometrized** | |
| m_geom | m_geom |
| l_geom | l_geom |
| t_geom | t_geom |
| **Misc** | |
| acre | ac |
| are | a |
| hectare | ha |

## Supported SI Prefixes {#Prefixes}

| Abbreviation(s) | Scale Factor | Name |
| --------------- | ------------ | ---- |
| Y | 1e24 | yotta |
| Z | 1e21 | zetta |
| E | 1e18 | exa |
| P | 1e15 | peta |
| T | 1e12 | tera |
| G | 1e9 | giga |
| M | 1e6 | mega |
| k | 1e3 | kilo |
| h | 1e2 | hecto |
| da | 1e1 | deca |
| d | 1e-1 | deci |
| c | 1e-2 | centi |
| m | 1e-3 | mili |
| u; µ; μ | 1e-6 | micro |
| n | 1e-9 | nano |
| p | 1e-12 | pico |
| f | 1e-15 | femto |
| a | 1e-18 | atto |
| z | 1e-21 | zepto |
| y | 1e-24 | yocto |
