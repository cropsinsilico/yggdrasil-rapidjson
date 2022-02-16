// Tencent is pleased to support the open source community by making RapidJSON available->
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip-> All rights reserved->
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License-> You may obtain a copy of the License at
//
// http://opensource->org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied-> See the License for the 
// specific language governing permissions and limitations under the License->

#ifndef RAPIDJSON_UNITS_H_
#define RAPIDJSON_UNITS_H_

#include "encodings.h"
#include "stream.h"
#include "stringbuffer.h"
#include "internal/meta.h"

RAPIDJSON_NAMESPACE_BEGIN

#ifdef RAPIDJSON_YGGDRASIL

namespace units {

  namespace constants {
    
    // Elementary masses
    static double mass_electron_kg = 9.10938291e-31;
    static double amu_kg = 1.660538921e-27;
    static double amu_grams = amu_kg * 1.0e3;
    static double mass_hydrogen_kg = 1.007947 * amu_kg;
    // static double mass_proton_kg = 1.672623110e-27;

    // Solar values (see Mamajek 2012)
    // https://sites.google.com/site/mamajeksstarnotes/bc-scale
    static double mass_sun_kg = 1.98841586e30;
    static double temp_sun_kelvin = 5870.0;
    static double luminosity_sun_watts = 3.8270e26;

    // Consistent with solar abundances used in Cloudy
    static double metallicity_sun = 0.01295;

    // Conversion Factors:  X au * mpc_per_au = Y mpc
    // length
    // static double mpc_per_mpc = 1e0;
    static double mpc_per_kpc = 1e-3;
    static double mpc_per_pc = 1e-6;
    static double mpc_per_au = 4.84813682e-12;
    static double mpc_per_rsun = 2.253962e-14;
    static double mpc_per_rearth = 2.06470307893e-16;
    static double mpc_per_rjup = 2.26566120943e-15;
    static double mpc_per_miles = 5.21552871e-20;
    static double mpc_per_km = 3.24077929e-20;
    static double mpc_per_m = 3.24077929e-23;
    static double kpc_per_m = mpc_per_m / mpc_per_kpc;
    static double pc_per_m = mpc_per_m / mpc_per_pc;
    static double km_per_pc = 3.08567758e13;
    // static double cm_per_pc = 3.08567758e18;
    // static double cm_per_mpc = 3.08567758e21;
    static double km_per_m = 1e-3;
    static double km_per_cm = 1e-5;
    static double m_per_cm = 1e-2;
    static double ly_per_m = 1.05702341e-16;
    static double rsun_per_m = 1.4378145e-9;
    static double rearth_per_m = 1.56961033e-7;  // Mean (volumetric) radius
    static double rjup_per_m = 1.43039006737e-8;  // Mean (volumetric) radius
    static double au_per_m = 6.68458712e-12;
    static double ang_per_m = 1.0e10;

    // static double m_per_fpc = 0.0324077929;

    static double kpc_per_mpc = 1.0 / mpc_per_kpc;
    static double pc_per_mpc = 1.0 / mpc_per_pc;
    static double au_per_mpc = 1.0 / mpc_per_au;
    static double rsun_per_mpc = 1.0 / mpc_per_rsun;
    static double rearth_per_mpc = 1.0 / mpc_per_rearth;
    static double rjup_per_mpc = 1.0 / mpc_per_rjup;
    static double miles_per_mpc = 1.0 / mpc_per_miles;
    static double km_per_mpc = 1.0 / mpc_per_km;
    static double m_per_mpc = 1.0 / mpc_per_m;
    static double m_per_kpc = 1.0 / kpc_per_m;
    static double m_per_km = 1.0 / km_per_m;
    static double cm_per_km = 1.0 / km_per_cm;
    static double cm_per_m = 1.0 / m_per_cm;
    static double pc_per_km = 1.0 / km_per_pc;
    static double m_per_pc = 1.0 / pc_per_m;
    static double m_per_ly = 1.0 / ly_per_m;
    static double m_per_rsun = 1.0 / rsun_per_m;
    static double m_per_rearth = 1.0 / rearth_per_m;
    static double m_per_rjup = 1.0 / rjup_per_m;
    static double m_per_au = 1.0 / au_per_m;
    static double m_per_ang = 1.0 / ang_per_m;

    // time
    // "IAU Style Manual" by G.A. Wilkins, Comm. 5, in IAU Transactions XXB (1989)
    // static double sec_per_Gyr = 31.5576e15;
    // static double sec_per_Myr = 31.5576e12;
    // static double sec_per_kyr = 31.5576e9;
    static double sec_per_year = 31.5576e6;
    static double sec_per_day = 86400.0;
    static double sec_per_hr = 3600.0;
    static double sec_per_min = 60.0;
    // static double day_per_year = 365.25;

    // velocities, accelerations
    static double speed_of_light_m_per_s = 2.99792458e8;
    static double speed_of_light_cm_per_s = speed_of_light_m_per_s * 100.0;
    static double standard_gravity_m_per_s2 = 9.80665;

    // some constants
    static double newton_mks = 6.67408e-11;
    static double planck_mks = 6.62606957e-34;
    // permeability of Free Space
    static double mu_0 = 4.0e-7 * M_PI;
    // permittivity of Free Space
    static double eps_0 = 1.0 / (pow(speed_of_light_m_per_s, 2) * mu_0);
    // static double avogadros_number = 6.02214085774e23;

    // temperature / energy
    static double boltzmann_constant_J_per_K = 1.3806488e-23;
    static double erg_per_eV = 1.602176562e-12;
    static double J_per_eV = erg_per_eV * 1.0e-7;
    static double erg_per_keV = erg_per_eV * 1.0e3;
    static double J_per_keV = J_per_eV * 1.0e3;
    static double K_per_keV = J_per_keV / boltzmann_constant_J_per_K;
    static double keV_per_K = 1.0 / K_per_keV;
    static double keV_per_erg = 1.0 / erg_per_keV;
    static double eV_per_erg = 1.0 / erg_per_eV;
    static double kelvin_per_rankine = 5.0 / 9.0;
    static double watt_per_horsepower = 745.69987158227022;
    static double celcius_zero_kelvin = -273.15;
    static double farenheit_zero_celcius = 32.0;
    static double farenheit_zero_kelvin = celcius_zero_kelvin / kelvin_per_rankine + farenheit_zero_celcius;
    // static double erg_per_s_per_watt = 1e7;

    // Solar System masses
    // Standish, E.M. (1995) "Report of the IAU WGAS Sub-Group on Numerical
    // Standards", in Highlights of Astronomy (I. Appenzeller, ed.), Table 1,
    // Kluwer Academic Publishers, Dordrecht.
    // REMARK: following masses include whole systems (planet + moons)
    static double mass_jupiter_kg = mass_sun_kg / 1047.3486;
    static double mass_mercury_kg = mass_sun_kg / 6023600.0;
    static double mass_venus_kg = mass_sun_kg / 408523.71;
    static double mass_earth_kg = mass_sun_kg / 328900.56;
    static double mass_mars_kg = mass_sun_kg / 3098708.0;
    static double mass_saturn_kg = mass_sun_kg / 3497.898;
    static double mass_uranus_kg = mass_sun_kg / 22902.98;
    static double mass_neptune_kg = mass_sun_kg / 19412.24;

    // flux
    static double jansky_mks = 1.0e-26;
    // Cosmological constants
    // Calculated with H = 100 km/s/Mpc, value given in units of h^2 g cm^-3
    // Multiply by h^2 to get the critical density in units of g cm^-3
    // static double rho_crit_g_cm3_h2 = 1.8788e-29;
    // static double primordial_H_mass_fraction = 0.76;

    // Misc. Approximations
    // static double mass_mean_atomic_cosmology = 1.22;
    // static double mass_mean_atomic_galactic = 2.3;

    // Miscellaneous
    // static double HUGE = 1.0e90;
    // static double TINY = 1.0e-40;

    // Planck units
    static double hbar_mks = 0.5 * planck_mks / M_PI;
    static double planck_mass_kg = sqrt(hbar_mks * speed_of_light_m_per_s / newton_mks);
    static double planck_length_m = sqrt(hbar_mks * newton_mks / pow(speed_of_light_m_per_s, 3));
    static double planck_time_s = planck_length_m / speed_of_light_m_per_s;
    static double planck_energy_J = planck_mass_kg * speed_of_light_m_per_s * speed_of_light_m_per_s;
    static double planck_temperature_K = planck_energy_J / boltzmann_constant_J_per_K;
    static double planck_charge_C = sqrt(4.0 * M_PI * eps_0 * hbar_mks * speed_of_light_m_per_s);

    // Imperial and other non-metric units
    static double kg_per_pound = 0.45359237;
    static double pascal_per_atm = 101325.0;
    static double m_per_inch = 0.0254;
    static double m_per_ft = 0.3048;

    // logarithmic units
    // IEC 60027-3: https://webstore.iec.ch/publication/94
    // NIST Special Publication 811: https://www.nist.gov/pml/special-publication-811
    // static double neper_per_bel = log(10) / 2;
  }

  static inline
  bool compare_doubles(const double &a, const double &b) {
    // double abs_precision = 1.0e-13; // DBL_EPSILON;
    double abs_precision = DBL_EPSILON;
    double rel_precision = DBL_EPSILON;
    if ((std::abs(a) < abs_precision) || (std::abs(b) < abs_precision))
      return (std::abs((a - b)*(b - a)) <= abs_precision);
    return (std::abs(((a - b)*(b - a)) / (a * b)) <= rel_precision);
  };

enum BaseDimension {
  kLengthUnit = 0,
  kMassUnit = 1,
  kTimeUnit = 2,
  kElectricCurrentUnit = 3,
  kTemperatureUnit = 4,
  kNumberUnit = 5,
  kLuminousIntensityUnit = 6,
  kAngleUnit = 7,
  kDimensionlessUnit = 8
};

class Dimension {
public:
  Dimension(const BaseDimension k, const double power=1.0) : Dimension() {
    if (k != kDimensionlessUnit)
      powers_[k] = power;
  }
  Dimension(const double L = 0.0,
	    const double M = 0.0,
	    const double T = 0.0,
	    const double EC = 0.0,
	    const double Temp = 0.0,
	    const double N = 0.0,
	    const double LI = 0.0,
	    const double A = 0.0) {
    powers_[0] = L;
    powers_[1] = M;
    powers_[2] = T;
    powers_[3] = EC;
    powers_[4] = Temp;
    powers_[5] = N;
    powers_[6] = LI;
    powers_[7] = A;
  }
  Dimension(const double powers[8]) :
    Dimension(powers[0], powers[1], powers[2], powers[3], powers[4],
	      powers[5], powers[6], powers[7]) {}
  Dimension operator*(const Dimension& x) const {
    double new_powers[8];
    for (size_t i = 0; i < 8; i++)
      new_powers[i] = powers_[i] + x.powers_[i];
    return Dimension(new_powers);
  }
  Dimension operator/(const Dimension& x) const {
    double new_powers[8];
    for (size_t i = 0; i < 8; i++)
      new_powers[i] = powers_[i] - x.powers_[i];
    return Dimension(new_powers);
  }
  void inplace_pow(const double x) {
    for (size_t i = 0; i < 8; i++)
      powers_[i] = powers_[i] * x;
  }
  Dimension pow(const double x) const {
    Dimension new_dim = Dimension(*this);
    new_dim.inplace_pow(x);
    return new_dim;
  }
  bool operator==(const Dimension& x) const {
    for (size_t i = 0; i < 8; i++)
      if (!(compare_doubles(powers_[i], x.powers_[i])))
	return false;
    return true;
  }
  bool operator!=(const Dimension& x) const { return (!(*this == x)); }
  bool is_irreducible() const {
    int ndim = 0;
    for (size_t i = 0; i < 8; i++)
      if (!(compare_doubles(powers_[i], 0.0)))
	ndim++;
    return (ndim == 1);
  }
  std::vector<Dimension> reduced() const {
    std::vector<Dimension> out;
    for (size_t i = 0; i < 8; i++)
      if (!(compare_doubles(powers_[i], 0.0)))
	out.push_back(Dimension((BaseDimension)i, powers_[i]));
    return out;
  }
protected:
  double powers_[8];
  friend std::ostream & operator << (std::ostream &os, const Dimension &x);
};
inline std::ostream & operator << (std::ostream& os, const Dimension &x) {
  os << "[" << x.powers_[0];
  for (size_t i = 1; i < 8; i++)
    os << "," << x.powers_[i];
  os << "]";
  return os;
};

namespace dimensions {
  // Base
  static Dimension dimensionless(0, 0);
  static Dimension length(1, 0);
  static Dimension mass(0, 1);
  static Dimension time(0, 0, 1);
  static Dimension current(0, 0, 0, 1);
  static Dimension temperature(0, 0, 0, 0, 1);
  static Dimension number(0, 0, 0, 0, 0, 1);
  static Dimension luminous_intensity(0, 0, 0, 0, 0, 0, 1);
  static Dimension angle(0, 0, 0, 0, 0, 0, 0, 1);
  // Derived
  static Dimension rate = dimensionless / time;
  static Dimension frequency = rate;
  static Dimension spatial_frequency = dimensionless / length;
  static Dimension solid_angle = angle * angle;
  static Dimension velocity = length / time;
  static Dimension acceleration = length / time.pow(2);
  static Dimension jerk = length / time.pow(3);
  static Dimension snap = length / time.pow(4);
  static Dimension crackle = length / time.pow(5);
  static Dimension pop = length / time.pow(6);
  static Dimension area = length * length;
  static Dimension volume = area * length;
  static Dimension momentum = mass * velocity;
  static Dimension force = mass * acceleration;
  static Dimension pressure = force / area;
  static Dimension energy = force * length;
  static Dimension power = energy / time;
  static Dimension flux = power / area;
  static Dimension specific_flux = flux / rate;
  static Dimension number_density = number / volume;
  static Dimension density = mass / volume;
  static Dimension angular_momentum = mass * length * velocity;
  static Dimension specific_energy = energy / mass;
  static Dimension count_flux = number / (area * time);
  static Dimension count_intensity = count_flux / solid_angle;
  static Dimension luminous_flux = luminous_intensity * solid_angle;
  static Dimension luminance = luminous_intensity / area;
  static Dimension charge_cgs = (energy * length).pow(2);
  static Dimension current_cgs = charge_cgs / time;
  static Dimension electric_field_cgs = charge_cgs / (length.pow(2));
  static Dimension magnetic_field_cgs = electric_field_cgs;
  static Dimension electric_potential_cgs = energy / charge_cgs;
  static Dimension resistance_cgs = electric_potential_cgs / current_cgs;
  static Dimension magnetic_flux_cgs = magnetic_field_cgs * area;
  static Dimension charge = current * time;
  static Dimension electric_field = force / charge;
  static Dimension magnetic_field = electric_field / velocity;
  static Dimension electric_potential = energy / charge;
  static Dimension resistance = electric_potential / current;
  static Dimension capacitance = charge / electric_potential;
  static Dimension magnetic_flux = magnetic_field * area;
  static Dimension inductance = magnetic_flux / current;
} // namespace dimensions


//! Unit prefix.
class UnitPrefix {
public:
  UnitPrefix(const char* abbr0, const double& factor0, const char* name0) :
    abbr(), factor(factor0), name() {
    strcpy(abbr, abbr0);
    strcpy(name, name0);
  }
  char abbr[3]; //! Abbreviation associated with the prefix.
  double factor; //! Factor that the prefix implies.
  char name[7]; //! Full name associated with the prefix.
};

// This dictionary formatting from magnitude package (secondarily via unyt), credit to Juan Reyero.
static std::vector<UnitPrefix> unit_prefixes {
  UnitPrefix("Y", 1e24, "yotta"),
  UnitPrefix("Z", 1e21, "zetta"),
  UnitPrefix("E", 1e18, "exa"),
  UnitPrefix("P", 1e15, "peta"),
  UnitPrefix("T", 1e12, "tera"),
  UnitPrefix("G", 1e9, "giga"),
  UnitPrefix("M", 1e6, "mega"),
  UnitPrefix("k", 1e3, "kilo"),
  UnitPrefix("h", 1e2, "hecto"),
  UnitPrefix("da", 1e1, "deca"),
  UnitPrefix("d", 1e-1, "deci"),
  UnitPrefix("c", 1e-2, "centi"),
  UnitPrefix("m", 1e-3, "mili"),
  UnitPrefix("µ", 1e-6, "micro"),  // ('MICRO SIGN' U+00B5)
  UnitPrefix("u", 1e-6, "micro"),
  UnitPrefix("μ", 1e-6, "micro"),  // ('GREEK SMALL LETTER MU' U+03BC)
  UnitPrefix("n", 1e-9, "nano"),
  UnitPrefix("p", 1e-12, "pico"),
  UnitPrefix("f", 1e-15, "femto"),
  UnitPrefix("a", 1e-18, "atto"),
  UnitPrefix("z", 1e-21, "zepto"),
  UnitPrefix("y", 1e-24, "yocto")
};


// Forward declarations
template<typename Ch=char>
class Unit;
template<typename Ch=char>
class Units;
template<typename T, typename Ch=char>
class Quantity;

//! Unit.
template<typename Ch>
class Unit {
public:
  Unit() : names_(), abbrs_(), dim_(), factor_(1.0), offset_(0.0), power_(1.0) {}
  Unit(const Ch* name, const Ch* abbr, const Dimension dim,
       const double factor=1.0, const double offset=0.0, const double power=1.0) :
    names_(), abbrs_(), dim_(dim), factor_(factor),
    offset_(offset), power_(power) {
    names_.push_back(std::basic_string<Ch>(name));
    abbrs_.push_back(std::basic_string<Ch>(abbr));
  }
  Unit(const std::vector<std::basic_string<Ch>> names,
       const std::vector<std::basic_string<Ch>> abbrs, const Dimension dim,
       const double factor=1.0, const double offset=0.0, const double power=1.0) :
    names_(names), abbrs_(abbrs), dim_(dim), factor_(factor),
    offset_(offset), power_(power) {
  }
  Unit(const std::basic_string<Ch> str, const double power=1.0) : Unit() {
    double prefix_factor = 1.0;
    const Unit* found = find_unit(str, prefix_factor);
    RAPIDJSON_ASSERT(found);
    names_.insert(names_.begin(), found->names_.begin(), found->names_.end());
    abbrs_.insert(abbrs_.begin(), found->abbrs_.begin(), found->abbrs_.end());
    dim_ = Dimension(found->dim_);
    factor_ = prefix_factor * found->factor_;
    offset_ = found->offset_;
    power_ = power; // Base units do not have powers
    RAPIDJSON_ASSERT(!(has_power() && has_offset()));
  }
  //! \brief Write the unit to an output stream with class information.
  //! \param os Output stream.
  void display(std::ostream& os) const {
    os << "Unit(\"" << names_[0] << "\", " << dim_ << ", " << factor_
       << ", " << offset_ << ")**" << power_;
  }
  //! \brief Get the dimensions of the unit, including the power.
  //! \return The dimensions of the unit.
  Dimension dimension() const { return dim_.pow(power_); }
  //! \brief Check if this unit is equal to another.
  //! \param x Unit to check against.
  //! \return true if this unit is equal to x.
  bool operator==(const Unit& x) const {
    if (names_ != x.names_) return false;
    if (abbrs_ != x.abbrs_) return false;
    if (dim_ != x.dim_) return false;
    if (!(compare_doubles(factor_, x.factor_))) return false;
    if (!(compare_doubles(offset_, x.offset_))) return false;
    if (!(compare_doubles(power_, x.power_))) return false;
    return true;
  }
  //! \brief Check if this unit is not equal to another.
  //! \param x Unit to check against.
  //! \return true if this unit is not equal to x.
  bool operator!=(const Unit& x) const { return (!(*this == x)); }
  //! \brief Perform power operation in place.
  //! \param x Power to raise this unit to.
  void inplace_pow(const double x) {
    RAPIDJSON_ASSERT(!(has_offset() && (!(compare_doubles(x, 1.0)))));
    power_ = power_ * x;
  }
  //! \brief Raise this unit to a power.
  //! \param x Power to raise this unit to.
  //! \return Resulting unit.
  Unit pow(const double x) const {
    Unit new_unit(*this);
    new_unit.inplace_pow(x);
    return new_unit;
  }
  //! \brief Check if this unit and another have the same base unit.
  //! \param x Unit to compare this unit to.
  //! \return true if this unit and x have the same base unit.
  bool is_same_base(const Unit& x) const {
    return (x.names_[0] == names_[0]);
  }
  //! \brief Check if a string matches any of the names or abbreviations
  //!   associated with this unit.
  //! \param str String to check.
  //! \return true if str matches this unit.
  bool matches(const std::basic_string<Ch> str) const {
    for (auto n = names_.begin(); n != names_.end(); n++) {
      if (str.compare(*n) == 0)
	return true;
      if (str.compare(*n + "s") == 0)
	return true;
    }
    for (auto n = abbrs_.begin(); n != abbrs_.end(); n++) {
      if (n->compare(str) == 0)
	return true;
    }
    return false;
  }
  //! \brief Check if a string matches any of the names or abbreviations
  //!   associated with this unit with the provided prefix added.
  //! \param str String to check.
  //! \param prefix Prefix to add when checking against the provided string.
  //! \return true if str matches this unit.
  bool matches(const std::basic_string<Ch> str, const UnitPrefix& prefix) const {
    for (auto n = names_.begin(); n != names_.end(); n++) {
      if (str.compare(prefix.name + (*n)) == 0)
	return true;
      if (str.compare(prefix.name + (*n) + "s") == 0)
	return true;
    }
    for (auto n = abbrs_.begin(); n != abbrs_.end(); n++)
      if (str.compare(prefix.abbr + (*n)) == 0)
	return true;
    return false;
  }
  //! \brief Check if a string matches any of the names or abbreviations
  //!   associated with this unit, checking for a partial match that allows
  //!   the possibility that there is a prefix present.
  //! \param str String to check.
  //! \param possibilities Vector to add this unit to if it matches the
  //!   end of the provided string.
  //! \return true if str matches this unit.
  bool prefix_matches(const std::basic_string<Ch> str,
		      std::vector<const Unit*>& possibilities) const {
    for (auto n = names_.begin(); n != names_.end(); n++) {
      if ((str.length() > n->length()) &&
	  (str.compare(str.length() - n->length(), n->length(), *n) == 0))
	possibilities.push_back(this);
      if ((str.length() > n->length()) &&
	  (str.compare(str.length() - (n->length() + 1), n->length() + 1, *n + "s") == 0))
	possibilities.push_back(this);
    }
    for (auto n = abbrs_.begin(); n != abbrs_.end(); n++) {
      if ((str.length() > n->length()) &&
	  (str.compare(str.length() - n->length(), n->length(), *n) == 0))
	possibilities.push_back(this);
    }
    return false;
  }
  //! \brief Check if this unit has a non-zero offset.
  //! \return true if this unit has a non-zero offset.
  bool has_offset() const { return (!(compare_doubles(offset_, 0.0))); }
  //! \brief Check if this unit has a power other than 1.
  //! \return true if this unit has a power other than 1.
  bool has_power() const { return (!(compare_doubles(power_, 1.0))); }
  //! \brief Check if this unit is irreducible or a product of more than
  //!   one irreducible unit.
  //! \return true if the unit is irreducible.
  bool is_irreducible() const { return dim_.is_irreducible(); }
  // Units reduced() const {
  //   if (is_irreducible()) return Units({*this});
  //   RAPIDJSON_ASSERT(not has_offset());
  //   // std::vector<Dimension> dims dim_.reduced();
  //   std::vector<Unit> units;
  //   for (size_t i = 0; i < 8; i++) {
  //     if (!(compare_doubles(dim_.powers_[i], 0.0))) {
  // 	Unit new_unit(base_units[i]);
  // 	// TODO: Check this, may need to divide by base unit factor in the
  // 	// case of grams
  // 	new_unit.inplace_pow(dim_.powers_[i]);
  // 	units.push_back(new_unit);
  //     }
  //   }
  //   units[0].factor_ = units[0].factor_ * factor_;
  //   return Units(units);
  // }
  // double to_base(const double x) { return factor_ * (x - offset_); }
  // double from_base(const double x) { return (x / factor_) + offset_; }
  //! \brief Get the conversion factors necessary to convert from this
  //!   unit to another.
  //! \param x Unit to convert to.
  //! \return Two element vector where the first element is the scale factor
  //!   and the second element is the offset.
  std::vector<double> conversion_factor(const Unit& x) const {
    RAPIDJSON_ASSERT(dimension() == x.dimension());
    // [a1 * (x - a2)]**a3 = [b1 * (y - b2)]**b3
    // y = [a1 * (x - a2)]**(a3/b3) / b1 + b2
    // v2 = a1 * (v1 - a2)
    // v3 = (v2 / b1) + b2
    // v3 = (a1 / b1) * (v1 - a2) + b2
    // v3 = (a1 / b1) * v1 - (a1 / b1) * a2 + b2
    // v3 = (a1 / b1) * v1 - ((a1 / b1) * a2 - b2)
    std::vector<double> a = conversion_factor();
    std::vector<double> b = x.conversion_factor();
    double ratio = a[0] / b[0];
    std::vector<double> out { ratio, a[1] - b[1] / ratio };
    return out;
  }
  //! \brief Get the conversion factors necessary to convert to/from this
  //!   unit from the base system of units.
  //! \return Two element vector where the first element is the scale factor
  //!   and the second element is the offset.
  std::vector<double> conversion_factor() const {
    std::vector<double> out { std::pow(factor_, power_), offset_ };
    return out;
  }
  //! \brief Find the unit that matches a string.
  //! \param str String to find a unit for.
  //! \param prefix_factor Prefix factor that is indicated by the string
  //!   prefix.
  //! \return Pointer to the matching unit.
  static const Unit<Ch>* find_unit(const std::basic_string<Ch> str, double& prefix_factor);
private:
  std::vector<std::basic_string<Ch>> names_;
  std::vector<std::basic_string<Ch>> abbrs_;
  Dimension dim_;
  double factor_;
  double offset_;
  double power_;
  friend class Units<Ch>;
  template<typename Ch2>
  friend std::ostream & operator << (std::ostream& os, const Unit<Ch2> &x);
};
template<typename Ch>
inline std::ostream & operator << (std::ostream& os, const Unit<Ch> &x) {
  os << x.abbrs_[0];
  if (x.has_power()) os << "**" << x.power_;
  return os;
};

template<typename Ch>
class Units {
public:
  Units() : units_() {}
  Units(std::initializer_list<Unit<Ch>> units) : units_(units) {}
  Units(const std::vector<Unit<Ch>> units) : units_(units) {}
  Units(const std::basic_string<Ch> str) : Units(str.c_str(), str.length()) {}
  Units(const Ch* str) : Units(str, strlen(str)) {}
  Units(const Ch str[], const size_t len, const bool verbose=false) : units_() {
    Units<Ch> new_units = parse_units(str, len, verbose);
    units_.insert(units_.begin(), new_units.units_.begin(), new_units.units_.end());
  }
  template<typename SourceEncoding, typename DestEncoding=ASCII<char>>
  static Units parse_units(const typename SourceEncoding::Ch* str, const size_t len, const bool verbose=false) {
    GenericStringStream<SourceEncoding> src(str);
    GenericStringBuffer<DestEncoding> dst;
    PutReserve(src, len);
    while (RAPIDJSON_LIKELY(src.Tell() < len)) {
      RAPIDJSON_ASSERT(src.Peek() != '\0');
      Transcoder<SourceEncoding, DestEncoding>::Transcode(src, dst);
    }
    // Pass pointer to token and populate this structure instead?
    return parse_units(dst.GetString(), dst.GetLength(), verbose);
  }
  void display(std::ostream& os) const {
    size_t i = 0;
    os << "Units([";
    for (auto it = units_.begin(); it != units_.end(); it++, i++) {
      if (i != 0) os << ",";
      it->display(os);
    }
    os << "])";
  }
  static Units parse_units(const Ch* str, const size_t len,
			   const bool verbose=false);  // Forward declaration
  Dimension dimension() const {
    Dimension out(dimensions::dimensionless);
    for (auto it = units_.begin(); it != units_.end(); it++)
      out = out * it->dimension();
    return out;
  }
  bool is_compatible(const Units& x) const {
    return (dimension() == x.dimension());
  }
  bool operator==(const Units& x) const {
    if (units_.size() != x.units_.size())
      return false;
    for (size_t i = 0; i < units_.size(); i++)
      if (units_[i] != x.units_[i])
	return false;
    return true;
  }
  bool operator!=(const Units& x) const { return (!(*this == x)); }
  Units operator*(const Units& x) const {
    RAPIDJSON_ASSERT(!(x.has_offset() || has_offset()));
    std::vector<Unit<Ch>> new_units(units_);
    for (auto it2 = x.units_.begin(); it2 != x.units_.end(); it2++) {
      size_t i = 0;
      for (i = 0; i < units_.size(); i++)
	if (it2->is_same_base(new_units[i]))
	  break;
      if (i < units_.size()) {
	// (a1*x)**a2 * (b1*x)**b2 = (a1**a2)*(b1**b2)*(x**(a2+b2))
	double new_power = new_units[i].power_ + it2->power_;
	new_units[i].factor_ = std::pow(std::pow(new_units[i].factor_,
						 new_units[i].power_) *
					std::pow(it2->factor_,
						 it2->power_),
					1.0 / new_power);
	new_units[i].power_ = new_power;
      } else {
	new_units.emplace_back(*it2);
      }
    }
    return Units(new_units);
  }
  Units operator/(const Units& x) const {
    return (*this) * (x.pow(-1)); }
  void inplace_pow(const double x) {
    for (auto it = units_.begin(); it != units_.end(); it++)
      it->inplace_pow(x);
  }
  Units pow(const double x) const {
    Units out(*this);
    out.inplace_pow(x);
    return out;
  }
  bool has_offset() const {
    for (auto it = units_.begin(); it != units_.end(); it++)
      if (it->has_offset())
	return true;
    return false;
  }
  // double to_base(const double x) {
  //   if (units_.size() == 1)
  //     return units_[0].to_base(x);
  //   double out = x;
  //   // V = (A^x)*(B^y)*(C^z) x ((A0/A)^x)*((B0/B)^y)*((C0/C)^z
  //   for (auto it = units_.begin(); it != units_.end(); it++) {
  //     RAPIDJSON_ASSERT(!(it->has_offset()));
      
  //   }
  //   return out;
  // }
  // double from_base(const double x) {
  //   return std::pow(x, 1.0 / power_) / factor_ + offset_; }
  std::vector<double> conversion_factor(const Units& x) const {
    if ((x.units_.size() == 1) && (units_.size() == 1))
      return units_[0].conversion_factor(x.units_[0]);
    RAPIDJSON_ASSERT(dimension() == x.dimension());
    std::vector<double> out {1.0, 0.0};
    for (auto it = units_.begin(); it != units_.end(); it++)
      out[0] = out[0] * it->conversion_factor()[0];
    for (auto it = x.units_.begin(); it != x.units_.end(); it++)
      out[0] = out[0] / it->conversion_factor()[0];
    return out;
  }
private:
  std::vector<Unit<Ch>> units_;
  friend class Quantity<Ch>;
  template<typename Ch2>
  friend std::ostream & operator << (std::ostream &os, const Units<Ch2> &x);
};
template<typename Ch>
inline std::ostream & operator << (std::ostream &os, const Units<Ch> &x) {
    size_t i = 0;
    for (auto it = x.units_.begin(); it != x.units_.end(); it++, i++) {
      if (i != 0) os << "*";
      os << "(" << *it << ")";
    }
    return os;
};

template<typename Ch>
Units<Ch> operator*(const Unit<Ch>& a, const Unit<Ch>& b) {
  RAPIDJSON_ASSERT(!(a.has_offset() || b.has_offset()));
  return Units<Ch>({a, b}); }
template<typename Ch>
Units<Ch> operator/(const Unit<Ch>& a, const Unit<Ch>& b) {
  return a * b.pow(-1); }


// MKS as base, units that can have SI prefixes
template<typename Ch>
std::vector<Unit<Ch>> get_base_units() {
  std::vector<Unit<Ch>> out {
    Unit<Ch>({"meter", "metre"}, {"m"}, dimensions::length),
    Unit<Ch>({"gram", "gramme"}, {"g"}, dimensions::mass, 1.0e-3),
    Unit<Ch>("second", "s", dimensions::time),
    Unit<Ch>({"ampere", "amp", "Amp"}, {"A"}, dimensions::current),
    Unit<Ch>({"kelvin", "degree_kelvin"}, {"K", "degK"}, dimensions::temperature),
    Unit<Ch>("mole", "mol", dimensions::number, 1.0 / constants::amu_grams),
    Unit<Ch>("candela", "cd", dimensions::luminous_intensity),
    Unit<Ch>("radian", "rad", dimensions::angle),
  };
  return out;
};
static std::vector<Unit<char>> base_units = get_base_units<char>();
  
template<typename Ch>
std::vector<Unit<Ch>> get_prefixable_units() {
  std::vector<Unit<Ch>> base = get_base_units<Ch>();
  std::vector<Unit<Ch>> out {
    // cgs
    Unit<Ch>("dyne", "dyn", dimensions::force, 1.0e-5),
    Unit<Ch>("erg", "erg", dimensions::energy, 1.0e-7),
    Unit<Ch>("barye", "Ba", dimensions::pressure, 0.1),
    Unit<Ch>("gauss", "G", dimensions::magnetic_field_cgs, pow(0.1, 0.5)),
    Unit<Ch>({"statcoulomb", "esu", "ESU", "electrostatic_unit"}, {"statC"}, dimensions::charge_cgs, pow(1.0e-3, 1.5)),
    Unit<Ch>("statampere", "statA", dimensions::current_cgs, pow(1.0e-3, 1.5)),
    Unit<Ch>("statvolt", "statV", dimensions::electric_potential_cgs, 0.1 * pow(1.0e-3, 1.5)),
    Unit<Ch>("statohm", "statohm", dimensions::resistance_cgs, 100.0),
    Unit<Ch>("maxwell", "Mx", dimensions::magnetic_flux_cgs, pow(1.0e-3, 1.5)),
    // SI
    Unit<Ch>("joule", "J", dimensions::energy),
    Unit<Ch>("watt", "W", dimensions::power),
    Unit<Ch>("hertz", "Hz", dimensions::rate),
    Unit<Ch>("newton", "N", dimensions::force),
    Unit<Ch>("coulomb", "C", dimensions::charge),
    Unit<Ch>("tesla", "T", dimensions::magnetic_field),
    Unit<Ch>("pascal", "Pa", dimensions::pressure),
    Unit<Ch>("bar", "bar", dimensions::pressure, 1.0e5),
    Unit<Ch>("volt", "V", dimensions::electric_potential),
    Unit<Ch>("farad", "F", dimensions::capacitance),
    Unit<Ch>("henry", "H", dimensions::inductance),
    Unit<Ch>({"ohm", "Ohm"}, {"Ω"}, dimensions::resistance),
    Unit<Ch>("weber", "Wb", dimensions::magnetic_flux),
    Unit<Ch>("lumen", "lm", dimensions::luminous_flux),
    Unit<Ch>("lux", "lx", dimensions::luminous_flux / dimensions::area),
    Unit<Ch>({"celcius", "degree_celsius", "degree_Celsius", "celsius"}, {"degC", "°C"}, dimensions::temperature, 1.0, constants::celcius_zero_kelvin),
    // other
    Unit<Ch>("calorie", "cal", dimensions::energy, 4.184),
    Unit<Ch>("year", "yr", dimensions::time, constants::sec_per_year),
    Unit<Ch>("parsec", "pc", dimensions::length, constants::m_per_pc),
    Unit<Ch>("electronvolt", "eV", dimensions::energy, constants::J_per_eV),
    Unit<Ch>("jansky", "J", dimensions::specific_flux, constants::jansky_mks),
    Unit<Ch>("sievert", "Sv", dimensions::specific_energy),
    Unit<Ch>("molar", "M", dimensions::number_density, 100.0 / constants::amu_grams),
  };
  out.insert(out.begin(), base.begin(), base.end());
  return out;
};
static std::vector<Unit<char>> prefixable_units = get_prefixable_units<char>();

template<typename Ch>
std::vector<Unit<Ch>> get_unprefixable_units() {
  std::vector<Unit<Ch>> out {
    // Imperial units
    Unit<Ch>({"mil", "thou", "thousandth"}, {"mil"}, dimensions::length, 1.0e-3 * constants::m_per_inch),
    Unit<Ch>("incl", "incl", dimensions::length, constants::m_per_inch),
    Unit<Ch>("feet", "ft", dimensions::length, constants::m_per_ft),
    Unit<Ch>("yard", "yd", dimensions::length, 0.9144),
    Unit<Ch>("mile", "mi", dimensions::length, 1609.344),
    Unit<Ch>("furlong", "fur", dimensions::length, constants::m_per_ft * 660.0),
    Unit<Ch>({"farenheit", "degree_fahrenheit", "degree_Fahrenheit"}, {"degF", "°F"}, dimensions::temperature, constants::kelvin_per_rankine, constants::farenheit_zero_kelvin),
    Unit<Ch>({"rankine", "degree_rankine"}, {"degR"}, dimensions::temperature, constants::kelvin_per_rankine),
    Unit<Ch>("pound_force", "lbf", dimensions::force, constants::kg_per_pound * constants::standard_gravity_m_per_s2),
    Unit<Ch>({"pound", "pound_mass"}, {"lb", "lbm"}, dimensions::mass, constants::kg_per_pound),
    Unit<Ch>("atmosphere", "atm", dimensions::pressure, constants::pascal_per_atm),
    Unit<Ch>("horsepower", "hp", dimensions::power, constants::watt_per_horsepower),
    Unit<Ch>("ounce", "oz", dimensions::mass, constants::kg_per_pound / 16.0),
    Unit<Ch>("ton", "ton", dimensions::mass, constants::kg_per_pound * 2000.0),
    Unit<Ch>("slug", "slug", dimensions::mass, constants::kg_per_pound * constants::standard_gravity_m_per_s2 / constants::m_per_ft),
    Unit<Ch>({"BTU", "british_thermal_unit"}, {"BTU"}, dimensions::energy, 1055.0559),
    Unit<Ch>("pounds_per_square_inch", "psi", dimensions::pressure, constants::kg_per_pound * constants::standard_gravity_m_per_s2 / pow(constants::m_per_inch, 2)),
    Unit<Ch>("smoot", "smoot", dimensions::length, 1.7018),
    Unit<Ch>("percent", "%", dimensions::dimensionless, 0.01),
    Unit<Ch>("minute", "min", dimensions::time, constants::sec_per_min),
    Unit<Ch>("hour", "hr", dimensions::time, constants::sec_per_hr),
    Unit<Ch>("day", "day", dimensions::time, constants::sec_per_day),
    // Astronomy units
    Unit<Ch>("c", "c", dimensions::velocity, constants::speed_of_light_m_per_s),
    Unit<Ch>({"solar_mass", "solMass", "mass_sun"}, {"Msun", "Msol", "msun", "m_sun", "M_sun", "m_Sun"}, dimensions::mass, constants::mass_sun_kg),
    Unit<Ch>({"solar_radius", "solRadius"}, {"Rsun", "Rsol", "rsun", "r_sun", "R_sun", "r_Sun"}, dimensions::length, constants::m_per_rsun),
    Unit<Ch>({"solar_luminosity", "solLumin"}, {"Lsun", "Lsol", "lsun", "l_sun", "L_sun", "l_Sun"}, dimensions::power, constants::luminosity_sun_watts),
    Unit<Ch>({"solar_temperature", "solTemperature"}, {"Tsun", "Tsol", "tsun", "t_sun", "T_sun", "t_Sun"}, dimensions::temperature, constants::temp_sun_kelvin),
    Unit<Ch>({"solar_metallicity", "solMetallicity"}, {"Zsun", "Zsol", "zsun", "z_sun", "Z_sun", "z_Sun"}, dimensions::dimensionless, constants::metallicity_sun),
    Unit<Ch>({"jupiter_mass"}, {"Mjup", "m_jup"}, dimensions::mass, constants::mass_jupiter_kg),
    Unit<Ch>({"jupiter_radius"}, {"Rjup", "r_jup"}, dimensions::length, constants::m_per_rjup),
    Unit<Ch>({"earth_mass"}, {"Mearth", "m_earth"}, dimensions::mass, constants::mass_earth_kg),
    Unit<Ch>({"earth_radius"}, {"Rearth", "r_earth"}, dimensions::length, constants::m_per_rearth),
    Unit<Ch>({"astronomical_unit"}, {"AU", "au"}, dimensions::length, constants::m_per_au),
    Unit<Ch>("light_year", "ly", dimensions::length, constants::m_per_ly),
    Unit<Ch>("degree", "deg", dimensions::angle, M_PI / 180.0),
    Unit<Ch>("arcmin", "arcmin", dimensions::angle, M_PI / 10800.0),
    Unit<Ch>("arcsec", "arcsec", dimensions::angle, M_PI / 648000.0),
    Unit<Ch>("miliarcsec", "mas", dimensions::angle, M_PI / 648000000.0),
    Unit<Ch>("hourangle", "hourangle", dimensions::angle, M_PI / 12.0),
    Unit<Ch>("steradian", "sr", dimensions::solid_angle),
    Unit<Ch>({"latitude", "degree_latitude"}, {"lat"}, dimensions::angle, -M_PI / 180.0, 90.0),
    Unit<Ch>({"longitude", "degree_longitude"}, {"long"}, dimensions::angle, M_PI / 180.0, -180.0),
    // Physics
    Unit<Ch>("amu", "amu", dimensions::mass, constants::amu_kg),
    Unit<Ch>("angstrom", "Å", dimensions::length, constants::m_per_ang),
    Unit<Ch>("counts", "counts", dimensions::number),
    Unit<Ch>("photons", "photons", dimensions::number),
    Unit<Ch>("me", "me", dimensions::mass, constants::mass_electron_kg),
    Unit<Ch>("mp", "mp", dimensions::mass, constants::mass_hydrogen_kg),
    Unit<Ch>("rayleigh", "rayleigh", dimensions::count_intensity, 2.5e9 / M_PI),
    Unit<Ch>("lambert", "lambert", dimensions::luminance, 1.0e4 / M_PI),
    Unit<Ch>("nit", "nt", dimensions::luminance),
    // Planck
    Unit<Ch>("m_pl", "m_pl", dimensions::mass, constants::planck_mass_kg),
    Unit<Ch>("l_pl", "l_pl", dimensions::length, constants::planck_length_m),
    Unit<Ch>("t_pl", "t_pl", dimensions::time, constants::planck_time_s),
    Unit<Ch>("T_pl", "T_pl", dimensions::temperature, constants::planck_temperature_K),
    Unit<Ch>("q_pl", "q_pl", dimensions::charge, constants::planck_charge_C),
    Unit<Ch>("E_pl", "E_pl", dimensions::energy, constants::planck_energy_J),
    // Geometrized
    Unit<Ch>("m_geom", "m_geom", dimensions::mass, constants::mass_sun_kg),
    Unit<Ch>("l_geom", "l_geom", dimensions::length, constants::newton_mks * constants::mass_sun_kg / pow(constants::speed_of_light_m_per_s, 2)),
    Unit<Ch>("t_geom", "t_geom", dimensions::time, constants::newton_mks * constants::mass_sun_kg / pow(constants::speed_of_light_m_per_s, 3)),
    // Logarithmic
    // Unit<Ch>("bel", "B", dimensions::logarithmic, neper_per_bel),
    // Unit<Ch>("neper", "Np", dimensions::logarithmic),
    // misc
    Unit<Ch>("acre", "ac", dimensions::area, 4046.86),
    Unit<Ch>("are", "a", dimensions::area, 100.0),
    Unit<Ch>("hectare", "ha", dimensions::area, 10000.0),
  };
  return out;
};
static std::vector<Unit<char>> unprefixable_units = get_unprefixable_units<char>();

template<typename Ch>
const Unit<Ch>* Unit<Ch>::find_unit(const std::basic_string<Ch> str, double& prefix_factor) {
  prefix_factor = 1.0;
  size_t idx_beg, idx_end;
  std::basic_string<Ch> whitespace = " \t\f\v\n\r";
  idx_beg = str.find_first_not_of(whitespace);
  idx_end = str.find_last_not_of(whitespace);
  RAPIDJSON_ASSERT(idx_end != std::string::npos);
  std::basic_string<Ch> substr = str.substr(idx_beg, idx_end + 1);
  std::vector<const Unit<Ch>*> possibilities;
  for (auto it = prefixable_units.begin(); it != prefixable_units.end(); it++) {
    if (it->matches(substr))
      return &(*it);
    it->prefix_matches(substr, possibilities);
  }
  for (auto it = unprefixable_units.begin(); it != unprefixable_units.end(); it++)
    if (it->matches(substr))
      return &(*it);
  if (possibilities.size() > 0) {
    for (auto it = possibilities.begin(); it != possibilities.end(); it++) {
      for (auto p = unit_prefixes.begin(); p != unit_prefixes.end(); p++) {
	if ((*it)->matches(substr, *p)) {
	  prefix_factor = p->factor;
	  return *it;
	}
      }      
    }
  }
  std::cerr << "No match found for \"" << substr << "\"" << std::endl; // GCOVR_EXCL_LINE
  return nullptr; // GCOVR_EXCL_LINE
};

namespace parser {

enum TokenType {
  kWhitespaceToken = 0,
  kOperatorToken = 1,
  kWordToken = 2,
  kGroupToken = 3
};

template<typename Ch>
class GroupToken; // Forward declaration

template<typename Ch>
class TokenBase {
public:
  TokenBase(const TokenBase& rhs) : t(rhs.t), parent(rhs.parent), units(rhs.units), finalized(rhs.finalized) {}
  TokenBase(const TokenType t0, TokenBase *parent0=nullptr) : t(t0), units(), finalized(false), parent(parent0) {}
  virtual ~TokenBase() {}
  virtual TokenBase<Ch>* current_token() { return this; }
  virtual GroupToken<Ch>* current_group() { return nullptr; }
  virtual Units<Ch> finalize() {
    finalized = true;
    return units;
  }
  virtual double value() { return 0.0; } // GCOVR_EXCL_LINE
  virtual bool is_numeric() { return false; }
  virtual void append(const Ch) {} // GCOVR_EXCL_LINE
  virtual std::ostream & display(std::ostream &os) const {
    os << "TokenBase(" << t << ")";
    return os;
  }
  TokenType t;
  Units<Ch> units;
  bool finalized;
  TokenBase<Ch> *parent;
  template<typename Ch2>
  friend std::ostream & operator << (std::ostream &os, const TokenBase<Ch2>* x);
};
template<typename Ch2>
inline std::ostream & operator << (std::ostream &os, const TokenBase<Ch2>* x) {
  return x->display(os);
};
  
template<typename Ch>
class OperatorToken : public TokenBase<Ch> {
public:
  OperatorToken(const Ch op0, TokenBase<Ch> *parent0=nullptr) : TokenBase<Ch>(kOperatorToken, parent0), op(op0) { this->finalize(); }
  Units<Ch> operate(const Units<Ch>& a, const Units<Ch>& b) {
    switch (op) {
    case '*':
      return a * b;
    case '/':
      return a / b;
    default:
      RAPIDJSON_ASSERT((op == '*') || (op == '/')); // GCOVR_EXCL_LINE
    }
    return this->units; // GCOVR_EXCL_LINE
  }
  Units<Ch> operate(const Units<Ch>& a, const double& b) {
    RAPIDJSON_ASSERT(op == '^');
    return a.pow(b);
  }
  double operate(const double& a, const double& b) {
    switch (op) {
    case '*':
      return a * b;
    case '/':
      return a / b;
    case '^':
      return std::pow(a, b);
    case '+':
      return a + b;
    case '-':
      return a - b;
    default:
      RAPIDJSON_ASSERT((op == '*') || (op == '/') || (op == '^') // GCOVR_EXCL_LINE
		       || (op == '+') || (op == '-'));
    }
    return 0.0; // GCOVR_EXCL_LINE
  }
  bool is_numeric() override { return true; }
  bool is_exp() { return (op == '^'); }
  bool matches(const std::vector<char> ops) {
    for (auto iop = ops.begin(); iop != ops.end(); iop++)
      if (*iop == op)
	return true;
    return false;
  }
  std::ostream & display(std::ostream &os) const override {
    os << "OperatorToken(" << op << ")";
    return os;
  }
  Ch op;
};

template<typename Ch>
class NumberToken; // Forward declaration
  
template<typename Ch>
class WordToken : public TokenBase<Ch> {
public:
  WordToken(const Ch c, TokenBase<Ch> *parent0=nullptr) : TokenBase<Ch>(kWordToken, parent0), word() {
    word.push_back(c);
  }
  void append(const Ch c) override {
    word.push_back(c);
  }
  Units<Ch> finalize() override {
    if (!(this->finalized))
      this->units = Units<Ch>({Unit<Ch>(word)});
    return TokenBase<Ch>::finalize();
  }
  std::ostream & display(std::ostream &os) const override {
    os << "WordToken(" << word << ")";
    return os;
  }
  std::basic_string<Ch> word;
  friend NumberToken<Ch>;
};

template<typename Ch>
class NumberToken : public WordToken<Ch> {
public:
  NumberToken(const Ch c, TokenBase<Ch> *parent0=nullptr) : WordToken<Ch>(c, parent0) {}
  bool is_numeric() override { return true; }
  double value() override { return atof(this->word.c_str()); }
  Units<Ch> finalize() override {
    return TokenBase<Ch>::finalize();
  }
  std::ostream & display(std::ostream &os) const override {
    os << "NumericToken(" << this->word << ")";
    return os;
  }
};

template<typename Ch>
class GroupToken : public TokenBase<Ch> {
public:
  GroupToken(TokenBase<Ch> *parent0=nullptr) : TokenBase<Ch>(kGroupToken, parent0), tokens(), value_(0.0) {}
  ~GroupToken() override {
    for (size_t i = 0; i < tokens.size(); i++)
      delete tokens[i];
    tokens.clear();
  }
  TokenBase<Ch>* current_token() override {
    if (tokens.size() == 0)
      return TokenBase<Ch>::current_token();
    return tokens[tokens.size() - 1]->current_token();
  }
  GroupToken<Ch>* current_group() override {
    int idx = (int)(tokens.size()) - 1;
    if ((idx >= 0) && (tokens[(size_t)idx]->t == kGroupToken)
	&& (!(tokens[(size_t)idx]->finalized)))
      return tokens[(size_t)idx]->current_group();
    return this;
  }
  OperatorToken<Ch>* append_op(const Ch c, bool dont_descend=false) {
    OperatorToken<Ch>* op = new OperatorToken<Ch>(c, this);
    append(op, dont_descend);
    return op;
  }
  void append(const Ch c) override {
    TokenBase<Ch>* curr = current_token();
    if ((curr->t == kGroupToken) || (curr->finalized)) {
      switch (c) {
      case '-':
      case '+':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
	curr = append(new NumberToken<Ch>(c, (TokenBase<Ch>*)(this)));
	break;
      default:
	curr = append(new WordToken<Ch>(c, (TokenBase<Ch>*)(this)));
      }
      return;
    } else if ((c == '-') || (c == '+')) {
      append_op(c);
      return;
    }
    RAPIDJSON_ASSERT(curr->t == kWordToken);
    WordToken<Ch>* word = static_cast<WordToken<Ch>*>(curr);
    word->append(c);
  }
  TokenBase<Ch>* append(TokenBase<Ch>* x, bool dont_descend=false) {
    GroupToken<Ch>* curr;
    if (dont_descend)
      curr = this;
    else {
      curr = current_group();
      if (curr->tokens.size() > 0) {
	TokenBase<Ch>* prev = curr->current_token();
	if (!(prev->finalized))
	  prev->finalize();
	if ((x->t != kOperatorToken) && (prev->t != kOperatorToken))
	  curr->append_op('*', true);
      }
    }
    RAPIDJSON_ASSERT(!(curr->finalized));
    curr->tokens.push_back(x);
    x->parent = curr;
    return x;
  }
  void group_operators(const char op) {
    std::vector<char> ops {op};
    group_operators(ops);
  }
  void group_operators(const std::vector<char> ops) {
    if (tokens.size() <= 3)
      return;
    std::vector<size_t> exponents;
    for (size_t i = 1; i < tokens.size(); i++) {
      if (tokens[i]->t == kOperatorToken) {
	OperatorToken<Ch> *op = static_cast<OperatorToken<Ch>*>(tokens[i]);
	if (op->matches(ops)) {
	  RAPIDJSON_ASSERT((i + 1) < tokens.size());
	  GroupToken<Ch>* new_group = new GroupToken<Ch>(this);
	  for (size_t ii = i - 1; ii <= (i + 1); ii++) {
	    tokens[ii]->parent = new_group;
	    new_group->append(tokens[ii]);
	    tokens[ii] = nullptr;
	  }
	  tokens[i + 1] = (TokenBase<Ch>*)(new_group);
	  new_group->finalize();
	  exponents.push_back(i - 1);
	  exponents.push_back(i);
	  i++;
	}
      }
    }
    for (auto it = exponents.rbegin(); it != exponents.rend(); it++)
      tokens.erase(tokens.begin() + (int)(*it));
  }
  Units<Ch> finalize() override {
    if ((tokens.size() == 0) || (this->finalized))
      return this->units;
    // Group operators first in order of operations
    if (tokens.size() > 3) {
      group_operators('^');
      group_operators({'*', '/'});
      group_operators({'+', '-'});
    }
    // Complete operations from left to right
    Units<Ch> out = tokens[0]->finalize();
    if (is_numeric()) {
      value_ = tokens[0]->value();
      for (size_t i = 1; i < tokens.size(); i = i+2) {
	RAPIDJSON_ASSERT(tokens[i]->t == kOperatorToken);
	RAPIDJSON_ASSERT(tokens[i + 1]->t != kOperatorToken);
	OperatorToken<Ch> *op = static_cast<OperatorToken<Ch>*>(tokens[i]);
	value_ = op->operate(value_, tokens[i + 1]->value());
      }
    } else {
      for (size_t i = 1; i < tokens.size(); i = i+2) {
	RAPIDJSON_ASSERT(tokens[i]->t == kOperatorToken);
	RAPIDJSON_ASSERT(tokens[i + 1]->t != kOperatorToken);
	OperatorToken<Ch> *op = static_cast<OperatorToken<Ch>*>(tokens[i]);
	if (tokens[i + 1]->is_numeric()) {
	  out = op->operate(out, tokens[i + 1]->value());
	} else {
	  out = op->operate(out, tokens[i + 1]->finalize());
	}
      }
    }
    this->units = out;
    return TokenBase<Ch>::finalize();
  }
  bool is_numeric() override {
    for (auto it = tokens.begin(); it != tokens.end(); it++) {
      if (!((*it)->is_numeric()))
	return false;
    }
    return true;
  }
  double value() override {
    RAPIDJSON_ASSERT(is_numeric());
    finalize();
    return value_;
  }
  std::ostream & display(std::ostream &os) const override {
    os << "GroupToken(";
    size_t i = 0;
    for (auto it = tokens.begin(); it != tokens.end(); it++, i++) {
      if (i > 0) os << ", ";
      (*it)->display(os);
    }
    os << ")";
    return os;
  }
  std::vector<TokenBase<Ch>*> tokens;
  double value_;
};

} // namespace parser


template<typename Ch>
Units<Ch> Units<Ch>::parse_units(const Ch* str, const size_t len,
				 const bool verbose) {
  if (verbose)
    std::cout << "parse_units(\"" << str << "\")" << std::endl;
  size_t i = 0;
  parser::GroupToken<Ch> token;
  Ch c;
  for (i = 0; i < len; i++) {
    c = str[i];
    switch (c) {
    case '(':
    case '[': // TODO: Concentration?
    case '{': {
      token.append(new parser::GroupToken<Ch>());
      break;
    }
    case ')':
    case ']':
    case '}': {
      token.current_group()->finalize();
      break;
    }
    // + and - operators will be handled by the word token append
    case '/':
    case '^': {
      token.append_op(c);
      break;
    }
    case '*': {
      if (str[i + 1] == '*') {
	c = '^';
	i++;
      }
      token.append_op(c);
      break;
    }
    case ' ':
    case '\t':
    case '\n':
    case '\v':
    case '\f':
    case '\r': {
      parser::TokenBase<Ch>* word = token.current_token();
      if (word->t == parser::kWordToken)
	word->finalize();
      break;
    }
    default:
      token.append(c);
    }
    if (verbose) {
      std::cout << c << ": ";
      token.display(std::cout);
      std::cout << std::endl;
    }
  }
  return token.finalize();
};

template<typename T, typename Ch>
class Quantity {
public:
  Quantity(const T& value, const Ch* units) : value_(value), units_(units) {}
  Quantity(const T& value, const Units<Ch>& units) : value_(value), units_(units) {}
  void display(std::ostream& os) const {
    os << "Quantity(" << value_ << ", ";
    units_.display(os);
    os << ")";
  }
private:
  Quantity raw_add(const Quantity& x, double factor=1.0) const {
    // Assumes units have already been matched
    return Quantity(value_ + (factor * x.value_), units_); }
public:
  T value() const { return value_; };
  Units<Ch> units() const { return units_; }
  bool operator==(const Quantity& x) const {
    if (units_ != x.units_)
      return false;
    return compare_doubles(value_, x.value_);
  }
  bool operator!=(const Quantity& x) const { return (!(*this==x)); }
  Quantity operator*(const Quantity& x) const {
    return Quantity(value_ * x.value_, units_ * x.units_); }
  Quantity operator/(const Quantity& x) const {
    return Quantity(value_ / x.value_, units_ / x.units_); }
  template<typename T2>
  Quantity operator*(const T2& x) const {
    return Quantity(value_ * x, units_); }
  template<typename T2>
  Quantity operator/(const T2& x) const {
    return Quantity(value_ / x, units_); }
  Quantity operator+(const Quantity& x) const {
    if (units_ != x.units_)
      return x.as(units_);
    return raw_add(x); }
  Quantity operator-(const Quantity& x) const {
    if (units_ != x.units_)
      return x.as(units_);
    return raw_add(x, -1.0); }
  void inplace_pow(const double& x) {
    value_ = std::pow(value_, x);
    units_.inplace_pow(x);
  }
  Quantity pow(const double& x) const {
    Quantity out(*this);
    out.inplace_pow(x);
    return out;
  }
  Dimension dimension() const { return units_.dimension(); }
  bool is_compatible(const Quantity& x) const {
    return (dimension() == x.dimension());
  }
  bool equivalent_to(const Quantity& x) {
    if (!(is_compatible(x)))
      return false;
    return (*this==x.as(units_));
  }
  void convert_to(const Units<Ch>& units) {
    std::vector<double> factor = units_.conversion_factor(units);
    value_ = factor[0] * (value_ - factor[1]);
    units_ = Units<Ch>(units);
  }
  Quantity as(const char* units0) {
    Units<Ch> units(units0);
    return as(units);
  }
  Quantity as(const Units<Ch>& units) const {
    Quantity out(*this);
    out.convert_to(units);
    return out;
  }
private:
  double value_;
  Units<Ch> units_;
  template<typename U, typename Ch2>
  friend std::ostream & operator << (std::ostream &os, const Quantity<U,Ch2> &x);
};
template<typename T, typename Ch=char>
inline std::ostream & operator << (std::ostream &os, const Quantity<T, Ch> &x) {
    os << x.value_ << " " << x.units_;
    return os;
};

} // namespace units
  
#endif // RAPIDJSON_YGGDRASIL

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_UNITS_H_
