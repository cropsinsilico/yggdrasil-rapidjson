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
#include <wchar.h>
#include <locale.h>
#include <typeindex>

RAPIDJSON_NAMESPACE_BEGIN

#ifdef RAPIDJSON_YGGDRASIL

namespace units {

  template<typename T>
  inline bool compare_values(const T& a, const T& b) {
    return compare_values(static_cast<double>(a), static_cast<double>(b));
  }
  template<>
  inline bool compare_values(const double &a, const double &b) {    
    // double abs_precision = 1.0e-13; // DBL_EPSILON;
    double abs_precision = DBL_EPSILON;
    double rel_precision = DBL_EPSILON;
    if ((std::abs(a) < abs_precision) || (std::abs(b) < abs_precision))
      return (std::abs((a - b)*(b - a)) <= abs_precision);
    return (std::abs(((a - b)*(b - a)) / (a * b)) <= rel_precision);
  }
  template<>
  inline bool compare_values(const std::complex<float>& a,
			     const std::complex<float>& b) {
    if (!compare_values(a.real(), b.real())) return false;
    return compare_values(a.imag(), b.imag());
  }
  template<>
  inline bool compare_values(const std::complex<double>& a,
			     const std::complex<double>& b) {
    if (!compare_values(a.real(), b.real())) return false;
    return compare_values(a.imag(), b.imag());
  }
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
  template<>
  inline bool compare_values(const std::complex<long double>& a,
			     const std::complex<long double>& b) {
    if (!compare_values(a.real(), b.real())) return false;
    return compare_values(a.imag(), b.imag());
  }
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE

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

  template<typename T>
  class CachedLUT {
  public:
    CachedLUT(const std::vector<T> base, const std::vector<T> add={}) :
      base_(base), cache_() {
      base_.insert(base_.begin(), add.begin(), add.end());
    }
    ~CachedLUT() {
      for (auto it = cache_.begin(); it != cache_.end(); it++)
        free(it->second);
    }
    template<typename T2>
    const std::vector<T2>* get(RAPIDJSON_ENABLEIF((internal::IsSame<T, T2>)))
    { return &base_; }
    template<typename T2>
    const std::vector<T2>* get(RAPIDJSON_DISABLEIF((internal::IsSame<T, T2>))) {
      std::type_index idx = std::type_index(typeid(typename T2::EncodingType));
      std::map<std::type_index, void*>::iterator match = cache_.find(idx);
      if (match == cache_.end()) {
        std::vector<T2>* new_entry = (std::vector<T2>*)malloc(sizeof(std::vector<T2>));
	RAPIDJSON_ASSERT(new_entry);
	new_entry[0] = std::vector<T2>();
        for (auto it = base_.begin(); it != base_.end(); it++)
          new_entry->push_back(it->template transcode<typename T2::EncodingType>());
	cache_.insert({idx, (void*)(new_entry)});
        return new_entry;
      } else {
	return (std::vector<T2>*)(match->second);
      }
    };
  private:
    std::vector<T> base_;
    std::map<std::type_index, void*> cache_;
  };

// Forward declarations
template<typename Encoding>
class Unit;
template<typename Encoding>
class Units;
template<typename T, typename Encoding>
class Quantity;
template<typename T, typename Encoding>
class QuantityArray;

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
      if (!(compare_values(powers_[i], x.powers_[i])))
	return false;
    return true;
  }
  bool operator!=(const Dimension& x) const { return (!(*this == x)); }
  bool is_irreducible() const {
    int ndim = 0;
    for (size_t i = 0; i < 8; i++)
      if (!(compare_values(powers_[i], 0.0)))
	ndim++;
    return (ndim == 1);
  }
  std::vector<Dimension> reduced() const {
    std::vector<Dimension> out;
    for (size_t i = 0; i < 8; i++)
      if (!(compare_values(powers_[i], 0.0)))
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

template<typename T1>
double char_to_double(const std::basic_string<T1>& x,
		      RAPIDJSON_ENABLEIF((internal::IsSame<T1,char>))) {
  return atof(x.c_str());
}
template<typename T1>
double char_to_double(const std::basic_string<T1>& x,
		      RAPIDJSON_ENABLEIF((internal::IsSame<T1,wchar_t>))) {
  return wcstod(x.c_str(), NULL);
}
template<typename SourceEncoding, typename DestEncoding>
const std::basic_string<typename DestEncoding::Ch> convert_chars(const std::basic_string<typename SourceEncoding::Ch>& x) {
  GenericStringStream<SourceEncoding> src(x.c_str());
  GenericStringBuffer<DestEncoding> dst;
  PutReserve(src, x.size());
  while (RAPIDJSON_LIKELY(src.Tell() < x.size())) {
    RAPIDJSON_ASSERT(src.Peek() != '\0');
    Transcoder<SourceEncoding, DestEncoding>::Transcode(src, dst);
  }
  std::basic_string<typename DestEncoding::Ch> out(dst.GetString(), dst.GetLength());
  return out;
};
								 
// template<typename T1, typename T2>
// const std::basic_string<T2> convert_chars(const std::basic_string<T1>& x,
// 					  RAPIDJSON_ENABLEIF((internal::IsSame<T1,T2>))) {
//   return x;
// }
  
//! Unit prefix.
template<typename Encoding>
class UnitPrefix {
public:
  typedef Encoding EncodingType;    //!< Encoding type from template parameter.
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
  UnitPrefix() : abbr(), factor(1.0), name() {}
  UnitPrefix(const Ch* abbr0, const double& factor0, const Ch* name0) :
    abbr(abbr0), factor(factor0), name(name0) {}
  UnitPrefix(const std::basic_string<Ch> abbr0, const double& factor0,
	     const std::basic_string<Ch> name0) :
    abbr(abbr0), factor(factor0), name(name0) {}
  //! Abbreviation associated with the prefix.
  std::basic_string<Ch> abbr;
  //! Factor that the prefix implies.
  double factor;
  //! Full name associated with the prefix.
  std::basic_string<Ch> name;

  //! \brief Determine if this unit prefix is identical to another.
  //! \param x Unit prefix for comparison.
  //! \return true if the unit prefixes are identical.
  bool operator==(const UnitPrefix& x) const {
    if (abbr != x.abbr) return false;
    return compare_values(factor, x.factor);
  }
  //! \brief Determine if this unit prefix is not identical to another.
  //! \param x Unit prefix for comparison.
  //! \return true if the unit prefixes are not identical.
  bool operator!=(const UnitPrefix& x) const { return (!(*this == x)); }

  //! \brief Create a copy of the prefix with a different encoding.
  //! \tparam DestEncoding Encoding that the copy should use.
  //! \return Copy w/ DestEncoding.
  template<typename DestEncoding>
  UnitPrefix<DestEncoding> transcode() const {
    return UnitPrefix<DestEncoding>(convert_chars<Encoding, DestEncoding>(abbr),
				    factor,
				    convert_chars<Encoding, DestEncoding>(name));
  }
  
  friend class Unit<Encoding>;
};

  
  // This dictionary formatting from magnitude package (secondarily via unyt), credit to Juan Reyero.
  static CachedLUT<UnitPrefix<UTF8<char>>> _unit_prefixes({
      UnitPrefix<UTF8<char>>("Y", 1e24, "yotta"),
      UnitPrefix<UTF8<char>>("Z", 1e21, "zetta"),
      UnitPrefix<UTF8<char>>("E", 1e18, "exa"),
      UnitPrefix<UTF8<char>>("P", 1e15, "peta"),
      UnitPrefix<UTF8<char>>("T", 1e12, "tera"),
      UnitPrefix<UTF8<char>>("G", 1e9, "giga"),
      UnitPrefix<UTF8<char>>("M", 1e6, "mega"),
      UnitPrefix<UTF8<char>>("k", 1e3, "kilo"),
      UnitPrefix<UTF8<char>>("h", 1e2, "hecto"),
      UnitPrefix<UTF8<char>>("da", 1e1, "deca"),
      UnitPrefix<UTF8<char>>("d", 1e-1, "deci"),
      UnitPrefix<UTF8<char>>("c", 1e-2, "centi"),
      UnitPrefix<UTF8<char>>("m", 1e-3, "mili"),
      UnitPrefix<UTF8<char>>(u8"\u00b5", 1e-6, "micro"),  // ('MICRO SIGN' U+00B5)
      UnitPrefix<UTF8<char>>("u", 1e-6, "micro"),
      UnitPrefix<UTF8<char>>(u8"\u03bc", 1e-6, "micro"),  // ('GREEK SMALL LETTER MU' U+03BC)
      UnitPrefix<UTF8<char>>("n", 1e-9, "nano"),
      UnitPrefix<UTF8<char>>("p", 1e-12, "pico"),
      UnitPrefix<UTF8<char>>("f", 1e-15, "femto"),
      UnitPrefix<UTF8<char>>("a", 1e-18, "atto"),
      UnitPrefix<UTF8<char>>("z", 1e-21, "zepto"),
      UnitPrefix<UTF8<char>>("y", 1e-24, "yocto")
    });

//! Unit.
template<typename Encoding>
class Unit {
public:
  typedef Encoding EncodingType;    //!< Encoding type from template parameter.
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
  //! \brief Empty constructor.
  Unit() : names_(), abbrs_(), dim_(), factor_(1.0), offset_(0.0), power_(1.0), prefix_() {}
  //! \brief Constructor from a look-up table.
  //! \param x Base unit.
  //! \param prefix Prefix that should be applied to the base unit.
  Unit(const Unit& x, const UnitPrefix<Encoding>& prefix) : Unit(x) {
    prefix_ = prefix;
    RAPIDJSON_ASSERT(!(has_power() && has_offset()));
  }
  //! \brief Construct from a single name/abbreviation.
  //! \param name Name.
  //! \param abbr Abbreviation.
  //! \param dim Dimensions.
  //! \param factor Scale factor from the base unit system.
  //! \param offset Offset from the zero point of the base unit system.
  //! \param power Power that will be applied to the unit during conversion.
  //! \param prefix Prefix that should be applied to the base unit.
  Unit(const Ch* name, const Ch* abbr, const Dimension dim,
       const double factor=1.0, const double offset=0.0,
       const double power=1.0,
       const UnitPrefix<Encoding>& prefix=UnitPrefix<Encoding>()) :
    names_(), abbrs_(), dim_(dim), factor_(factor), offset_(offset), power_(power), prefix_(prefix) {
    names_.push_back(std::basic_string<Ch>(name));
    abbrs_.push_back(std::basic_string<Ch>(abbr));
    _add_plural();
  }
  //! \brief Construct from a single name/abbreviation.
  //! \param names Names.
  //! \param abbrs Abbreviations.
  //! \param dim Dimensions.
  //! \param factor Scale factor from the base unit system.
  //! \param offset Offset from the zero point of the base unit system.
  //! \param power Power that will be applied to the unit during conversion.
  //! \param prefix Prefix that should be applied to the base unit.
  //! \param no_plural If true, the plural versions of the names will not be
  //!   added.
  Unit(const std::vector<std::basic_string<Ch>>& names,
       const std::vector<std::basic_string<Ch>>& abbrs,
       const Dimension dim, const double factor=1.0, const double offset=0.0,
       const double power=1.0,
       const UnitPrefix<Encoding>& prefix=UnitPrefix<Encoding>(),
       const bool& no_plural=false) :
    names_(names), abbrs_(abbrs), dim_(dim), factor_(factor), offset_(offset), power_(power), prefix_(prefix) { if (!no_plural) _add_plural(); }
  //! \brief Construct a unit by looking up a string in the tables of
  //!   recognized units.
  //! \param str Unit string.
  //! \param power Power that should be applied to the located unit.
  //! \param prefix Prefix that should be applied to the base unit.
  Unit(const std::basic_string<Ch> str, const double& power=1.0) : Unit() {
    RAPIDJSON_ASSERT(from_table(str));
    power_ = power; // Base units do not have powers
    RAPIDJSON_ASSERT(!(has_power() && has_offset()));
  }
  //! \brief Set instance attributes based on an entry from one of the lookup
  //!   tables.
  //! \param found Table entry.
  //! \param prefix Prefix that should be applied to the base unit.
  //! \return true if the unit could be initialized, false otherwise.
  bool from_table(const Unit<Encoding>& found,
		  const UnitPrefix<Encoding>& prefix=UnitPrefix<Encoding>()) {
    RAPIDJSON_ASSERT((found.names_.size() > 0) && (found.abbrs_.size() > 0));
    names_.insert(names_.begin(), found.names_.begin(), found.names_.end());
    abbrs_.insert(abbrs_.begin(), found.abbrs_.begin(), found.abbrs_.end());
    RAPIDJSON_ASSERT((names_.size() > 0) && (abbrs_.size() > 0));
    dim_ = found.dim_;
    factor_ = found.factor_;
    offset_ = found.offset_;
    prefix_ = prefix;
    return true;
  }
  //! \brief Write the unit to an output stream with class information.
  //! \param os Output stream.
  void display(std::ostream& os) const {
    RAPIDJSON_ASSERT(names_.size() > 0);
    os << "Unit(\"" << convert_chars<Encoding,UTF8<char>>(prefix_.name)
       << convert_chars<Encoding,UTF8<char>>(names_[0]) << "\", " << dim_ << ", "
       << factor_ << ", " << offset_ << ")**" << power_;
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
    if (!(compare_values(factor_, x.factor_))) return false;
    if (!(compare_values(offset_, x.offset_))) return false;
    if (!(compare_values(power_, x.power_))) return false;
    if (prefix_ != x.prefix_) return false;
    return true;
  }
  //! \brief Check if this unit is not equal to another.
  //! \param x Unit to check against.
  //! \return true if this unit is not equal to x.
  bool operator!=(const Unit& x) const { return (!(*this == x)); }
  //! \brief Create a copy of the unit with a different encoding.
  //! \tparam DestEncoding Encoding that the copy should use.
  //! \return Copy w/ DestEncoding.
  template<typename DestEncoding>
  Unit<DestEncoding> transcode() const {
    std::vector<std::basic_string<typename DestEncoding::Ch>> names, abbrs;
    for (auto it = names_.begin(); it != names_.end(); it++)
      names.push_back(convert_chars<Encoding, DestEncoding>(*it));
    for (auto it = abbrs_.begin(); it != abbrs_.end(); it++)
      abbrs.push_back(convert_chars<Encoding, DestEncoding>(*it));
    return Unit<DestEncoding>(names, abbrs, dim_, factor_, offset_, power_,
			      prefix_.template transcode<DestEncoding>());
  }
  //! \brief Perform power operation in place.
  //! \param x Power to raise this unit to.
  void inplace_pow(const double x) {
    RAPIDJSON_ASSERT(!(has_offset() && (!(compare_values(x, 1.0)))));
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
    RAPIDJSON_ASSERT(x.names_.size() > 0);
    RAPIDJSON_ASSERT(names_.size() > 0);
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
  bool matches(const std::basic_string<Ch> str,
	       const UnitPrefix<Encoding>& prefix) const {
    for (auto n = names_.begin(); n != names_.end(); n++) {
      if (str.compare(prefix.name + (*n)) == 0)
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
  bool has_offset() const { return (!(compare_values(offset_, 0.0))); }
  //! \brief Check if this unit has a power other than 1.
  //! \return true if this unit has a power other than 1.
  bool has_power() const { return (!(compare_values(power_, 1.0))); }
  //! \brief Check if this unit is irreducible or a product of more than
  //!   one irreducible unit.
  //! \return true if the unit is irreducible.
  bool is_irreducible() const { return dim_.is_irreducible(); }
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
    std::vector<double> out { std::pow(factor_ * prefix_.factor, power_),
      offset_ };
    return out;
  }
  //! \brief Find the unit that matches a string in one of the look up tables.
  //! \param str String to find a unit for.
  //! \return true if a unit could be located, false otherwise.
  bool from_table(const std::basic_string<Ch> str);
private:
  std::vector<std::basic_string<Ch>> names_;
  std::vector<std::basic_string<Ch>> abbrs_;
  Dimension dim_;
  double factor_;
  double offset_;
  double power_;
  UnitPrefix<Encoding> prefix_;

  void _add_plural() {
    size_t N = names_.size();
    std::basic_string<Ch> iname;
    for (size_t i = 0; i < N; i++) {
      iname = names_[i];
      iname.push_back('s');
      names_.push_back(iname);
    }
  }
  static const std::basic_string<Ch> get_whitespace() {
    static const Ch s[] = {' ', '\t', '\f', '\v', '\n', '\r', '\0'};
    return std::basic_string<Ch>(s);
  }
  
  friend class Units<Encoding>;
  template<typename Ch2>
  friend std::ostream & operator << (std::ostream& os, const Unit<Ch2> &x);
};
template<typename Encoding>
inline std::ostream & operator << (std::ostream& os, const Unit<Encoding> &x) {
  RAPIDJSON_ASSERT(x.abbrs_.size() > 0);
  os << convert_chars<Encoding,UTF8<char>>(x.prefix_.abbr)
     << convert_chars<Encoding,UTF8<char>>(x.abbrs_[0]);
  if (x.has_power()) os << "**" << x.power_;
  return os;
};

//! \brief Units class.
template<typename Encoding>
class Units {
public:
  typedef Encoding EncodingType;    //!< Encoding type from template parameter.
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
  //! \brief Empty constructor.
  Units() : units_() {}
  //! \brief Initialize from an initializer list.
  //! \param units Initializer list of units.
  Units(std::initializer_list<Unit<Encoding>> units) : units_(units) {}
  //! \brief Initialize from a vector of units.
  //! \param units Vector of units.
  Units(const std::vector<Unit<Encoding>> units) : units_(units) {}
  //! \brief Initialize from a string.
  //! \param str Units string.
  //! \param verbose If true, verbose information is displayed when
  //!   parsing the units string.
  Units(const std::basic_string<Ch> str, const bool& verbose=false) :
    Units(str.c_str(), str.length(), verbose) {}
  //! \brief Initialize from a string.
  //! \tparam N Number of characters in the string.
  //! \param str Units string.
  //! \param verbose If true, verbose information is displayed when
  //!   parsing the units string.
  template<size_t N>
  Units(const Ch str[N], const bool& verbose=false) :
    Units(str, N, verbose) {}
  //! \brief Initialize from a string.
  //! \param str Units string. The length is determined by assuming str is
  //!   null terminated.
  //! \param verbose If true, verbose information is displayed when
  //!   parsing the units string.
  Units(const Ch* str, const bool& verbose=false) :
    Units(str, internal::StrLen(str), verbose) {}
  //! \brief Initialize from a string.
  //! \param str Units string.
  //! \param len Number of characters in str.
  //! \param verbose If true, verbose information is displayed when
  //!   parsing the units string.
  Units(const Ch* str, const size_t len, const bool& verbose=false) : units_() {
    Units<Encoding> new_units = parse_units(str, len, verbose);
    units_.insert(units_.begin(), new_units.units_.begin(), new_units.units_.end());
  }
  //! \brief Parse a units string.
  //! \param str Units string.
  //! \param len Number of characters in str.
  //! \param verbose If true, verbose information is displayed when
  //!   parsing the units string.
  static Units parse_units(const Ch* str, const size_t len,
			   const bool verbose=false);  // Forward declaration
  //! \brief Add a unit to the unit set from a string.
  //! \param str Unit string.
  void add_unit(const std::basic_string<Ch> str) {
    units_.emplace_back(str);
  }
  //! \brief Display the units instance.
  //! \param os Output stream.
  void display(std::ostream& os) const {
    size_t i = 0;
    os << "Units([";
    for (auto it = units_.begin(); it != units_.end(); it++, i++) {
      if (i != 0) os << ",";
      it->display(os);
    }
    os << "])";
  }
  //! \brief Get the units as a string.
  //! \return Units string.
  std::basic_string<Ch> str() const {
    std::basic_stringstream<Ch> ss;
    ss << *this;
    return ss.str();
  }
  //! \brief Get the dimensions of the units.
  //! \returns Consolidated dimensions of the units.
  Dimension dimension() const {
    Dimension out(dimensions::dimensionless);
    for (auto it = units_.begin(); it != units_.end(); it++)
      out = out * it->dimension();
    return out;
  }
  //! \brief Determine if another set of units are compatible and share the
  //!   same dimensions.
  //! \param x Units for comparison.
  //! \return true if the units are compatible, false otherwise.
  bool is_compatible(const Units& x) const {
    return (dimension() == x.dimension());
  }
  //! \brief Determine if this set of units is identical to another.
  //! \param x Units for comparison.
  //! \return true if the units are identical.
  bool operator==(const Units& x) const {
    if (units_.size() != x.units_.size())
      return false;
    for (size_t i = 0; i < units_.size(); i++)
      if (units_[i] != x.units_[i])
	return false;
    return true;
  }
  //! \brief Determine if this set of units is not identical to another.
  //! \param x Units for comparison.
  //! \return true if the units are not identical.
  bool operator!=(const Units& x) const { return (!(*this == x)); }
  //! \brief Perform multiplication with another set of units.
  //! \param x Units for multiplication.
  //! \return Multiplied units.
  Units operator*(const Units& x) const {
    RAPIDJSON_ASSERT(!(x.has_offset() || has_offset()));
    std::vector<Unit<Encoding>> new_units(units_);
    for (auto it2 = x.units_.begin(); it2 != x.units_.end(); it2++) {
      size_t i = 0;
      for (i = 0; i < units_.size(); i++)
	if (it2->is_same_base(new_units[i]))
	  break;
      if (i < units_.size()) {
	// (a1*ap*x)**a2 * (b1*bp*x)**b2
	//     = (a1**a2)*(b1**b2)*(ap**a2)*(bp**b2)*(x**(a2+b2))
	//     = (a1**a2)*(b1**b2)*(ap**-b2)*(bp**b2)*((ap*x)**(a2+b2))
	double new_power = new_units[i].power_ + it2->power_;
	new_units[i].factor_ = std::pow(std::pow(new_units[i].factor_,
						 new_units[i].power_) *
					std::pow(it2->factor_ * it2->prefix_.factor / new_units[i].prefix_.factor,
						 it2->power_),
					1.0 / new_power);
	new_units[i].power_ = new_power;
      } else {
	new_units.emplace_back(*it2);
      }
    }
    return Units(new_units);
  }
  //! \brief Perform division with another set of units.
  //! \param x Units for division.
  //! \return Divided units.
  Units operator/(const Units& x) const {
    return (*this) * (x.pow(-1)); }
  //! \brief Raise these units to a power without creating a new instance.
  //! \param x Power.
  void inplace_pow(const double x) {
    for (auto it = units_.begin(); it != units_.end(); it++)
      it->inplace_pow(x);
  }
  //! \brief Raise these units to a power.
  //! \param x Power.
  //! \return Resulting units.
  Units pow(const double x) const {
    Units out(*this);
    out.inplace_pow(x);
    return out;
  }
  //! \brief Check if there are any units in the instance.
  //! \return true if there arn't any units, false otherwise.
  bool is_empty() const { return (units_.size() == 0); }
  //! \brief Check if the units have an offset.
  //! \return true if the units have an offset, false otherwise.
  bool has_offset() const {
    for (auto it = units_.begin(); it != units_.end(); it++)
      if (it->has_offset())
	return true;
    return false;
  }
  //! \brief Determine the conversion factors necessary to convert quantities
  //!   with these units to another set of units.
  //! \param x Units that conversion factors should convert to.
  //! \return Array of conversion factors where the first element is the
  //!   factor that values should be multiplied by and the second element is
  //!   the offset between the zero points in this and x.
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
  std::vector<Unit<Encoding>> units_;
  template<typename Enc2>
  friend std::ostream & operator << (std::ostream &os, const Units<Enc2> &x);
};
template<typename Encoding>
inline std::ostream & operator << (std::ostream &os, const Units<Encoding> &x) {
    size_t i = 0;
    for (auto it = x.units_.begin(); it != x.units_.end(); it++, i++) {
      if (i != 0) os << "*";
      os << "(" << *it << ")";
    }
    return os;
};

template<typename Encoding>
Units<Encoding> operator*(const Unit<Encoding>& a, const Unit<Encoding>& b) {
  RAPIDJSON_ASSERT(!(a.has_offset() || b.has_offset()));
  return Units<Encoding>({a, b}); }
template<typename Encoding>
Units<Encoding> operator/(const Unit<Encoding>& a, const Unit<Encoding>& b) {
  return a * b.pow(-1); }


  // MKS as base, units that can have SI prefixes
  static CachedLUT<Unit<UTF8<char>>> _base_units ({
      Unit<UTF8<char>>({"meter", "metre"}, {"m"}, dimensions::length),
      Unit<UTF8<char>>({"gram", "gramme"}, {"g"}, dimensions::mass, 1.0e-3),
      Unit<UTF8<char>>("second", "s", dimensions::time),
      Unit<UTF8<char>>({"ampere", "amp", "Amp"}, {"A"}, dimensions::current),
      Unit<UTF8<char>>({"kelvin", "degree_kelvin"}, {"K", "degK"}, dimensions::temperature),
      Unit<UTF8<char>>("mole", "mol", dimensions::number, 1.0 / constants::amu_grams),
      Unit<UTF8<char>>("candela", "cd", dimensions::luminous_intensity),
      Unit<UTF8<char>>("radian", "rad", dimensions::angle),
    });
  
  static CachedLUT<Unit<UTF8<char>>> _prefixable_units (_base_units.template get<Unit<UTF8<char>>>()[0], {
      // cgs
      Unit<UTF8<char>>("dyne", "dyn", dimensions::force, 1.0e-5),
      Unit<UTF8<char>>("erg", "erg", dimensions::energy, 1.0e-7),
      Unit<UTF8<char>>("barye", "Ba", dimensions::pressure, 0.1),
      Unit<UTF8<char>>("gauss", "G", dimensions::magnetic_field_cgs, pow(0.1, 0.5)),
      Unit<UTF8<char>>({"statcoulomb", "esu", "ESU", "electrostatic_unit"}, {"statC"}, dimensions::charge_cgs, pow(1.0e-3, 1.5)),
      Unit<UTF8<char>>("statampere", "statA", dimensions::current_cgs, pow(1.0e-3, 1.5)),
      Unit<UTF8<char>>("statvolt", "statV", dimensions::electric_potential_cgs, 0.1 * pow(1.0e-3, 1.5)),
      Unit<UTF8<char>>("statohm", "statohm", dimensions::resistance_cgs, 100.0),
      Unit<UTF8<char>>("maxwell", "Mx", dimensions::magnetic_flux_cgs, pow(1.0e-3, 1.5)),
      // SI
      Unit<UTF8<char>>("joule", "J", dimensions::energy),
      Unit<UTF8<char>>("watt", "W", dimensions::power),
      Unit<UTF8<char>>("hertz", "Hz", dimensions::rate),
      Unit<UTF8<char>>("newton", "N", dimensions::force),
      Unit<UTF8<char>>("coulomb", "C", dimensions::charge),
      Unit<UTF8<char>>("tesla", "T", dimensions::magnetic_field),
      Unit<UTF8<char>>("pascal", "Pa", dimensions::pressure),
      Unit<UTF8<char>>("bar", "bar", dimensions::pressure, 1.0e5),
      Unit<UTF8<char>>("volt", "V", dimensions::electric_potential),
      Unit<UTF8<char>>("farad", "F", dimensions::capacitance),
      Unit<UTF8<char>>("henry", "H", dimensions::inductance),
      Unit<UTF8<char>>({"ohm", "Ohm"}, {"Ω"}, dimensions::resistance),
      Unit<UTF8<char>>("weber", "Wb", dimensions::magnetic_flux),
      Unit<UTF8<char>>("lumen", "lm", dimensions::luminous_flux),
      Unit<UTF8<char>>("lux", "lx", dimensions::luminous_flux / dimensions::area),
      Unit<UTF8<char>>({"celcius", "degree_celsius", "degree_Celsius", "celsius"}, {"degC", "°C"}, dimensions::temperature, 1.0, constants::celcius_zero_kelvin),
      // other
      Unit<UTF8<char>>("calorie", "cal", dimensions::energy, 4.184),
      Unit<UTF8<char>>("year", "yr", dimensions::time, constants::sec_per_year),
      Unit<UTF8<char>>("parsec", "pc", dimensions::length, constants::m_per_pc),
      Unit<UTF8<char>>("electronvolt", "eV", dimensions::energy, constants::J_per_eV),
      Unit<UTF8<char>>("jansky", "J", dimensions::specific_flux, constants::jansky_mks),
      Unit<UTF8<char>>("sievert", "Sv", dimensions::specific_energy),
      Unit<UTF8<char>>("molar", "M", dimensions::number_density, 100.0 / constants::amu_grams),
    });
  
  static CachedLUT<Unit<UTF8<char>>> _unprefixable_units({
    // Imperial units
    Unit<UTF8<char>>({"mil", "thou", "thousandth"}, {"mil"}, dimensions::length, 1.0e-3 * constants::m_per_inch),
    Unit<UTF8<char>>("incl", "incl", dimensions::length, constants::m_per_inch),
    Unit<UTF8<char>>("feet", "ft", dimensions::length, constants::m_per_ft),
    Unit<UTF8<char>>("yard", "yd", dimensions::length, 0.9144),
    Unit<UTF8<char>>("mile", "mi", dimensions::length, 1609.344),
    Unit<UTF8<char>>("furlong", "fur", dimensions::length, constants::m_per_ft * 660.0),
    Unit<UTF8<char>>({"farenheit", "degree_fahrenheit", "degree_Fahrenheit"}, {"degF", "°F"}, dimensions::temperature, constants::kelvin_per_rankine, constants::farenheit_zero_kelvin),
    Unit<UTF8<char>>({"rankine", "degree_rankine"}, {"degR"}, dimensions::temperature, constants::kelvin_per_rankine),
    Unit<UTF8<char>>("pound_force", "lbf", dimensions::force, constants::kg_per_pound * constants::standard_gravity_m_per_s2),
    Unit<UTF8<char>>({"pound", "pound_mass"}, {"lb", "lbm"}, dimensions::mass, constants::kg_per_pound),
    Unit<UTF8<char>>("atmosphere", "atm", dimensions::pressure, constants::pascal_per_atm),
    Unit<UTF8<char>>("horsepower", "hp", dimensions::power, constants::watt_per_horsepower),
    Unit<UTF8<char>>("ounce", "oz", dimensions::mass, constants::kg_per_pound / 16.0),
    Unit<UTF8<char>>("ton", "ton", dimensions::mass, constants::kg_per_pound * 2000.0),
    Unit<UTF8<char>>("slug", "slug", dimensions::mass, constants::kg_per_pound * constants::standard_gravity_m_per_s2 / constants::m_per_ft),
    Unit<UTF8<char>>({"BTU", "british_thermal_unit"}, {"BTU"}, dimensions::energy, 1055.0559),
    Unit<UTF8<char>>("pounds_per_square_inch", "psi", dimensions::pressure, constants::kg_per_pound * constants::standard_gravity_m_per_s2 / pow(constants::m_per_inch, 2)),
    Unit<UTF8<char>>("smoot", "smoot", dimensions::length, 1.7018),
    Unit<UTF8<char>>("percent", "%", dimensions::dimensionless, 0.01),
    Unit<UTF8<char>>("minute", "min", dimensions::time, constants::sec_per_min),
    Unit<UTF8<char>>("hour", "hr", dimensions::time, constants::sec_per_hr),
    Unit<UTF8<char>>("day", "day", dimensions::time, constants::sec_per_day),
    // Astronomy units
    Unit<UTF8<char>>("c", "c", dimensions::velocity, constants::speed_of_light_m_per_s),
    Unit<UTF8<char>>({"solar_mass", "solMass", "mass_sun"}, {"Msun", "Msol", "msun", "m_sun", "M_sun", "m_Sun"}, dimensions::mass, constants::mass_sun_kg),
    Unit<UTF8<char>>({"solar_radius", "solRadius"}, {"Rsun", "Rsol", "rsun", "r_sun", "R_sun", "r_Sun"}, dimensions::length, constants::m_per_rsun),
    Unit<UTF8<char>>({"solar_luminosity", "solLumin"}, {"Lsun", "Lsol", "lsun", "l_sun", "L_sun", "l_Sun"}, dimensions::power, constants::luminosity_sun_watts),
    Unit<UTF8<char>>({"solar_temperature", "solTemperature"}, {"Tsun", "Tsol", "tsun", "t_sun", "T_sun", "t_Sun"}, dimensions::temperature, constants::temp_sun_kelvin),
    Unit<UTF8<char>>({"solar_metallicity", "solMetallicity"}, {"Zsun", "Zsol", "zsun", "z_sun", "Z_sun", "z_Sun"}, dimensions::dimensionless, constants::metallicity_sun),
    Unit<UTF8<char>>({"jupiter_mass"}, {"Mjup", "m_jup"}, dimensions::mass, constants::mass_jupiter_kg),
    Unit<UTF8<char>>({"jupiter_radius"}, {"Rjup", "r_jup"}, dimensions::length, constants::m_per_rjup),
    Unit<UTF8<char>>({"earth_mass"}, {"Mearth", "m_earth"}, dimensions::mass, constants::mass_earth_kg),
    Unit<UTF8<char>>({"earth_radius"}, {"Rearth", "r_earth"}, dimensions::length, constants::m_per_rearth),
    Unit<UTF8<char>>({"astronomical_unit"}, {"AU", "au"}, dimensions::length, constants::m_per_au),
    Unit<UTF8<char>>("light_year", "ly", dimensions::length, constants::m_per_ly),
    Unit<UTF8<char>>("degree", "deg", dimensions::angle, M_PI / 180.0),
    Unit<UTF8<char>>("arcmin", "arcmin", dimensions::angle, M_PI / 10800.0),
    Unit<UTF8<char>>("arcsec", "arcsec", dimensions::angle, M_PI / 648000.0),
    Unit<UTF8<char>>("miliarcsec", "mas", dimensions::angle, M_PI / 648000000.0),
    Unit<UTF8<char>>("hourangle", "hourangle", dimensions::angle, M_PI / 12.0),
    Unit<UTF8<char>>("steradian", "sr", dimensions::solid_angle),
    Unit<UTF8<char>>({"latitude", "degree_latitude"}, {"lat"}, dimensions::angle, -M_PI / 180.0, 90.0),
    Unit<UTF8<char>>({"longitude", "degree_longitude"}, {"long"}, dimensions::angle, M_PI / 180.0, -180.0),
    // Physics
    Unit<UTF8<char>>("amu", "amu", dimensions::mass, constants::amu_kg),
    Unit<UTF8<char>>("angstrom", "Å", dimensions::length, constants::m_per_ang),
    Unit<UTF8<char>>("counts", "counts", dimensions::number),
    Unit<UTF8<char>>("photons", "photons", dimensions::number),
    Unit<UTF8<char>>("me", "me", dimensions::mass, constants::mass_electron_kg),
    Unit<UTF8<char>>("mp", "mp", dimensions::mass, constants::mass_hydrogen_kg),
    Unit<UTF8<char>>("rayleigh", "rayleigh", dimensions::count_intensity, 2.5e9 / M_PI),
    Unit<UTF8<char>>("lambert", "lambert", dimensions::luminance, 1.0e4 / M_PI),
    Unit<UTF8<char>>("nit", "nt", dimensions::luminance),
    // Planck
    Unit<UTF8<char>>("m_pl", "m_pl", dimensions::mass, constants::planck_mass_kg),
    Unit<UTF8<char>>("l_pl", "l_pl", dimensions::length, constants::planck_length_m),
    Unit<UTF8<char>>("t_pl", "t_pl", dimensions::time, constants::planck_time_s),
    Unit<UTF8<char>>("T_pl", "T_pl", dimensions::temperature, constants::planck_temperature_K),
    Unit<UTF8<char>>("q_pl", "q_pl", dimensions::charge, constants::planck_charge_C),
    Unit<UTF8<char>>("E_pl", "E_pl", dimensions::energy, constants::planck_energy_J),
    // Geometrized
    Unit<UTF8<char>>("m_geom", "m_geom", dimensions::mass, constants::mass_sun_kg),
    Unit<UTF8<char>>("l_geom", "l_geom", dimensions::length, constants::newton_mks * constants::mass_sun_kg / pow(constants::speed_of_light_m_per_s, 2)),
    Unit<UTF8<char>>("t_geom", "t_geom", dimensions::time, constants::newton_mks * constants::mass_sun_kg / pow(constants::speed_of_light_m_per_s, 3)),
    // Logarithmic
    // Unit<UTF8<char>>("bel", "B", dimensions::logarithmic, neper_per_bel),
    // Unit<UTF8<char>>("neper", "Np", dimensions::logarithmic),
    // misc
    Unit<UTF8<char>>("acre", "ac", dimensions::area, 4046.86),
    Unit<UTF8<char>>("are", "a", dimensions::area, 100.0),
    Unit<UTF8<char>>("hectare", "ha", dimensions::area, 10000.0),
  });

template<typename Encoding>
bool Unit<Encoding>::from_table(const std::basic_string<typename Encoding::Ch> str) {
  size_t idx_beg, idx_end;
  std::basic_string<Ch> whitespace = get_whitespace();
  idx_beg = str.find_first_not_of(whitespace);
  idx_end = str.find_last_not_of(whitespace);
  RAPIDJSON_ASSERT(idx_end != std::string::npos);
  std::basic_string<Ch> substr = str.substr(idx_beg, idx_end + 1);
  std::vector<const Unit<Encoding>*> possibilities;
  const std::vector<Unit<Encoding>>* prefix_units = _prefixable_units.template get<Unit<Encoding>>();
  for (auto it = prefix_units->begin(); it != prefix_units->end(); it++) {
    if (it->matches(substr))
      return from_table(*it);
    it->prefix_matches(substr, possibilities);
  }
  const std::vector<Unit<Encoding>>* unprefix_units = _unprefixable_units.template get<Unit<Encoding>>();
  for (auto it = unprefix_units->begin(); it != unprefix_units->end(); it++)
    if (it->matches(substr))
      return from_table(*it);
  if (possibilities.size() > 0) {
    const std::vector<UnitPrefix<Encoding>>* prefixes = _unit_prefixes.template get<UnitPrefix<Encoding>>();
    for (auto it = possibilities.begin(); it != possibilities.end(); it++)
      for (auto p = prefixes->begin(); p != prefixes->end(); p++)
	if ((*it)->matches(substr, *p))
	  return from_table(**it, *p);
  }
  std::cerr << "No match found for \"" << convert_chars<Encoding,UTF8<char>>(substr) << "\"" << std::endl; // GCOVR_EXCL_LINE
  RAPIDJSON_ASSERT(!sizeof("No match found")); // GCOVR_EXCL_LINE
  return false; // GCOVR_EXCL_LINE
};

namespace parser {

enum TokenType {
  kWhitespaceToken = 0,
  kOperatorToken = 1,
  kWordToken = 2,
  kGroupToken = 3
};

template<typename Encoding>
class GroupToken; // Forward declaration


template<typename Encoding>
class TokenBase {
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
private:
  TokenBase(const TokenBase<Encoding>& rhs);
public:
  TokenBase(const TokenType t0, TokenBase *parent0=nullptr) : t(t0), units(), finalized(false), parent(parent0), value_(0.0) {}
  virtual ~TokenBase() {}
  virtual TokenBase<Encoding>* current_token() { return this; }
  virtual Units<Encoding> finalize() {
    finalized = true;
    return units;
  }
  double value() {
    RAPIDJSON_ASSERT(is_numeric());
    finalize();
    return value_;
  }
  virtual bool is_numeric() { return false; }
  virtual void append(const Ch) = 0;
  virtual std::ostream & display(std::ostream &os) const = 0;
  TokenBase<Encoding>& operator=(const TokenBase<Encoding>& other);
  TokenType t;
  Units<Encoding> units;
  bool finalized;
  TokenBase<Encoding> *parent;
  double value_;
  template<typename Encoding2>
  friend std::ostream & operator << (std::ostream &os, const TokenBase<Encoding2>* x);
};
template<typename Encoding2>
inline std::ostream & operator << (std::ostream &os, const TokenBase<Encoding2>* x) {
  return x->display(os);
};
  
template<typename Encoding>
class OperatorToken : public TokenBase<Encoding> {
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
public:
  OperatorToken(const Ch op0, TokenBase<Encoding> *parent0=nullptr) : TokenBase<Encoding>(kOperatorToken, parent0), op(op0) { this->finalize(); }
  void append(const Ch c) override { RAPIDJSON_ASSERT(!c); } // GCOVR_EXCL_LINE
  Units<Encoding> operate(const Units<Encoding>& a, const Units<Encoding>& b) {
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
  Units<Encoding> operate(const Units<Encoding>& a, const double& b) {
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

template<typename Encoding>
class NumberToken; // Forward declaration
  
template<typename Encoding>
class WordToken : public TokenBase<Encoding> {
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
public:
  WordToken(const Ch c, TokenBase<Encoding> *parent0=nullptr) : TokenBase<Encoding>(kWordToken, parent0), word() {
    word.push_back(c);
  }
  void append(const Ch c) override {
    word.push_back(c);
  }
  Units<Encoding> finalize() override {
    RAPIDJSON_ASSERT(word.size());
    if (!(this->finalized))
      this->units.add_unit(word);
      // this->units = Units<Encoding>({Unit<Encoding>(word)});
    return TokenBase<Encoding>::finalize();
  }
  std::ostream & display(std::ostream &os) const override {
    os << "WordToken(" << convert_chars<Encoding,UTF8<char>>(word) << ")";
    return os;
  }
  std::basic_string<Ch> word;
  friend NumberToken<Encoding>;
};

template<typename Encoding>
class NumberToken : public WordToken<Encoding> {
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
public:
  NumberToken(const Ch c, TokenBase<Encoding> *parent0=nullptr) : WordToken<Encoding>(c, parent0) {}
  bool is_numeric() override { return true; }
  Units<Encoding> finalize() override {
    if (!(this->finalized))
      this->value_ = char_to_double<Ch>(this->word);
    return TokenBase<Encoding>::finalize();
  }
  std::ostream & display(std::ostream &os) const override {
    os << "NumericToken(" << convert_chars<Encoding,UTF8<char>>(this->word) << ")";
    return os;
  }
};

template<typename Encoding>
class GroupToken : public TokenBase<Encoding> {
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
public:
  GroupToken(TokenBase<Encoding> *parent0=nullptr) : TokenBase<Encoding>(kGroupToken, parent0), tokens() {}
  ~GroupToken() override {
    for (size_t i = 0; i < tokens.size(); i++)
      delete tokens[i];
    tokens.clear();
  }
  TokenBase<Encoding>* current_token() override {
    if (tokens.size() == 0)
      return TokenBase<Encoding>::current_token();
    return tokens[tokens.size() - 1]->current_token();
  }
  GroupToken<Encoding>* current_group() {
    int idx = (int)(tokens.size()) - 1;
    if ((idx >= 0) && (tokens[(size_t)idx]->t == kGroupToken)
	&& (!(tokens[(size_t)idx]->finalized))) {
      GroupToken<Encoding>* grp = static_cast<GroupToken<Encoding>*>(tokens[(size_t)idx]);
      return grp->current_group();
    }
    return this;
  }
  OperatorToken<Encoding>* append_op(const Ch c, bool dont_descend=false) {
    OperatorToken<Encoding>* op = new OperatorToken<Encoding>(c, this);
    append(op, dont_descend);
    return op;
  }
  void append(const Ch c) override {
    TokenBase<Encoding>* curr = current_token();
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
	curr = append(new NumberToken<Encoding>(c, (TokenBase<Encoding>*)(this)));
	break;
      default:
	curr = append(new WordToken<Encoding>(c, (TokenBase<Encoding>*)(this)));
      }
      return;
    } else if ((c == '-') || (c == '+')) {
      append_op(c);
      return;
    }
    RAPIDJSON_ASSERT(curr->t == kWordToken);
    WordToken<Encoding>* word = static_cast<WordToken<Encoding>*>(curr);
    word->append(c);
  }
  TokenBase<Encoding>* append(TokenBase<Encoding>* x, bool dont_descend=false) {
    GroupToken<Encoding>* curr;
    if (dont_descend)
      curr = this;
    else {
      curr = current_group();
      if (curr->tokens.size() > 0) {
	TokenBase<Encoding>* prev = curr->current_token();
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
	OperatorToken<Encoding> *op = static_cast<OperatorToken<Encoding>*>(tokens[i]);
	if (op->matches(ops)) {
	  RAPIDJSON_ASSERT((i + 1) < tokens.size());
	  GroupToken<Encoding>* new_group = new GroupToken<Encoding>(this);
	  for (size_t ii = i - 1; ii <= (i + 1); ii++) {
	    tokens[ii]->parent = new_group;
	    new_group->append(tokens[ii]);
	    tokens[ii] = nullptr;
	  }
	  tokens[i + 1] = (TokenBase<Encoding>*)(new_group);
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
  Units<Encoding> finalize() override {
    if ((tokens.size() == 0) || (this->finalized))
      return this->units;
    // Group operators first in order of operations
    if (tokens.size() > 3) {
      group_operators('^');
      group_operators({'*', '/'});
      group_operators({'+', '-'});
    }
    // Complete operations from left to right
    Units<Encoding> out = tokens[0]->finalize();
    if (is_numeric()) {
      this->value_ = tokens[0]->value();
      for (size_t i = 1; i < tokens.size(); i = i+2) {
	RAPIDJSON_ASSERT(tokens[i]->t == kOperatorToken);
	RAPIDJSON_ASSERT(tokens[i + 1]->t != kOperatorToken);
	OperatorToken<Encoding> *op = static_cast<OperatorToken<Encoding>*>(tokens[i]);
	this->value_ = op->operate(this->value_, tokens[i + 1]->value());
      }
    } else {
      for (size_t i = 1; i < tokens.size(); i = i+2) {
	RAPIDJSON_ASSERT(tokens[i]->t == kOperatorToken);
	RAPIDJSON_ASSERT(tokens[i + 1]->t != kOperatorToken);
	OperatorToken<Encoding> *op = static_cast<OperatorToken<Encoding>*>(tokens[i]);
	if (tokens[i + 1]->is_numeric()) {
	  out = op->operate(out, tokens[i + 1]->value());
	} else {
	  out = op->operate(out, tokens[i + 1]->finalize());
	}
      }
    }
    this->units = out;
    return TokenBase<Encoding>::finalize();
  }
  bool is_numeric() override {
    for (auto it = tokens.begin(); it != tokens.end(); it++) {
      if (!((*it)->is_numeric()))
	return false;
    }
    return true;
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
  std::vector<TokenBase<Encoding>*> tokens;
};

} // namespace parser


template<typename Encoding>
Units<Encoding> Units<Encoding>::parse_units(const typename Encoding::Ch* str,
					     const size_t len,
					     const bool verbose) {
  if (verbose)
    std::cout << "parse_units(\"" << convert_chars<Encoding,UTF8<char>>(str) << "\")" << std::endl;
  size_t i = 0;
  parser::GroupToken<Encoding> token;
  typename Encoding::Ch c;
  for (i = 0; i < len; i++) {
    c = str[i];
    switch (c) {
    case '(':
    case '[': // TODO: Concentration?
    case '{': {
      token.append(new parser::GroupToken<Encoding>());
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
      parser::TokenBase<Encoding>* word = token.current_token();
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

//! Scalar quantity with units.
//! \tparam T Type of the underlying scalar.
//! \tparam Encoding Encoding used to store the unit strings.
template<typename T, typename Encoding>
class Quantity {
public:
  typedef Encoding EncodingType;    //!< Encoding type from template parameter.
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
  //! \brief Empty constructor.
  Quantity() : value_(_initialize_value<T>()), units_() {}
  //! \brief Create a quantity without units.
  //! \param value Scalar value.
  Quantity(const T& value) :
    value_(value), units_() {}
  //! \brief Constructor from units string.
  //! \param value Scalar value.
  //! \param units Units string.
  Quantity(const T& value, const Ch* units) :
    value_(value), units_(units) {}
  //! \brief Constructor from units string.
  //! \param value Scalar value.
  //! \param units Units instance.
  Quantity(const T& value, const Units<Encoding>& units) :
    value_(value), units_(units) {}
  //! \brief Print instance information to an output stream.
  //! \param os Output stream.
  void display(std::ostream& os) const {
    os << "Quantity(" << value_ << ", ";
    units_.display(os);
    os << ")";
  }
private:
  Quantity raw_add(const Quantity& x, double factor=1.0) const {
    // Assumes units have already been matched
    return Quantity(value_ + (factor * x.value_), units_); }
  template<typename T1>
  static T1 _initialize_value(RAPIDJSON_DISABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T1))))
  { return (T1)(0); }
  template<typename T1>
  static T1 _initialize_value(RAPIDJSON_ENABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T1))))
  { return T1(0.0, 0.0); }
public:
  //! \brief Get the quantity value without units.
  //! \return Value.
  T value() const { return value_; };
  //! \brief Get the units instance.
  //! \return Units.
  Units<Encoding> units() const { return units_; }
  //! \brief Get the units string.
  //! \return Units string.
  std::basic_string<Ch> unitsStr() const { return units_.str(); }
  //! \brief Check if two quantities are identical. The units must be
  //!   identical, not just compatible.
  //! \param x Quantity for comparison.
  //! \return true if the two quantities are identical, false otherwise.
  bool operator==(const Quantity& x) const {
    if (units_ != x.units_)
      return false;
    return compare_values(value_, x.value_);
  }
  //! \brief Check if two quantities are not identical.
  //! \param x Quantity for comparison.
  //! \return true if the two quantities are not identical, false otherwise.
  bool operator!=(const Quantity& x) const { return (!(*this==x)); }
  //! \brief Multiply by another quantity.
  //! \param x Quantity to multiply by.
  //! \return Result of multiplication.
  Quantity operator*(const Quantity& x) const {
    return Quantity(value_ * x.value_, units_ * x.units_); }
  //! \brief Divide by another quantity.
  //! \param x Quantity to divide by.
  //! \return Result of division.
  Quantity operator/(const Quantity& x) const {
    return Quantity(value_ / x.value_, units_ / x.units_); }
  //! \brief Multiply by a scalar.
  //! \tparam T2 Scalar type.
  //! \param x Scalar to multiply by.
  //! \return Result of multiplication.
  template<typename T2>
  Quantity operator*(const T2& x) const {
    return Quantity(value_ * x, units_); }
  //! \brief Divide by a scalar.
  //! \tparam T2 Scalar type.
  //! \param x Scalar to divide by.
  //! \return Result of division.
  template<typename T2>
  Quantity operator/(const T2& x) const {
    return Quantity(value_ / x, units_); }
  //! \brief Add a quantity with compatible units.
  //! \param x Quantity to add.
  //! \return Result of addition.
  Quantity operator+(const Quantity& x) const {
    if (units_ != x.units_)
      return (*this + x.as(units_));
    return raw_add(x); }
  //! \brief Subtract a quantity with compatible units.
  //! \param x Quantity to subtract.
  //! \return Result of subtraction.
  Quantity operator-(const Quantity& x) const {
    if (units_ != x.units_)
      return (*this - x.as(units_));
    return raw_add(x, -1.0); }
  //! \brief Perform power operation in place.
  //! \param x Power to raise this quantity to.
  void inplace_pow(const double& x) {
    value_ = std::pow(value_, x);
    units_.inplace_pow(x);
  }
  //! \brief Raise this quantity to a power.
  //! \param x Power to raise this quantity to.
  //! \return Resulting quantity.
  Quantity pow(const double& x) const {
    Quantity out(*this);
    out.inplace_pow(x);
    return out;
  }
  //! \brief Get the dimensions of this quantity's units.
  //! \return The dimensions of the units.
  Dimension dimension() const { return units_.dimension(); }
  //! \brief Check if another quantity has compatible units with the same
  //!   dimensions.
  //! \param x Quantity for comparison.
  //! \return true if the units are compatible, false otherwise.
  bool is_compatible(const Quantity& x) const {
    return (dimension() == x.dimension());
  }
  //! \brief Check if another quantity is equivalent to this one, allowing
  //!    for the possibility that it has different, but compatible, units.
  //! \param x Quantity for comparison.
  //! \return true if the two quantities are equivalent, false otherwise.
  bool equivalent_to(const Quantity& x) {
    if (!(is_compatible(x)))
      return false;
    return (*this==x.as(units_));
  }
  //! \brief Convert the quantity to a different set of units. The new units
  //!   must be compatible with the current ones.
  //! \param units New units.
  void convert_to(const Units<Encoding>& units) {
    std::vector<double> factor = units_.conversion_factor(units);
    value_ = do_conv<T>(value_, factor[0], factor[1]);
    units_ = Units<Encoding>(units);
  }
  //! \brief Create a new quantity by converting this one to a new set of
  //!   compatible units.
  //! \param units New units.
  //! \return New quantity.
  Quantity as(const char* units0) const {
    Units<Encoding> units(units0);
    return as(units);
  }
  //! \brief Create a new quantity by converting this one to a new set of
  //!   compatible units.
  //! \param units New units.
  //! \return New quantity.
  Quantity as(const Units<Encoding>& units) const {
    Quantity out(*this);
    out.convert_to(units);
    return out;
  }
private:
  template<typename T2>
  static T2 do_conv(const T2& value, const double& factor, const double& offset,
		    RAPIDJSON_DISABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T2)))) {
    return static_cast<T2>((static_cast<double>(value) - offset) * factor);
  }
  template<typename T2>
  static T2 do_conv(const T2& value, const double& factor, const double& offset,
		    RAPIDJSON_ENABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T2)))) {
    T2 offset2(static_cast<typename T2::value_type>(offset), 0);
    typename T2::value_type factor2 = static_cast<typename T2::value_type>(factor);
    return (value - offset2) * factor2;
  }

  T value_;
  Units<Encoding> units_;
  template<typename U, typename Encoding2>
  friend std::ostream & operator << (std::ostream &os, const Quantity<U,Encoding2> &x);
  friend class QuantityArray<T,Encoding>;
};
template<typename T, typename Encoding>
inline std::ostream & operator << (std::ostream &os, const Quantity<T, Encoding> &x) {
    os << x.value_ << " " << x.units_;
    return os;
};

#define ARRAY_ARRAY_OP(op)						\
  QuantityArray operator op(const QuantityArray& x) const {		\
    RAPIDJSON_ASSERT(is_same_shape(x));					\
    SizeType N = nelements();						\
    QuantityArray out(value_, ndim_, shape_, units_ op x.units_);	\
    for (SizeType i = 0; i < N; i++)					\
      out.value_[i] = value_[i] op x.value_[i];				\
    return out;								\
  }
#define ARRAY_SCALAR_OP(op)						\
  template<typename T2>							\
  QuantityArray operator op(const T2& x) const {			\
    SizeType N = nelements();						\
    QuantityArray out(value_, ndim_, shape_, units_ op x);		\
    for (SizeType i = 0; i < N; i++)					\
      out.value_[i] = value_[i] op x;					\
    return out;								\
  }
  

//! Array quantity with units.
//! \tparam T Type of the underlying scalar.
//! \tparam Encoding Encoding used to store the unit strings.
template<typename T, typename Encoding>
class QuantityArray {
public:
  typedef Encoding EncodingType;    //!< Encoding type from template parameter.
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
  //! \brief Empty constructor.
  QuantityArray() : value_(nullptr), units_(), ndim_(0), shape_(nullptr) {}
  //! \brief Create a quantity.
  //! \param value Pointer to an array.
  //! \param ndim Number of dimensions in the array.
  //! \param shape Size of the array in each dimension.
  //! \param units Units instance.
  QuantityArray(const T* value, const SizeType& ndim, const SizeType* shape,
		const Units<Encoding>& units = Units<Encoding>()) :
    value_(nullptr), units_(units), ndim_(ndim), shape_(nullptr) {
    RAPIDJSON_ASSERT(ndim > 0);
    _init_shape(shape);
    _init_value(value);
  }
  //! \brief Create a quantity.
  //! \param value Pointer to an array.
  //! \param len Number of elements in the 1D array.
  //! \param units Units instance.
  QuantityArray(const T* value, const SizeType& len,
		const Units<Encoding>& units = Units<Encoding>()) :
    QuantityArray(value, 1, &len, units) {}
  //! \brief Create a quantity from units string.
  //! \param value Pointer to an array.
  //! \param ndim Number of dimensions in the array.
  //! \param shape Size of the array in each dimension.
  //! \param units Units string.
  QuantityArray(const T* value, const SizeType& ndim, const SizeType* shape,
		const Ch* units) :
    QuantityArray(value, ndim, shape, Units<Encoding>(units)) {}
  
  //! \brief Create a quantity without units.
  //! \tparam N Number of elements in the array.
  //! \param value 1D array.
  //! \param units Units instance.
  template<SizeType N>
  QuantityArray(const T (&value)[N], const Units<Encoding>& units = Units<Encoding>()) :
    QuantityArray(&(value[0]), N, units) {}
  //! \brief Constructor from units string.
  //! \tparam N Number of elements in the array.
  //! \param value 1D array.
  //! \param units Units string.
  template<SizeType N>
  QuantityArray(const T (&value)[N], const Ch* units) :
    QuantityArray(value, Units<Encoding>(units)) {}
  
  //! \brief Create a quantity without units.
  //! \tparam N Number of elements in the array in dimension 1.
  //! \tparam M Number of elements in the array in dimension 2.
  //! \param value 1D array.
  //! \param units Units instance.
  template<SizeType N, SizeType M>
  QuantityArray(const T (&value)[N][M], const Units<Encoding>& units = Units<Encoding>()) :
    value_(nullptr), units_(units), ndim_(2), shape_(nullptr) {
    SizeType shape[] = {N, M};
    _init_shape(&(shape[0]));
    _init_value(&(value[0][0]));
  }
  //! \brief Constructor from units string.
  //! \tparam N Number of elements in the array in dimension 1.
  //! \tparam M Number of elements in the array in dimension 2.
  //! \param value 1D array.
  //! \param units Units string.
  template<SizeType N, SizeType M>
  QuantityArray(const T (&value)[N][M], const Ch* units) :
    QuantityArray(value, Units<Encoding>(units)) {}
  //! \brief Copy constructor.
  //! \param other QuantityArray to copy.
  QuantityArray(const QuantityArray<T, Encoding>& other) :
    QuantityArray() { *this = other; }
  //! \brief Destructor.
  ~QuantityArray() {
    if (value_ != nullptr) free(value_);
    if (shape_ != nullptr) free(shape_);
    ndim_ = 0;
  }
  //! \brief Copy assignment.
  //! \param other QuantityArray to copy.
  //! \return Copy.
  QuantityArray<T, Encoding>& operator=(const QuantityArray<T, Encoding>& other) {
    if (value_ != nullptr) free(value_);
    if (shape_ != nullptr) free(shape_);
    ndim_ = other.ndim_;
    units_ = other.units_;
    _init_shape(other.shape_);
    _init_value(other.value_);
    return *this;
  }
  //! \brief Print instance information to an output stream.
  //! \param os Output stream.
  void display(std::ostream& os) const {
    os << "QuantityArray(";
    _write_array(os);
    os << ", ";
    units_.display(os);
    os << ")";
  }
  //! \brief Get the total number of elements in the array.
  //! \return The number of elements.
  SizeType nelements() const {
    SizeType out = 0;
    if (ndim_ > 0) {
      out = 1;
      for (SizeType i = 0; i < ndim_; i++)
	out = out * shape_[i];
    }
    return out;
  }
private:
  std::vector<SizeType> _index(const SizeType idx) const {
    RAPIDJSON_ASSERT(ndim_ > 0);
    std::vector<SizeType> out;
    SizeType prev = 0;
    for (SizeType i = 0; i < (ndim_ - 1); i++) {
      out.push_back((idx - prev) / shape_[i]);
      prev += shape_[i] * out[i];
    }
    out.push_back(idx - prev);
    return out;
  }
  void _init_shape(const SizeType* shape) {
    shape_ = (SizeType*)malloc(ndim_ * sizeof(SizeType));
    RAPIDJSON_ASSERT(shape_);
    for (SizeType i = 0; i < ndim_; i++)
      shape_[i] = shape[i];
  }
  void _init_value(const T* value) {
    SizeType N = nelements();
    value_ = (T*)malloc(N * sizeof(T));
    for (SizeType i = 0; i < N; i++)
      value_[i] = value[i];
  }
  void _write_array(std::ostream& os) const {
    SizeType N = nelements();
    std::vector<SizeType> idx;
    for (SizeType i = 0; i < N; i++) {
      idx = _index(i);
      for (SizeType j = 0; j < ndim_; j++) {
	if (idx[j] == 0) {
	  if ((j == 0) || (idx[j - 1] == 0)) {
	    os << "[";
	  } else {
	    os << "], [";
	  }
	}
      }
      if (idx[ndim_ - 1] > 0)
	os << ", ";
      os << value_[i];
    }
    for (SizeType i = 0; i < ndim_; i++)
      os << "]";
  }
  QuantityArray raw_add(const QuantityArray& x, double factor=1.0) const {
    // Assumes units have already been matched
    RAPIDJSON_ASSERT(is_same_shape(x));
    QuantityArray out(value_, ndim_, shape_, units_);
    SizeType N = nelements();
    for (SizeType i = 0; i < N; i++)
      out.value_[i] = value_[i] + (factor * x.value_[i]);
    return out;
  }
public:
  //! \brief Get the quantity value without units.
  //! \return Value.
  const T* value() const { return value_; };
  //! \brief Get the units instance.
  //! \return Units.
  Units<Encoding> units() const { return units_; }
  //! \brief Get the number of dimensions in the array.
  //! \return Number of dimensions.
  SizeType ndim() const { return ndim_; }
  //! \brief Get the size of the array in each dimension.
  //! \return Array shape.
  const SizeType* shape() const { return shape_; }
  //! \brief Return the pointer to the value and then reset it.
  //! \return Value.
  T* pop_value() {
    T* out = value_;
    value_ = nullptr;
    return out;
  };
  //! \brief Retrun the pointer to the shape and then reset it.
  //! \return Shape.
  SizeType* pop_shape() {
    SizeType* out = shape_;
    shape_ = nullptr;
    return out;
  }
  //! \brief Get the units string.
  //! \return Units string.
  std::basic_string<Ch> unitsStr() const { return units_.str(); }
  //! \brief Check if another quantity array has the same shape.
  //! \param x QuantityArray for comparison.
  //! \return true if the shapes are equivalent, false otherwise.
  bool is_same_shape(const QuantityArray& x) const {
    if (ndim_ != x.ndim_) return false;
    for (SizeType i = 0; i < ndim_; i++)
      if (shape_[i] != x.shape_[i]) return false;
    return true;
  }
  //! \brief Check if two quantities are identical. The units must be
  //!   identical, not just compatible.
  //! \param x QuantityArray for comparison.
  //! \return true if the two quantities are identical, false otherwise.
  bool operator==(const QuantityArray& x) const {
    if (units_ != x.units_) return false;
    if (!is_same_shape(x)) return false;
    for (SizeType i = 0; i < nelements(); i++)
      if (!(compare_values(value_[i], x.value_[i]))) return false;
    return true;
  }
  //! \brief Check if two quantities are not identical.
  //! \param x QuantityArray for comparison.
  //! \return true if the two quantities are not identical, false otherwise.
  bool operator!=(const QuantityArray& x) const { return (!(*this==x)); }
  //! \brief Multiply by another quantity element by element.
  //! \param x QuantityArray to multiply by.
  //! \return Result of multiplication.
  ARRAY_ARRAY_OP(*);
  //! \brief Divide by another quantity.
  //! \param x QuantityArray to divide by.
  //! \return Result of division.
  ARRAY_ARRAY_OP(/);
  //! \brief Multiply by a scalar.
  //! \tparam T2 Scalar type.
  //! \param x Scalar to multiply by.
  //! \return Result of multiplication.
  ARRAY_SCALAR_OP(*);
  //! \brief Divide by a scalar.
  //! \tparam T2 Scalar type.
  //! \param x Scalar to divide by.
  //! \return Result of division.
  ARRAY_SCALAR_OP(/);
  //! \brief Add a quantity with compatible units.
  //! \param x QuantityArray to add.
  //! \return Result of addition.
  QuantityArray operator+(const QuantityArray& x) const {
    if (units_ != x.units_)
      return (*this + x.as(units_));
    return raw_add(x); }
  //! \brief Subtract a quantity with compatible units.
  //! \param x QuantityArray to subtract.
  //! \return Result of subtraction.
  QuantityArray operator-(const QuantityArray& x) const {
    if (units_ != x.units_)
      return (*this - x.as(units_));
    return raw_add(x, -1.0); }
  //! \brief Perform power operation in place.
  //! \param x Power to raise this quantity to.
  void inplace_pow(const double& x) {
    SizeType N = nelements();
    for (SizeType i = 0; i < N; i++)
      value_[i] = std::pow(value_[i], x);
    units_.inplace_pow(x);
  }
  //! \brief Raise this quantity to a power.
  //! \param x Power to raise this quantity to.
  //! \return Resulting quantity.
  QuantityArray pow(const double& x) const {
    QuantityArray out(*this);
    out.inplace_pow(x);
    return out;
  }
  //! \brief Get the dimensions of this quantity's units.
  //! \return The dimensions of the units.
  Dimension dimension() const { return units_.dimension(); }
  //! \brief Check if another quantity has compatible units with the same
  //!   dimensions.
  //! \param x QuantityArray for comparison.
  //! \return true if the units are compatible, false otherwise.
  bool is_compatible(const QuantityArray& x) const {
    return (dimension() == x.dimension());
  }
  //! \brief Check if another quantity is equivalent to this one, allowing
  //!    for the possibility that it has different, but compatible, units.
  //! \param x QuantityArray for comparison.
  //! \return true if the two quantities are equivalent, false otherwise.
  bool equivalent_to(const QuantityArray& x) {
    if (!(is_compatible(x)))
      return false;
    return (*this==x.as(units_));
  }
  //! \brief Convert the quantity to a different set of units. The new units
  //!   must be compatible with the current ones.
  //! \param units New units.
  void convert_to(const Units<Encoding>& units) {
    std::vector<double> factor = units_.conversion_factor(units);
    SizeType N = nelements();
    for (SizeType i = 0; i < N; i++)
      value_[i] = Quantity<T,Encoding>::template do_conv<T>(value_[i], factor[0], factor[1]);
    units_ = Units<Encoding>(units);
  }
  //! \brief Create a new quantity by converting this one to a new set of
  //!   compatible units.
  //! \param units New units.
  //! \return New quantity.
  QuantityArray as(const char* units0) const {
    Units<Encoding> units(units0);
    return as(units);
  }
  //! \brief Create a new quantity by converting this one to a new set of
  //!   compatible units.
  //! \param units New units.
  //! \return New quantity.
  QuantityArray as(const Units<Encoding>& units) const {
    QuantityArray out(*this);
    out.convert_to(units);
    return out;
  }
private:
  T* value_;
  Units<Encoding> units_;
  SizeType ndim_;
  SizeType* shape_;
  template<typename U, typename Encoding2>
  friend std::ostream & operator << (std::ostream &os, const QuantityArray<U,Encoding2> &x);
};
template<typename T, typename Encoding>
inline std::ostream & operator << (std::ostream &os, const QuantityArray<T, Encoding> &x) {
  x._write_array(os);
  os << " " << x.units_;
  return os;
};
  
#undef ARRAY_ARRAY_OP
#undef ARRAY_SCALAR_OP

} // namespace units
  
#endif // RAPIDJSON_YGGDRASIL

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_UNITS_H_
