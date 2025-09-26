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
#include "internal/strfunc.h"
#include "precision.h"
#include "rapidjson.h"
#include <wchar.h>
#include <locale.h>
#include <map>
#include <vector>
#include <set>
#include <iostream>
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#if defined(__GNUC__) || defined(__clang__)
#define UNIT_TYPE \
  static double __attribute__((unused))
#else
#define UNIT_TYPE \
  static double
#endif

RAPIDJSON_NAMESPACE_BEGIN

#ifdef RAPIDJSON_YGGDRASIL

#if RAPIDJSON_HAS_CXX11
#define OVERRIDE_CXX11 override
#else // RAPIDJSON_HAS_CXX11
#define OVERRIDE_CXX11
#endif // RAPIDJSON_HAS_CXX11

namespace units {

  enum DeltaFlag {
    kNoDelta,
    kInactiveDelta,
    kActiveDelta
  };

  namespace constants {
    
    // Elementary masses
    UNIT_TYPE mass_electron_kg = 9.10938291e-31;
    UNIT_TYPE amu_kg = 1.660538921e-27;
    UNIT_TYPE amu_grams = amu_kg * 1.0e3;
    UNIT_TYPE mass_hydrogen_kg = 1.007947 * amu_kg;
    // UNIT_TYPE mass_proton_kg = 1.672623110e-27;

    // Solar values (see Mamajek 2012)
    // https://sites.google.com/site/mamajeksstarnotes/bc-scale
    UNIT_TYPE mass_sun_kg = 1.98841586e30;
    UNIT_TYPE temp_sun_kelvin = 5870.0;
    UNIT_TYPE luminosity_sun_watts = 3.8270e26;

    // Consistent with solar abundances used in Cloudy
    UNIT_TYPE metallicity_sun = 0.01295;

    // Conversion Factors:  X au * mpc_per_au = Y mpc
    // length
    // UNIT_TYPE mpc_per_mpc = 1e0;
    UNIT_TYPE mpc_per_kpc = 1e-3;
    UNIT_TYPE mpc_per_pc = 1e-6;
    UNIT_TYPE mpc_per_au = 4.84813682e-12;
    UNIT_TYPE mpc_per_rsun = 2.253962e-14;
    UNIT_TYPE mpc_per_rearth = 2.06470307893e-16;
    UNIT_TYPE mpc_per_rjup = 2.26566120943e-15;
    UNIT_TYPE mpc_per_miles = 5.21552871e-20;
    UNIT_TYPE mpc_per_km = 3.24077929e-20;
    UNIT_TYPE mpc_per_m = 3.24077929e-23;
    UNIT_TYPE kpc_per_m = mpc_per_m / mpc_per_kpc;
    UNIT_TYPE pc_per_m = mpc_per_m / mpc_per_pc;
    UNIT_TYPE km_per_pc = 3.08567758e13;
    // UNIT_TYPE cm_per_pc = 3.08567758e18;
    // UNIT_TYPE cm_per_mpc = 3.08567758e21;
    UNIT_TYPE km_per_m = 1e-3;
    UNIT_TYPE km_per_cm = 1e-5;
    UNIT_TYPE m_per_cm = 1e-2;
    UNIT_TYPE ly_per_m = 1.05702341e-16;
    UNIT_TYPE rsun_per_m = 1.4378145e-9;
    UNIT_TYPE rearth_per_m = 1.56961033e-7;  // Mean (volumetric) radius
    UNIT_TYPE rjup_per_m = 1.43039006737e-8;  // Mean (volumetric) radius
    UNIT_TYPE au_per_m = 6.68458712e-12;
    UNIT_TYPE ang_per_m = 1.0e10;

    // UNIT_TYPE m_per_fpc = 0.0324077929;

    UNIT_TYPE kpc_per_mpc = 1.0 / mpc_per_kpc;
    UNIT_TYPE pc_per_mpc = 1.0 / mpc_per_pc;
    UNIT_TYPE au_per_mpc = 1.0 / mpc_per_au;
    UNIT_TYPE rsun_per_mpc = 1.0 / mpc_per_rsun;
    UNIT_TYPE rearth_per_mpc = 1.0 / mpc_per_rearth;
    UNIT_TYPE rjup_per_mpc = 1.0 / mpc_per_rjup;
    UNIT_TYPE miles_per_mpc = 1.0 / mpc_per_miles;
    UNIT_TYPE km_per_mpc = 1.0 / mpc_per_km;
    UNIT_TYPE m_per_mpc = 1.0 / mpc_per_m;
    UNIT_TYPE m_per_kpc = 1.0 / kpc_per_m;
    UNIT_TYPE m_per_km = 1.0 / km_per_m;
    UNIT_TYPE cm_per_km = 1.0 / km_per_cm;
    UNIT_TYPE cm_per_m = 1.0 / m_per_cm;
    UNIT_TYPE pc_per_km = 1.0 / km_per_pc;
    UNIT_TYPE m_per_pc = 1.0 / pc_per_m;
    UNIT_TYPE m_per_ly = 1.0 / ly_per_m;
    UNIT_TYPE m_per_rsun = 1.0 / rsun_per_m;
    UNIT_TYPE m_per_rearth = 1.0 / rearth_per_m;
    UNIT_TYPE m_per_rjup = 1.0 / rjup_per_m;
    UNIT_TYPE m_per_au = 1.0 / au_per_m;
    UNIT_TYPE m_per_ang = 1.0 / ang_per_m;

    // time
    // "IAU Style Manual" by G.A. Wilkins, Comm. 5, in IAU Transactions XXB (1989)
    // UNIT_TYPE sec_per_Gyr = 31.5576e15;
    // UNIT_TYPE sec_per_Myr = 31.5576e12;
    // UNIT_TYPE sec_per_kyr = 31.5576e9;
    UNIT_TYPE sec_per_year = 31.5576e6;
    UNIT_TYPE sec_per_day = 86400.0;
    UNIT_TYPE sec_per_hr = 3600.0;
    UNIT_TYPE sec_per_min = 60.0;
    // UNIT_TYPE day_per_year = 365.25;

    // velocities, accelerations
    UNIT_TYPE speed_of_light_m_per_s = 2.99792458e8;
    UNIT_TYPE speed_of_light_cm_per_s = speed_of_light_m_per_s * 100.0;
    UNIT_TYPE standard_gravity_m_per_s2 = 9.80665;

    // some constants
    UNIT_TYPE newton_mks = 6.67408e-11;
    UNIT_TYPE planck_mks = 6.62606957e-34;
    // permeability of Free Space
    UNIT_TYPE mu_0 = 4.0e-7 * M_PI;
    // permittivity of Free Space
    UNIT_TYPE eps_0 = 1.0 / (pow(speed_of_light_m_per_s, 2) * mu_0);
    // UNIT_TYPE avogadros_number = 6.02214085774e23;

    // temperature / energy
    UNIT_TYPE boltzmann_constant_J_per_K = 1.3806488e-23;
    UNIT_TYPE erg_per_eV = 1.602176562e-12;
    UNIT_TYPE J_per_eV = erg_per_eV * 1.0e-7;
    UNIT_TYPE erg_per_keV = erg_per_eV * 1.0e3;
    UNIT_TYPE J_per_keV = J_per_eV * 1.0e3;
    UNIT_TYPE K_per_keV = J_per_keV / boltzmann_constant_J_per_K;
    UNIT_TYPE keV_per_K = 1.0 / K_per_keV;
    UNIT_TYPE keV_per_erg = 1.0 / erg_per_keV;
    UNIT_TYPE eV_per_erg = 1.0 / erg_per_eV;
    UNIT_TYPE kelvin_per_rankine = 5.0 / 9.0;
    UNIT_TYPE watt_per_horsepower = 745.69987158227022;
    UNIT_TYPE celcius_zero_kelvin = -273.15;
    UNIT_TYPE farenheit_zero_celcius = 32.0;
    UNIT_TYPE farenheit_zero_kelvin = celcius_zero_kelvin / kelvin_per_rankine + farenheit_zero_celcius;
    // UNIT_TYPE erg_per_s_per_watt = 1e7;

    // Solar System masses
    // Standish, E.M. (1995) "Report of the IAU WGAS Sub-Group on Numerical
    // Standards", in Highlights of Astronomy (I. Appenzeller, ed.), Table 1,
    // Kluwer Academic Publishers, Dordrecht.
    // REMARK: following masses include whole systems (planet + moons)
    UNIT_TYPE mass_jupiter_kg = mass_sun_kg / 1047.3486;
    UNIT_TYPE mass_mercury_kg = mass_sun_kg / 6023600.0;
    UNIT_TYPE mass_venus_kg = mass_sun_kg / 408523.71;
    UNIT_TYPE mass_earth_kg = mass_sun_kg / 328900.56;
    UNIT_TYPE mass_mars_kg = mass_sun_kg / 3098708.0;
    UNIT_TYPE mass_saturn_kg = mass_sun_kg / 3497.898;
    UNIT_TYPE mass_uranus_kg = mass_sun_kg / 22902.98;
    UNIT_TYPE mass_neptune_kg = mass_sun_kg / 19412.24;

    // flux
    UNIT_TYPE jansky_mks = 1.0e-26;
    // Cosmological constants
    // Calculated with H = 100 km/s/Mpc, value given in units of h^2 g cm^-3
    // Multiply by h^2 to get the critical density in units of g cm^-3
    // UNIT_TYPE rho_crit_g_cm3_h2 = 1.8788e-29;
    // UNIT_TYPE primordial_H_mass_fraction = 0.76;

    // Misc. Approximations
    // UNIT_TYPE mass_mean_atomic_cosmology = 1.22;
    // UNIT_TYPE mass_mean_atomic_galactic = 2.3;

    // Miscellaneous
    // UNIT_TYPE HUGE = 1.0e90;
    // UNIT_TYPE TINY = 1.0e-40;

    // Planck units
    UNIT_TYPE hbar_mks = 0.5 * planck_mks / M_PI;
    UNIT_TYPE planck_mass_kg = sqrt(hbar_mks * speed_of_light_m_per_s / newton_mks);
    UNIT_TYPE planck_length_m = sqrt(hbar_mks * newton_mks / pow(speed_of_light_m_per_s, 3));
    UNIT_TYPE planck_time_s = planck_length_m / speed_of_light_m_per_s;
    UNIT_TYPE planck_energy_J = planck_mass_kg * speed_of_light_m_per_s * speed_of_light_m_per_s;
    UNIT_TYPE planck_temperature_K = planck_energy_J / boltzmann_constant_J_per_K;
    UNIT_TYPE planck_charge_C = sqrt(4.0 * M_PI * eps_0 * hbar_mks * speed_of_light_m_per_s);

    // Imperial and other non-metric units
    UNIT_TYPE kg_per_pound = 0.45359237;
    UNIT_TYPE pascal_per_atm = 101325.0;
    UNIT_TYPE m_per_inch = 0.0254;
    UNIT_TYPE m_per_ft = 0.3048;

    // logarithmic units
    // IEC 60027-3: https://webstore.iec.ch/publication/94
    // NIST Special Publication 811: https://www.nist.gov/pml/special-publication-811
    // UNIT_TYPE neper_per_bel = log(10) / 2;
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

  template<typename Ch>
  std::vector<std::basic_string<Ch> > pack_strings(const Ch* first...) {
    std::vector<std::basic_string<Ch> > out;
    out.push_back(first);
    va_list args;
    va_start(args, first);
    while (true) {
      Ch* i = va_arg(args, Ch*);
      if (i == NULL) break;
      out.push_back(i);
    }
    va_end(args);
    return out;
  }
  
#define PACK_LUT(x, args) (void*)(new x args)

  template<typename T>
  class CachedLUT {
  public:
    CachedLUT(void* first...) :
      base_(), cache_() {
      if (!first) return;
      T* firstT = (T*)first;
      base_.push_back(*firstT);
      va_list args;
      va_start(args, first);
      _add_from_va(args);
      va_end(args);
      delete firstT;
    }
    CachedLUT(const std::vector<T>* base...) :
      base_(), cache_() {
      va_list args;
      va_start(args, base);
      _add_from_va(args);
      va_end(args);
      base_.insert(base_.end(), base->begin(), base->end());
    }
    ~CachedLUT() {
      for (std::map<int32_t, void*>::iterator it = cache_.begin(); it != cache_.end(); it++)
        free(it->second);
    }
  private:
    void _add_from_va(va_list args) {
      while (true) {
	T* i = (T*)va_arg(args, void*);
	if (!i) return;
	base_.push_back(*i);
	delete i;
      }
    }
  public:
    template<typename T2>
    const std::vector<T2>* get(RAPIDJSON_ENABLEIF((internal::IsSame<T, T2>)))
    { return &base_; }
    template<typename T2>
    const std::vector<T2>* get(RAPIDJSON_DISABLEIF((internal::IsSame<T, T2>))) {
      int32_t idx = T2::EncodingType::HashCode();
      std::map<int32_t, void*>::iterator match = cache_.find(idx);
      if (match == cache_.end()) {
        std::vector<T2>* new_entry = (std::vector<T2>*)malloc(sizeof(std::vector<T2*>));
	RAPIDJSON_ASSERT(new_entry);
	new_entry[0] = std::vector<T2>();
        for (typename std::vector<T>::const_iterator it = base_.begin(); it != base_.end(); it++)
          new_entry->push_back(it->template transcode<typename T2::EncodingType>());
	cache_[idx] = (void*)(new_entry);
        return new_entry;
      } else {
	return (std::vector<T2>*)(match->second);
      }
    }
  private:
    std::vector<T> base_;
    std::map<int32_t, void*> cache_;
  };

// Forward declarations
template<typename Encoding>
class GenericUnit;
template<typename Encoding>
class GenericUnits;
template<typename T, typename Encoding>
class GenericQuantity;
template<typename T, typename Encoding>
class GenericQuantityArray;

class Dimension {
private:
  struct DimArray { double values[8]; };
  static DimArray const& packArray() {
    static DimArray const values = {
      {0, 0, 0, 0, 0, 0, 0, 0} };
    return values;
  }
  static DimArray packArray(const double powers[8]) {
    DimArray values = {
      {powers[0], powers[1], powers[2], powers[3], powers[4], powers[5],
       powers[6], powers[7]} };
    return values;
  }
  static DimArray packArray(const double p0, const double p1, const double p2,
			    const double p3, const double p4, const double p5,
			    const double p6, const double p7) {
    DimArray values = {p0, p1, p2, p3, p4, p5, p6, p7};
    return values;
  }
public:
  Dimension(const BaseDimension k, const double power=1.0) :
    powers_(packArray()) {
    if (k != kDimensionlessUnit)
      powers_.values[k] = power;
  }
  Dimension(const double L = 0.0,
	    const double M = 0.0,
	    const double T = 0.0,
	    const double EC = 0.0,
	    const double Temp = 0.0,
	    const double N = 0.0,
	    const double LI = 0.0,
	    const double A = 0.0) :
    powers_(packArray(L, M, T, EC, Temp, N, LI, A)) {}
  Dimension(const double powers[8]) : powers_(packArray(powers)) {}
  Dimension operator*(const Dimension& x) const {
    double new_powers[8];
    for (size_t i = 0; i < 8; i++)
      new_powers[i] = powers_.values[i] + x.powers_.values[i];
    return Dimension(new_powers);
  }
  Dimension operator/(const Dimension& x) const {
    double new_powers[8];
    for (size_t i = 0; i < 8; i++)
      new_powers[i] = powers_.values[i] - x.powers_.values[i];
    return Dimension(new_powers);
  }
  void pow_inplace(const double x) {
    for (size_t i = 0; i < 8; i++)
      powers_.values[i] = powers_.values[i] * x;
  }
  Dimension pow(const double x) const {
    Dimension new_dim = Dimension(*this);
    new_dim.pow_inplace(x);
    return new_dim;
  }
  bool operator==(const Dimension& x) const {
    for (size_t i = 0; i < 8; i++)
      if (!(internal::values_eq(powers_.values[i], x.powers_.values[i])))
	return false;
    return true;
  }
  bool operator!=(const Dimension& x) const { return (!(*this == x)); }
  int ndim() const {
    int ndim = 0;
    for (size_t i = 0; i < 8; i++)
      if (!(internal::values_eq(powers_.values[i], 0.0)))
	ndim++;
    return ndim;
  }
  bool is_irreducible() const { return (ndim() == 1); }
  bool is_dimensionless() const { return (ndim() == 0); }
  std::vector<Dimension> reduced() const {
    std::vector<Dimension> out;
    for (size_t i = 0; i < 8; i++)
      if (!(internal::values_eq(powers_.values[i], 0.0)))
	out.push_back(Dimension((BaseDimension)i, powers_.values[i]));
    return out;
  }
protected:
  DimArray powers_;
  template<typename Ch>
  friend std::basic_ostream<Ch> & operator << (std::basic_ostream<Ch> &os, const Dimension &x);
};
template<typename Ch>
inline std::basic_ostream<Ch> & operator << (std::basic_ostream<Ch>& os, const Dimension &x) {
  os << '[' << x.powers_.values[0];
  for (size_t i = 1; i < 8; i++)
    os << ',' << x.powers_.values[i];
  os << ']';
  return os;
}

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
const std::basic_string<typename DestEncoding::Ch> convert_chars(const std::basic_string<typename SourceEncoding::Ch>& x,
								 RAPIDJSON_ENABLEIF((internal::IsSame<SourceEncoding, DestEncoding>))) {
  return x;
}
  
template<typename SourceEncoding, typename DestEncoding>
const std::basic_string<typename DestEncoding::Ch> convert_chars(const std::basic_string<typename SourceEncoding::Ch>& x,
								 RAPIDJSON_DISABLEIF((internal::IsSame<SourceEncoding, DestEncoding>))) {
  GenericStringStream<SourceEncoding> src(x.c_str());
  GenericStringBuffer<DestEncoding> dst;
  if (DestEncoding::supportUnicode)
    PutReserve(dst, x.size() * 6);
  else
    PutReserve(dst, x.size() * 12);
  while (RAPIDJSON_LIKELY(src.Tell() < x.size())) {
    RAPIDJSON_ASSERT(src.Peek() != '\0');
    Transcoder<SourceEncoding, DestEncoding>::Transcode(src, dst);
  }
  std::basic_string<typename DestEncoding::Ch> out(dst.GetString(), dst.GetLength());
  return out;
}
								 
//! Represents a unit prefix modifying a base unit.
/*!
  A prefix is defined by the full string (e.g. "kilo") and abbreviation
  (e.g. "k"), and a scale factor that should be applied when converting
  from the prefixed unit to the base unit (e.g. 1e3).
  \tparam Encoding Encoding of the prefix string.
 */
template<typename Encoding>
class GenericUnitPrefix {
public:
  typedef Encoding EncodingType;    //!< Encoding type from template parameter.
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
  //! Default constructor creates an empty prefix.
  GenericUnitPrefix() : abbr(), factor(1.0), name() {}
  //! Constructor from C-style string types
  /*!
    \param[in] abbr0 Abbreviation to use for the prefix.
    \param[in] factor0 Scale factor when going from the prefixed unit to
      the base unit.
    \param[in] name0 Full name to use for the prefix.
   */
  GenericUnitPrefix(const Ch* abbr0, const double& factor0, const Ch* name0) :
    abbr(abbr0), factor(factor0), name(name0) {}
  //! Constructor from C++-style string types
  /*!
    \param[in] abbr0 Abbreviation to use for the prefix.
    \param[in] factor0 Scale factor when going from the prefixed unit to
      the base unit.
    \param[in] name0 Full name to use for the prefix.
   */
  GenericUnitPrefix(const std::basic_string<Ch> abbr0, const double& factor0,
		    const std::basic_string<Ch> name0) :
    abbr(abbr0), factor(factor0), name(name0) {}
#if RAPIDJSON_HAS_CXX20
  //! Constructor from C-style string types with char8_t prefix
  /*!
    \param[in] abbr0 Abbreviation to use for the prefix.
    \param[in] factor0 Scale factor when going from the prefixed unit to
      the base unit.
    \param[in] name0 Full name to use for the prefix.
   */
  GenericUnitPrefix(const char8_t* abbr0, const double& factor0, const Ch* name0) :
    abbr(reinterpret_cast<const char*>(abbr0)),
    factor(factor0), name(name0) {}
#endif // RAPIDJSON_HAS_CXX20
  //! Abbreviation associated with the prefix.
  std::basic_string<Ch> abbr;
  //! Factor that the prefix implies.
  double factor;
  //! Full name associated with the prefix.
  std::basic_string<Ch> name;

  //! \brief Determine if this unit prefix is identical to another.
  //! \param x Unit prefix for comparison.
  //! \return true if the unit prefixes are identical.
  bool operator==(const GenericUnitPrefix& x) const {
    if (abbr != x.abbr) return false;
    return internal::values_eq(factor, x.factor);
  }
  //! \brief Determine if this unit prefix is not identical to another.
  //! \param x Unit prefix for comparison.
  //! \return true if the unit prefixes are not identical.
  bool operator!=(const GenericUnitPrefix& x) const { return (!(*this == x)); }

  //! \brief Create a copy of the prefix with a different encoding.
  //! \tparam DestEncoding Encoding that the copy should use.
  //! \return Copy w/ DestEncoding.
  template<typename DestEncoding>
  GenericUnitPrefix<DestEncoding> transcode() const {
    return GenericUnitPrefix<DestEncoding>(convert_chars<Encoding, DestEncoding>(abbr),
					   factor,
					   convert_chars<Encoding, DestEncoding>(name));
  }

  //! \brief Write the prefix to an output stream with class information.
  //! \param os Output stream.
  template<typename Ch2>
  void display(std::basic_ostream<Ch2>& os) const {
    os << 'G' << 'e' << 'n' << 'e' << 'r' << 'i' << 'c' << 'U' <<
      'n' << 'i' << 't' << 'P' << 'r' << 'e' << 'f' << 'i' << 'x' << '('
       << convert_chars<Encoding,UTF8<Ch2> >(name)
       << ',' << ' ' << convert_chars<Encoding,UTF8<Ch2> >(abbr)
       << ',' << ' ' << factor << ')';
  }
  
  friend class GenericUnit<Encoding>;
  template<typename Ch2, typename Enc2>
  friend std::basic_ostream<Ch2> & operator << (std::basic_ostream<Ch2>& os, const GenericUnitPrefix<Enc2> &x);
};
//! Output stream operator for GenericUnitPrefix.
/*!
  \tparam Ch Type used to represent characters in the output stream.
  \tparam Encoding Encoding of the prefix string.
  \param os Output stream.
  \param[in] x Prefix to output.
  \returns Output stream.
 */
template<typename Ch, typename Encoding>
inline std::basic_ostream<Ch> & operator << (std::basic_ostream<Ch>& os, const GenericUnitPrefix<Encoding> &x) {
  if (x.abbr.size() > 0)
    os << convert_chars<Encoding,UTF8<Ch> >(x.abbr);
  return os;
}

  //! Generic UnitPrefix with UTF8 encoding
  typedef GenericUnitPrefix<UTF8<char> > UnitPrefix;

//! Macro for constructing UnitPrefix entries for prefix LUTs.
#define PACK_PREFIX(...) PACK_LUT(UnitPrefix, (__VA_ARGS__))

  /*!
    \brief Cached lookup table containing recognized SI unit prefixes.
    
    This dictionary formatting was adapted from the magnitude package
    (secondarily via unyt), credit to Juan Reyero.
  */
  static CachedLUT<UnitPrefix> _unit_prefixes(
      PACK_PREFIX("Y", 1e24, "yotta"),
      PACK_PREFIX("Z", 1e21, "zetta"),
      PACK_PREFIX("E", 1e18, "exa"),
      PACK_PREFIX("P", 1e15, "peta"),
      PACK_PREFIX("T", 1e12, "tera"),
      PACK_PREFIX("G", 1e9, "giga"),
      PACK_PREFIX("M", 1e6, "mega"),
      PACK_PREFIX("k", 1e3, "kilo"),
      PACK_PREFIX("h", 1e2, "hecto"),
      PACK_PREFIX("da", 1e1, "deca"),
      PACK_PREFIX("d", 1e-1, "deci"),
      PACK_PREFIX("c", 1e-2, "centi"),
      PACK_PREFIX("m", 1e-3, "mili"),
      PACK_PREFIX("u", 1e-6, "micro"),
#if RAPIDJSON_HAS_CXX11
      PACK_PREFIX(u8"\u00b5", 1e-6, "micro"),  // ('MICRO SIGN' U+00B5)
      PACK_PREFIX(u8"\u03bc", 1e-6, "micro"),  // ('GREEK SMALL LETTER MU' U+03BC)
#else // RAPIDJSON_HAS_CXX11
      PACK_PREFIX("\u00b5", 1e-6, "micro"),  // ('MICRO SIGN' U+00B5)
      PACK_PREFIX("\u03bc", 1e-6, "micro"),  // ('GREEK SMALL LETTER MU' U+03BC)
#endif // RAPIDJSON_HAS_CXX11
      PACK_PREFIX("n", 1e-9, "nano"),
      PACK_PREFIX("p", 1e-12, "pico"),
      PACK_PREFIX("f", 1e-15, "femto"),
      PACK_PREFIX("a", 1e-18, "atto"),
      PACK_PREFIX("z", 1e-21, "zepto"),
      PACK_PREFIX("y", 1e-24, "yocto"),
      (void*)NULL
    );

#undef PACK_PREFIX

//! Class to represent a single unit.
/*!
  A unit is defined by the set of names that can be interpreted as the
  unit, abbreviations that are associated with the unit, the
  dimension that the unit measures, the scale factor that should be
  applied to convert the unit to the unit measuring the same dimension in
  the MKS unit system, the offset that should be applied to convert the
  unit to the MKS unit system, a flag indicating if the unit represents
  a difference within a dimension, the power that the unit is raised to
  (e.g. m**2), and the prefix that is applied to the unit.
  \tparam Encoding Encoding of the name, abbreviation, & prefix strings.
 */
template<typename Encoding>
class GenericUnit {
public:
  typedef Encoding EncodingType;    //!< Encoding type from template parameter.
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
  //! \brief Empty constructor.
  GenericUnit() :
    names_(), abbrs_(), dim_(), factor_(1.0), offset_(0.0), delta_(0), power_(1.0), prefix_() {}
  //! \brief Constructor from a look-up table.
  //! \param x Base unit.
  //! \param prefix Prefix that should be applied to the base unit.
  GenericUnit(const GenericUnit& x, const GenericUnitPrefix<Encoding>& prefix) :
    names_(x.names_), abbrs_(x.abbrs_), dim_(x.dim_), factor_(x.factor_), offset_(x.offset_), delta_(x.delta_), power_(x.power_), prefix_() {
    prefix_ = prefix;
    if (is_null()) {
      factor_ = std::pow(factor_, power_);
      power_ = 1.0;
    }
    _check_valid();
    RAPIDJSON_ASSERT(_check_valid());
  }
  //! \brief Construct from a single name/abbreviation.
  //! \param name Name.
  //! \param abbr Abbreviation.
  //! \param dim Dimensions.
  //! \param factor Scale factor from the base unit system.
  //! \param offset Offset from the zero point of the base unit system.
  //! \param power Power that will be applied to the unit during conversion.
  //! \param delta Flag indicating if a unit has special handling of
  //!   differences.
  //! \param prefix Prefix that should be applied to the base unit.
  GenericUnit(const Ch* name, const Ch* abbr, const Dimension dim,
	      const double factor=1.0, const double offset=0.0,
	      const int delta=0, const double power=1.0, 
	      const GenericUnitPrefix<Encoding>& prefix=GenericUnitPrefix<Encoding>()) :
    names_(), abbrs_(), dim_(dim), factor_(factor), offset_(offset), delta_(delta), power_(power), prefix_(prefix) {
    names_.push_back(std::basic_string<Ch>(name));
    abbrs_.push_back(std::basic_string<Ch>(abbr));
    _add_plural();
    _check_valid();
    RAPIDJSON_ASSERT(_check_valid());
  }
  //! \brief Construct from a single name/abbreviation.
  //! \param names Names.
  //! \param abbrs Abbreviations.
  //! \param dim Dimensions.
  //! \param factor Scale factor from the base unit system.
  //! \param offset Offset from the zero point of the base unit system.
  //! \param power Power that will be applied to the unit during conversion.
  //! \param delta Flag indicating if a unit has special handling of
  //!   differences.
  //! \param prefix Prefix that should be applied to the base unit.
  //! \param no_plural If true, the plural versions of the names will not be
  //!   added.
  GenericUnit(const std::vector<std::basic_string<Ch> >& names,
	      const std::vector<std::basic_string<Ch> >& abbrs,
	      const Dimension dim, const double factor=1.0, const double offset=0.0,
	      const int delta=0, const double power=1.0,
	      const GenericUnitPrefix<Encoding>& prefix=GenericUnitPrefix<Encoding>(),
	      const bool& no_plural=false) :
    names_(names), abbrs_(abbrs), dim_(dim), factor_(factor), offset_(offset), delta_(delta), power_(power), prefix_(prefix) {
    if (!no_plural) _add_plural();
    if (is_null()) {
      factor_ = std::pow(factor_, power_);
      power_ = 1.0;
    }
    _check_valid();
    RAPIDJSON_ASSERT(_check_valid());
  }
  //! \brief Construct a unit by looking up a string in the tables of
  //!   recognized units.
  //! \param str Unit string.
  //! \param power Power that should be applied to the located unit.
  //! \param prefix Prefix that should be applied to the base unit.
  GenericUnit(const std::basic_string<Ch> str, const double& power=1.0) :
    names_(), abbrs_(), dim_(), factor_(1.0), offset_(0.0), delta_(0), power_(1.0), prefix_() {
    bool errorFlag = (!from_table(str));
    (void)errorFlag;
    power_ = power; // Base units do not have powers
    (void)str;
    if (is_null()) {
      factor_ = std::pow(factor_, power_);
      power_ = 1.0;
    }
    _check_valid();
    RAPIDJSON_ASSERT(_check_valid());
  }
  //! \brief Set instance attributes based on an entry from one of the lookup
  //!   tables.
  //! \param found Table entry.
  //! \param If true, the unit will be marked as a difference.
  //! \param prefix Prefix that should be applied to the base unit.
  //! \return true if the unit could be initialized, false otherwise.
  bool from_table(const GenericUnit<Encoding>& found, bool delta,
		  const GenericUnitPrefix<Encoding>& prefix=GenericUnitPrefix<Encoding>()) {
    RAPIDJSON_ASSERT((found.names_.size() > 0) && (found.abbrs_.size() > 0));
    names_.insert(names_.begin(), found.names_.begin(), found.names_.end());
    abbrs_.insert(abbrs_.begin(), found.abbrs_.begin(), found.abbrs_.end());
    RAPIDJSON_ASSERT((names_.size() > 0) && (abbrs_.size() > 0));
    dim_ = found.dim_;
    factor_ = found.factor_;
    offset_ = found.offset_;
    delta_ = found.delta_;
    if (delta)
      delta_ = kActiveDelta;
    prefix_ = prefix;
    return true;
  }
  //! \brief Write the unit to an output stream with class information.
  //! \param os Output stream.
  template<typename Ch2>
  void display(std::basic_ostream<Ch2>& os) const {
    RAPIDJSON_ASSERT(names_.size() > 0);
    os << 'G' << 'e' << 'n' << 'e' << 'r' << 'i' << 'c' << 'U' <<
      'n' << 'i' << 't' << '(' << '\"';
    if (delta_ == kActiveDelta)
      os << get_delta<Ch2>();
    if (prefix_.name.size() > 0)
      os << convert_chars<Encoding,UTF8<Ch2> >(prefix_.name);
    os << convert_chars<Encoding,UTF8<Ch2> >(names_[0]) << '\"' << ','
       << '\"' << dim_ << ',' << ' '
       << factor_ << ',' << ' ' << offset_ << ')' << '*' << '*' << power_;
  }
  //! \brief Get the dimensions of the unit, including the power.
  //! \return The dimensions of the unit.
  Dimension dimension() const { return dim_.pow(power_); }
  //! \brief Determine if the unit is dimensionless.
  //! \return true if the unit is dimensionless, false otherwise.
  bool is_dimensionless() const { return dim_.is_dimensionless(); }
  //! \brief Check if this unit is equal to another.
  //! \param x Unit to check against.
  //! \return true if this unit is equal to x.
  bool operator==(const GenericUnit& x) const {
    if (names_ != x.names_) return false;
    if (abbrs_ != x.abbrs_) return false;
    if (dim_ != x.dim_) return false;
    if (!(internal::values_eq(factor_, x.factor_))) return false;
    if (!(internal::values_eq(offset_, x.offset_))) return false;
    if (delta_ != x.delta_) return false;
    if (!(internal::values_eq(power_, x.power_))) return false;
    if (prefix_ != x.prefix_) return false;
    return true;
  }
  //! \brief Check if this unit is not equal to another.
  //! \param x GenericUnit to check against.
  //! \return true if this unit is not equal to x.
  bool operator!=(const GenericUnit& x) const { return (!(*this == x)); }
  //! \brief Create a copy of the unit with a different encoding.
  //! \tparam DestEncoding Encoding that the copy should use.
  //! \return Copy w/ DestEncoding.
  template<typename DestEncoding>
  GenericUnit<DestEncoding> transcode() const {
    std::vector<std::basic_string<typename DestEncoding::Ch> > names, abbrs;
    for (typename std::vector<std::basic_string<Ch> >::const_iterator it = names_.begin(); it != names_.end(); it++)
      names.push_back(convert_chars<Encoding, DestEncoding>(*it));
    for (typename std::vector<std::basic_string<Ch> >::const_iterator it = abbrs_.begin(); it != abbrs_.end(); it++)
      abbrs.push_back(convert_chars<Encoding, DestEncoding>(*it));
    return GenericUnit<DestEncoding>(names, abbrs, dim_, factor_, offset_, delta_, power_,
				     prefix_.template transcode<DestEncoding>());
  }
  //! \brief Perform power operation in place.
  //! \param x Power to raise this unit to.
  template<typename T>
  void pow_inplace(const T x) {
    if (is_null())
      factor_ = std::pow(factor_, x);
    else
      power_ = power_ * x;
    _check_valid();
    RAPIDJSON_ASSERT(_check_valid());
  }
  //! \brief Raise this unit to a power.
  //! \param x Power to raise this unit to.
  //! \return Resulting unit.
  template<typename T>
  GenericUnit pow(const T x) const {
    GenericUnit new_unit(*this);
    new_unit.pow_inplace(x);
    return new_unit;
  }
  //! \brief Check if this unit and another have the same base unit.
  //! \param x Unit to compare this unit to.
  //! \return true if this unit and x have the same base unit.
  bool is_same_base(const GenericUnit& x) const {
    RAPIDJSON_ASSERT(x.names_.size() > 0);
    RAPIDJSON_ASSERT(names_.size() > 0);
    return (x.names_[0] == names_[0]);
  }
  //! \brief Check if a string matches any of the names or abbreviations
  //!   associated with this unit.
  //! \param str String to check.
  //! \return true if str matches this unit.
  bool matches(const std::basic_string<Ch> str) const {
    for (typename std::vector<std::basic_string<Ch> >::const_iterator n = names_.begin(); n != names_.end(); n++) {
      if (str.compare(*n) == 0)
	return true;
    }
    for (typename std::vector<std::basic_string<Ch> >::const_iterator n = abbrs_.begin(); n != abbrs_.end(); n++) {
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
	       const GenericUnitPrefix<Encoding>& prefix) const {
    for (typename std::vector<std::basic_string<Ch> >::const_iterator n = names_.begin(); n != names_.end(); n++) {
      if (str.compare(prefix.name + (*n)) == 0)
	return true;
    }
    for (typename std::vector<std::basic_string<Ch> >::const_iterator n = abbrs_.begin(); n != abbrs_.end(); n++)
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
		      std::vector<const GenericUnit*>& possibilities) const {
    for (typename std::vector<std::basic_string<Ch> >::const_iterator n = names_.begin(); n != names_.end(); n++) {
      if ((str.length() > n->length()) &&
	  (str.compare(str.length() - n->length(), n->length(), *n) == 0))
	possibilities.push_back(this);
    }
    for (typename std::vector<std::basic_string<Ch> >::const_iterator n = abbrs_.begin(); n != abbrs_.end(); n++) {
      if ((str.length() > n->length()) &&
	  (str.compare(str.length() - n->length(), n->length(), *n) == 0))
	possibilities.push_back(this);
    }
    return false;
  }
  //! \brief Check if this unit has a non-zero offset.
  //! \return true if this unit has a non-zero offset.
  bool has_offset() const { return (!is_difference() &&
				    !(internal::values_eq(offset_, 0.0))); }
  //! \brief Check if this unit has a power other than 1.
  //! \return true if this unit has a power other than 1.
  bool has_power() const { return (!(internal::values_eq(power_, 1.0))); }
  //! \brief Check if this unit has a factor other than 1.
  //! \return true if this unit has a factor other than 1.
  bool has_factor() const { return (!(internal::values_eq(factor_, 1.0))); }
  //! \brief Check if this unit is irreducible or a product of more than
  //!   one irreducible unit.
  //! \return true if the unit is irreducible.
  bool is_irreducible() const { return dim_.is_irreducible(); }
  //! \brief Check if the unit definition is empty.
  //! \return true if there arn't any units, false otherwise.
  bool is_empty() const { return (names_.size() == 0); }
  //! \brief Check if the unit is the dimensionless null unit.
  //! \return true if the unit is null, false otherwise.
  bool is_null() const { return ((names_.size() > 0) && (names_[0].size() == 0)); }
  //! \brief Check if this unit has a specialized difference unit.
  //! \return true if this unit has a specialized difference unit.
  bool has_difference() const { return delta_ == kInactiveDelta; }
  //! \brief Check if this unit is a specialized difference unit.
  //! \return true if this unit is a specialized difference unit.
  bool is_difference() const { return delta_ == kActiveDelta; }
  
  //! \brief Get the conversion factors necessary to convert from this
  //!   unit to another.
  //! \param x Unit to convert to.
  //! \return Two element vector where the first element is the scale factor
  //!   and the second element is the offset.
  std::vector<double> conversion_factor(const GenericUnit& x) const {
    std::vector<double> out;
    RAPIDJSON_ASSERT(dimension() == x.dimension() &&
		     _check_valid() && x._check_valid());
    if (dimension() != x.dimension() || !_check_valid() || !x._check_valid()) {
      out.push_back(1);
      out.push_back(0);
      return out;
    }
    // If offsets == 0
    // x * a1**p = y * b1**p
    // y = x * (a1/b2)**p
    // If power == 1
    // a1 * (x - a2) = b2 * (x - b2)
    // y = [a1 * (x - a2)] / b1 + b2
    // y = (a1 / b1) * x - (a1 / b1) * a2 + b2
    // y = (a1 / b1) * [x - a2 + (b1 / a1) * b2]
    // ratio = a1 / b1
    // y = ratio * [x - (a2 - b2 / ratio)]
    std::vector<double> a = conversion_factor();
    std::vector<double> b = x.conversion_factor();
    double ratio = a[0] / b[0];
    out.push_back(ratio);
    if (is_difference())
      out.push_back(0.0);
    else
      out.push_back(a[1] - b[1] / ratio);
    return out;
  }
  //! \brief Get the conversion factors necessary to convert to/from this
  //!   unit from the base system of units.
  //! \return Two element vector where the first element is the scale factor
  //!   and the second element is the offset.
  std::vector<double> conversion_factor() const {
    std::vector<double> out;
    out.push_back(std::pow(factor_ * prefix_.factor, power_));
    out.push_back(offset_);
    return out;
  }
  //! \brief Find the unit that matches a string in one of the look up tables.
  //! \param str String to find a unit for.
  //! \return true if a unit could be located, false otherwise.
  bool from_table(const std::basic_string<Ch> str);

private:
  std::vector<std::basic_string<Ch> > names_;
  std::vector<std::basic_string<Ch> > abbrs_;
  Dimension dim_;
  double factor_;
  double offset_;
  int delta_;
  double power_;
  GenericUnitPrefix<Encoding> prefix_;

  void _add_plural() {
    size_t N = names_.size();
    std::basic_string<Ch> iname;
    for (size_t i = 0; i < N; i++) {
      iname = names_[i];
      iname.push_back('s');
      names_.push_back(iname);
    }
  }
  bool _check_valid() const {
    return (!(has_power() && has_offset()));
  }
  static const std::basic_string<Ch> get_whitespace() {
    static const Ch s[] = {' ', '\t', '\f', '\v', '\n', '\r', '\0'};
    return std::basic_string<Ch>(s);
  }
  template<typename Ch2>
  static const std::basic_string<Ch2> get_delta() {
    std::string s = "Δ";
    return convert_chars<UTF8<char>, UTF8<Ch2> >(s);
  }
  
  friend class GenericUnits<Encoding>;
  template<typename Ch2, typename Encoding2>
  friend std::basic_ostream<Ch2> & operator << (std::basic_ostream<Ch2>& os, const GenericUnit<Encoding2> &x);
};
//! Output stream operator for GenericUnit.
/*!
  \tparam Ch2 Type used to represent characters in the output stream.
  \tparam Encoding Encoding of the unit strings.
  \param os Output stream.
  \param[in] x Unit to output.
  \returns Output stream.
 */
template<typename Ch2, typename Encoding>
inline std::basic_ostream<Ch2> & operator << (std::basic_ostream<Ch2>& os, const GenericUnit<Encoding> &x) {
  bool has_factor = x.has_factor();
  bool has_power = x.has_power();
  bool has_delta = x.is_difference();
  RAPIDJSON_ASSERT(x.abbrs_.size() > 0);
  if (has_factor && (x.abbrs_.size() > 0) && (x.abbrs_[0].size() == 0))
    os << x.factor_;
  if (has_delta)
    os << x.template get_delta<Ch2>();
  os << x.prefix_;
  if (x.abbrs_.size() > 0)
    os << convert_chars<Encoding,UTF8<Ch2> >(x.abbrs_[0]);
  if (has_power)
    os << '*' << '*' << x.power_;
  return os;
}

//! GenericUnit with UTF8 encoding
typedef GenericUnit<UTF8<char> > Unit;

//! Class to represent a set of units.
/*!
  Container for a set of units (GenericUnit instances) stored in a vector.
  \tparam Encoding Encoding of the units in the set.
 */
template<typename Encoding>
class GenericUnits {
public:
  typedef Encoding EncodingType;    //!< Encoding type from template parameter.
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
  //! \brief Empty constructor.
  GenericUnits() : units_() {}
  //! \brief Initialize from a vector of units.
  //! \param units Vector of units.
  GenericUnits(const std::vector<GenericUnit<Encoding> > units) :
    units_(units) {}
  //! \brief Initialize from a string.
  //! \param str Units string.
  //! \param verbose If true, verbose information is displayed when
  //!   parsing the units string.
  GenericUnits(const std::basic_string<Ch> str, const bool& verbose=false) :
    units_() {
    GenericUnits<Encoding> new_units = parse_units(str.c_str(), str.length(), verbose);
    units_.insert(units_.begin(), new_units.units_.begin(), new_units.units_.end());
  }
  //! \brief Initialize from a string.
  //! \tparam N Number of characters in the string.
  //! \param str Units string.
  //! \param verbose If true, verbose information is displayed when
  //!   parsing the units string.
  template<size_t N>
  GenericUnits(const Ch str[N], const bool& verbose=false) :
    units_() {
    GenericUnits<Encoding> new_units = parse_units(str, N, verbose);
    units_.insert(units_.begin(), new_units.units_.begin(), new_units.units_.end());
  }
  //! \brief Initialize from a string.
  //! \param str Units string. The length is determined by assuming str is
  //!   null terminated.
  //! \param verbose If true, verbose information is displayed when
  //!   parsing the units string.
  GenericUnits(const Ch* str, const bool& verbose=false) :
    units_() {
    GenericUnits<Encoding> new_units = parse_units(str, internal::StrLen(str), verbose);
    units_.insert(units_.begin(), new_units.units_.begin(), new_units.units_.end());
  }
  //! \brief Initialize from a string.
  //! \param str Units string.
  //! \param len Number of characters in str.
  //! \param verbose If true, verbose information is displayed when
  //!   parsing the units string.
  GenericUnits(const Ch* str, const size_t len, const bool& verbose=false) :
    units_() {
    GenericUnits<Encoding> new_units = parse_units(str, len, verbose);
    units_.insert(units_.begin(), new_units.units_.begin(), new_units.units_.end());
  }
  GenericUnits(const GenericUnits<Encoding>& rhs) :
    units_() {
    units_.insert(units_.begin(), rhs.units_.begin(), rhs.units_.end());
  }
  GenericUnits<Encoding>& operator=(const GenericUnits<Encoding>& rhs) {
    units_.clear();
    this->~GenericUnits();
    new (this) GenericUnits<Encoding>(rhs);
    return *this;
  }
  //! \brief Parse a units string.
  //! \param str Units string.
  //! \param len Number of characters in str.
  //! \param verbose If true, verbose information is displayed when
  //!   parsing the units string.
  static GenericUnits parse_units(const Ch* str, const size_t len,
				  const bool verbose=false);  // Forward declaration
  //! \brief Add a unit to the unit set from a string.
  //! \param str Unit string.
  //! \returns true if the unit was added successfully, false otherwise.
  bool add_unit(const std::basic_string<Ch> str) {
#if RAPIDJSON_HAS_CXX11
    units_.emplace_back(str);
#else // RAPIDJSON_HAS_CXX11
    units_.push_back(GenericUnit<Encoding>(str));
#endif // RAPIDJSON_HAS_CXX11
    if ((units_.end() - 1)->is_empty()) {
      units_.clear();
      return false;
    }
    if (((units_.end() - 1)->is_null()) && (units_.size() > 1))
      units_.pop_back();
    return true;
  }
  //! \brief Clear all units.
  void clear() {
    units_.clear();
  }
  //! \brief Display the units instance.
  //! \param os Output stream.
  template<typename Ch2>
  void display(std::basic_ostream<Ch2>& os) const {
    size_t i = 0;
    os << 'G' << 'e' << 'n' << 'e' << 'r' << 'i' << 'c' << 'U' <<
      'n' << 'i' << 't' << 's' << '(' << '[';
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++, i++) {
      if (i != 0) os << ',';
      it->display(os);
    }
    os << ']' << ')';
  }
  //! \brief Get the units as a string.
  //! \return Units string.
  std::basic_string<Ch> str() const {
    std::basic_stringstream<Ch> ss;
    ss << *this;
    return ss.str();
  }
  //! \brief Create a copy of the units with a different encoding.
  //! \tparam DestEncoding Encoding that the copy should use.
  //! \return Copy w/ DestEncoding.
  template<typename DestEncoding>
  GenericUnits<DestEncoding> transcode() const {
    GenericUnits<DestEncoding> out;
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++)
      out.units_.push_back(it->template transcode<DestEncoding>());
    return out;
  }
  //! \brief Get the dimensions of the units.
  //! \returns Consolidated dimensions of the units.
  Dimension dimension() const {
    Dimension out(dimensions::dimensionless);
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++)
      out = out * it->dimension();
    return out;
  }
  //! \brief Determine if the units are dimensionless.
  //! \return true if the units are dimensionless, false otherwise.
  bool is_dimensionless() const {
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++)
      if (!it->is_dimensionless())
	return false;
    return true;
  }
  //! \brief Determine if the units are null.
  //! \return true if the units are null.
  bool is_null() const {
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++)
      if (!it->is_null())
	return false;
    return true;
  }
  //! \brief Determine if another set of units are compatible and share the
  //!   same dimensions.
  //! \param x Units for comparison.
  //! \return true if the units are compatible, false otherwise.
  bool is_compatible(const GenericUnits& x) const {
    return (dimension() == x.dimension());
  }
  //! \brief Determine if this set of units is identical to another.
  //! \param x Units for comparison.
  //! \return true if the units are identical.
  bool operator==(const GenericUnits& x) const {
    if (units_.size() != x.units_.size())
      return false;
    for (size_t i = 0; i < units_.size(); i++) {
      size_t j = 0;
      for (j = 0; j < x.units_.size(); j++) {
	if (units_[i] == x.units_[j])
	  break;
      }
      if (j >= x.units_.size())
	return false;
    }
    return true;
  }
  //! \brief Determine if this set of units is not identical to another.
  //! \param x Units for comparison.
  //! \return true if the units are not identical.
  bool operator!=(const GenericUnits& x) const { return (!(*this == x)); }
  //! \brief Perform multiplication with another set of units in place.
  //! \param x Units for multiplication.
  //! \return Multiplied units.
  GenericUnits& operator*=(const GenericUnits& x) {
    double factor = 1.0;
    size_t old_size = units_.size();
    std::set<size_t> idx_remove;
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it2 = x.units_.begin(); it2 != x.units_.end(); it2++) {
      if (it2->has_offset())
	continue;
      size_t i = 0;
      for (i = 0; i < old_size; i++) {
	if (!units_[i].has_offset() && !it2->has_offset() && it2->dim_ == units_[i].dim_)
	  break;
      }
      if (i < old_size) {
	// (a1*ap*x)**a2 * (b1*bp*x)**b2
	//     = (a1**a2)*(b1**b2)*(ap**a2)*(bp**b2)*(x**(a2+b2))
	//     = (a1**a2)*(b1**b2)*(ap**-b2)*(bp**b2)*((ap*x)**(a2+b2))
	//     = (a1**-b2)*(b1**b2)*(ap**-b2)*(bp**b2)*((a1*ap*x)**(a2+b2))
	//     = (a1*ap)**-b2 * (b1*bp)**b2 * (a1*ap*x)**(a2+b2)
	if (it2->is_null()) {
	  factor *= std::pow(it2->factor_, it2->power_);
	  idx_remove.insert(i);
	} else {
	  double new_power = units_[i].power_ + it2->power_;
	  factor *= std::pow((it2->factor_ * it2->prefix_.factor) /
			     (units_[i].factor_ * units_[i].prefix_.factor),
			     it2->power_);
	  if (internal::values_eq(new_power, 0))
	    idx_remove.insert(i);
	  units_[i].power_ = new_power;
	}
      } else {
#if RAPIDJSON_HAS_CXX11
	units_.emplace_back(*it2);
#else // RAPIDJSON_HAS_CXX11
	units_.push_back(GenericUnit<Encoding>(*it2));
#endif // RAPIDJSON_HAS_CXX11
      }
    }
    for (typename std::set<size_t>::reverse_iterator it = idx_remove.rbegin(); it != idx_remove.rend(); it++)
      units_.erase(units_.begin() + (int)(*it));
    typename std::vector<GenericUnit<Encoding> >::iterator nodim = units_.end();
    for (typename std::vector<GenericUnit<Encoding> >::iterator it = units_.begin(); it != units_.end(); it++) {
      if (it->is_null()) {
	nodim = it;
	break;
      }
    }
    if (nodim == units_.end()) {
      std::basic_string<Ch> empty;
#if RAPIDJSON_HAS_CXX11
      units_.emplace_back(empty);
#else // RAPIDJSON_HAS_CXX11
      units_.push_back(GenericUnit<Encoding>(empty));
#endif // RAPIDJSON_HAS_CXX11
      nodim = (units_.end() - 1);
    }
    RAPIDJSON_ASSERT(internal::values_eq(nodim->power_, 1.0));
    nodim->factor_ *= factor;
    if (!nodim->has_factor() && (units_.size() > 1))
      units_.erase(nodim);
    RAPIDJSON_ASSERT(_check_valid());
    return *this;
  }
  /*!
    Report whether or not any of the units have a non-unity scale factor.
    \returns true if there is a non-unity scale factor, false otherwise.
   */
  bool has_factor() const {
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++) {
      if (!(it->is_null())) continue;
      if (!internal::values_eq(it->factor_, 1.0))
	return true;
    }
    return false;
  }
  /*!
    Pull out all of the scale factors from the units that are
    dimensionless, removing them. If only dimensionless units are present
    a single dimensionless unit will remain with a unitary (1.0) factor.
    \returns Product of all dimensionless scale factors.
   */
  double pull_factor() {
    double factor = 1.0;
    std::vector<size_t> idx_remove;
    size_t i = 0;
    for (typename std::vector<GenericUnit<Encoding> >::iterator it = units_.begin(); it != units_.end(); it++, i++) {
      if (!(it->is_null())) continue;
      factor *= std::pow(it->factor_, it->power_);
      it->factor_ = 1.0;
      if (units_.size() > 1)
	idx_remove.push_back(i);
    }
    for (typename std::vector<size_t>::reverse_iterator it = idx_remove.rbegin(); it != idx_remove.rend(); it++)
      units_.erase(units_.begin() + (int)(*it));
    return factor;
  }
  /*!
    Add a dimensionless scale factor to the unit set.
    \param factor Scale factor to add.
   */
  void add_factor(double factor) {
    std::basic_string<Ch> empty;
#if RAPIDJSON_HAS_CXX11
    units_.emplace_back(empty);
#else // RAPIDJSON_HAS_CXX11
    units_.push_back(GenericUnit<Encoding>(empty));
#endif // RAPIDJSON_HAS_CXX11
    units_[units_.size() - 1].factor_ = factor;
  }
  //! \brief Perform multiplication with another set of units.
  //! \param x Units for multiplication.
  //! \return Multiplied units.
  friend GenericUnits operator*(GenericUnits lhs, const GenericUnits& rhs) {
    lhs *= rhs;
    return lhs;
  }
  //! \brief Perform division with another set of units in place
  //! \param x Units for division.
  //! \return Divided units.
  GenericUnits& operator/=(const GenericUnits& x) {
    return *this *= x.pow(-1);
  }
  //! \brief Perform division with another set of units.
  //! \param x Units for division.
  //! \return Divided units.
  friend GenericUnits operator/(GenericUnits lhs, const GenericUnits& rhs) {
    lhs /= rhs;
    return lhs;
  }
  //! \brief Raise these units to a power without creating a new instance.
  //! \param x Power.
  template<typename T>
  void pow_inplace(const T x) {
    for (typename std::vector<GenericUnit<Encoding> >::iterator it = units_.begin(); it != units_.end(); it++)
      it->pow_inplace(x);
  }
  //! \brief Raise these units to a power.
  //! \param x Power.
  //! \return Resulting units.
  template<typename T>
  GenericUnits pow(const T x) const {
    GenericUnits out(*this);
    out.pow_inplace(x);
    return out;
  }
  //! \brief Check if there are any units in the instance.
  //! \return true if there arn't any units, false otherwise.
  bool is_empty() const { return (units_.size() == 0); }
  //! \brief Check if the units have an offset.
  //! \return true if the units have an offset, false otherwise.
  bool has_offset() const {
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++)
      if (it->has_offset())
	return true;
    return false;
  }
  //! \brief Get the number of units with dimensions.
  //! \return Number of units with dimensions.
  size_t size() const {
    size_t out = 0;
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++)
      if (!it->is_null())
	out++;
    return out;
  }
  //! \brief Check if the units have a specialized difference unit.
  //! \return true if the units have a difference unit.
  bool has_difference() const {
    if (units_.size() != 1)
      return false;
    return units_[0].has_difference();
  }
  //! \brief Check if the units are a specialized difference unit.
  //! \return true if the units are a difference unit.
  bool is_difference() const {
    if (units_.size() != 1)
      return false;
    return units_[0].is_difference();
  }
  //! \brief Get the difference flag.
  //! \return Flag specifying if the unit has or is a difference unit.
  int get_delta() const {
    if (units_.size() != 1)
      return kNoDelta;
    return units_[0].delta_;
  }
  //! \brief Set the difference flag.
  //! \param delta New difference flag.
  void set_delta(int delta) {
    RAPIDJSON_ASSERT(has_difference() || is_difference());
    if ((has_difference() || is_difference()) && delta > 0)
      units_[0].delta_ = delta;
  }
  //! \brief Determine the conversion factors necessary to convert quantities
  //!   with these units to another set of units.
  //! \param x Units that conversion factors should convert to.
  //! \return Array of conversion factors where the first element is the
  //!   factor that values should be multiplied by and the second element is
  //!   the offset between the zero points in this and x.
  std::vector<double> conversion_factor(const GenericUnits& x) const {
    if ((x.units_.size() == 1) && (units_.size() == 1))
      return units_[0].conversion_factor(x.units_[0]);
    std::vector<double> out;
    bool singular = (x.size() == 1 && size() == 1);
    RAPIDJSON_ASSERT(dimension() == x.dimension() &&
		     (is_difference() || !has_offset() || singular));
    if (!(dimension() == x.dimension() &&
	  (is_difference() || !has_offset() || singular))) {
      out.push_back(1);
      out.push_back(0);
      return out;
    }
    out.push_back(1.0);
    out.push_back(0.0);
    typename std::vector<GenericUnit<Encoding> >::const_iterator x_it, y_it;
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++) {
      out[0] = out[0] * it->conversion_factor()[0];
      if (!it->is_null())
	x_it = it;
    }
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = x.units_.begin(); it != x.units_.end(); it++) {
      out[0] = out[0] / it->conversion_factor()[0];
      if (!it->is_null())
	y_it = it;
    }
    if (singular)
      out[1] = x_it->conversion_factor(*y_it)[1];
    return out;
  }
  //! \brief Determine the conversion factors necessary to convert quantities
  //!   with these units to another set of units and convert these units to
  //!   the other set of units in place.
  //! \param x Units that conversion factors should convert to.
  //! \return Array of conversion factors where the first element is the
  //!   factor that values should be multiplied by and the second element is
  //!   the offset between the zero points in this and x.
  std::vector<double> convert_to(const GenericUnits& x) {
    int delta = get_delta();
    std::vector<double> out = conversion_factor(x);
    units_.clear();
    units_.insert(units_.begin(), x.units_.begin(), x.units_.end());
    // units_ = x.units_;
    if (delta)
      set_delta(delta);
    return out;
  }
  //! \brief Find the unit in the current unit set that matches a particular
  //!   dimension at its base (ignoring powers).
  //! \param d Dimension to find.
  //! \return Pointer to the unit that matches or NULL if there is not a match.
  const GenericUnit<Encoding>* find_dimension(const Dimension& d) const {
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++) {
      if (it->dim_ == d)
	return &(*it);
    }
    return NULL;
  }
  //! \brief Get the equivalent of this set of units in the units system
  //!   used by another set of units.
  //! \param x Unit system to get units in.
  //! \return Equivalent units in the x unit system.
  GenericUnits<Encoding> as_units_system(const GenericUnits& x) const {
    GenericUnits<Encoding> new_Units;
    as_units_system(x, new_Units);
    return new_Units;
  }
  //! \brief Get the equivalent of this set of units in the units system
  //!   used by another set of units.
  //! \param x Unit system to get units in.
  //! \param[out] dest Destination for equivalent units in the x unit
  //!   system.
  void as_units_system(const GenericUnits& x, GenericUnits& dest) const {
    std::vector<GenericUnit<Encoding> > new_units;
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++) {
      if (it->is_null())
	continue;
      const GenericUnit<Encoding>* x_dim = x.find_dimension(it->dim_);
      if (x_dim == NULL) {
	new_units.push_back(*it);
      } else {
	GenericUnit<Encoding> tmp(*x_dim);
	tmp.power_ = it->power_;
	tmp.delta_ = it->delta_;
	new_units.push_back(tmp);
      }
    }
    dest = GenericUnits<Encoding>(new_units);
  }
  
  //! \brief Convert units to the system used by another set of units in place
  //!   and determine the conversion factors necessary to convert quantities
  //!   with these units to the new unit system.
  //! \param x Unit system to convert to.
  //! \return Array of conversion factors where the first element is the
  //!   factor that values should be multiplied by and the second element is
  //!   the offset between the zero points in this and x.
  std::vector<double> convert_to_units_system(const GenericUnits& x) {
    if (dimension() == x.dimension())
      return convert_to(x);
    GenericUnits<Encoding> new_Units = as_units_system(x);
    return convert_to(new_Units);
  }
private:
  std::vector<GenericUnit<Encoding> > units_;

  bool _check_valid() const {
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = units_.begin(); it != units_.end(); it++) {
      if (!it->_check_valid())
	return false;
    }
    return (!(has_offset() && size() > 1));
  }
  
  template<typename Ch2, typename Enc2>
  friend std::basic_ostream<Ch2> & operator << (std::basic_ostream<Ch2> &os, const GenericUnits<Enc2> &x);
};
//! Output stream operator for GenericUnits.
/*!
  \tparam Ch2 Type used to represent characters in the output stream.
  \tparam Encoding Encoding of the unit strings.
  \param os Output stream.
  \param[in] x Unit set to output.
  \returns Output stream.
 */
template<typename Ch2, typename Encoding>
inline std::basic_ostream<Ch2> & operator << (std::basic_ostream<Ch2> &os, const GenericUnits<Encoding> &x) {
    size_t i = 0;
    size_t N = x.units_.size();
    for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = x.units_.begin(); it != x.units_.end(); it++, i++) {
      if (i != 0) os << '*';
      if ((N > 1) && it->has_power())
	os << '(';
      os << *it;
      if ((N > 1) && it->has_power())
	os << ')';
    }
    return os;
}

//! Multiplication operator for two units.
/*!
  Perform multiplication of two units, creating a unit set.
  \param[in] a First unit factor in multiplication.
  \param[in] b Second unit factor in multiplication.
  \returns Result of unit multiplication.
 */
template<typename Encoding>
GenericUnits<Encoding> operator*(const GenericUnit<Encoding>& a, const GenericUnit<Encoding>& b) {
  std::vector<GenericUnit<Encoding> > units_a, units_b;
  units_a.push_back(a);
  units_b.push_back(b);
  GenericUnits<Encoding> Units_a = GenericUnits<Encoding>(units_a);
  GenericUnits<Encoding> Units_b = GenericUnits<Encoding>(units_b);
  Units_a *= Units_b;
  return Units_a;
}
//! Division operator for two units.
/*!
  Perform division of two units, creating a unit set.
  \param[in] a Numerator unit in division.
  \param[in] b Denominator unit in division.
  \returns Result of unit division.
 */
template<typename Encoding>
GenericUnits<Encoding> operator/(const GenericUnit<Encoding>& a, const GenericUnit<Encoding>& b) {
  return a * b.pow(-1);
}

//! GenericUnits with UTF8 encoding
typedef GenericUnits<UTF8<char> > Units;

//! Macro for constructing Unit entries for unit LUTs.
#define PACK_UNIT(...) PACK_LUT(Unit, (__VA_ARGS__))
//! Macro for packing multiple names/abbreviations for unit construction
#define VSTR(...) pack_strings<char>(__VA_ARGS__, (char*)NULL)

  /*!
    \brief Cached lookup table containing the base unit set (MKS).

    This unit set can have SI prefixes.
   */
  static CachedLUT<Unit> _base_units (
      PACK_UNIT(VSTR("meter", "metre"), VSTR("m"), dimensions::length),
      PACK_UNIT(VSTR("gram", "gramme"), VSTR("g"), dimensions::mass, 1.0e-3),
      PACK_UNIT("second", "s", dimensions::time),
      PACK_UNIT(VSTR("ampere", "amp", "Amp"), VSTR("A"), dimensions::current),
      PACK_UNIT(VSTR("kelvin", "degree_kelvin"), VSTR("K", "degK"), dimensions::temperature, 1, 0, 1),
      PACK_UNIT("mole", "mol", dimensions::number, 1.0 / constants::amu_grams),
      PACK_UNIT("candela", "cd", dimensions::luminous_intensity),
      PACK_UNIT("radian", "rad", dimensions::angle),
      (void*)NULL
    );

  /*!
    \brief Cached lookup table containing units that can be given SI unit
      prefixes.
   */
  static CachedLUT<Unit> _prefixable_units (
#if RAPIDJSON_HAS_CXX11
      _base_units.template get<Unit>(),
#else // RAPIDJSON_HAS_CXX11
      _base_units.get<Unit>(),
#endif // RAPIDJSON_HAS_CXX11
      // cgs
      PACK_UNIT("dyne", "dyn", dimensions::force, 1.0e-5),
      PACK_UNIT("erg", "erg", dimensions::energy, 1.0e-7),
      PACK_UNIT("barye", "Ba", dimensions::pressure, 0.1),
      PACK_UNIT("gauss", "G", dimensions::magnetic_field_cgs, pow(0.1, 0.5)),
      PACK_UNIT(VSTR("statcoulomb", "esu", "ESU", "electrostatic_unit"), VSTR("statC"), dimensions::charge_cgs, pow(1.0e-3, 1.5)),
      PACK_UNIT("statampere", "statA", dimensions::current_cgs, pow(1.0e-3, 1.5)),
      PACK_UNIT("statvolt", "statV", dimensions::electric_potential_cgs, 0.1 * pow(1.0e-3, 1.5)),
      PACK_UNIT("statohm", "statohm", dimensions::resistance_cgs, 100.0),
      PACK_UNIT("maxwell", "Mx", dimensions::magnetic_flux_cgs, pow(1.0e-3, 1.5)),
      // SI
      PACK_UNIT("joule", "J", dimensions::energy),
      PACK_UNIT("watt", "W", dimensions::power),
      PACK_UNIT("hertz", "Hz", dimensions::rate),
      PACK_UNIT("newton", "N", dimensions::force),
      PACK_UNIT("coulomb", "C", dimensions::charge),
      PACK_UNIT("tesla", "T", dimensions::magnetic_field),
      PACK_UNIT("pascal", "Pa", dimensions::pressure),
      PACK_UNIT("bar", "bar", dimensions::pressure, 1.0e5),
      PACK_UNIT("volt", "V", dimensions::electric_potential),
      PACK_UNIT("farad", "F", dimensions::capacitance),
      PACK_UNIT("henry", "H", dimensions::inductance),
      PACK_UNIT(VSTR("ohm", "Ohm"), VSTR("Ω"), dimensions::resistance),
      PACK_UNIT("weber", "Wb", dimensions::magnetic_flux),
      PACK_UNIT("lumen", "lm", dimensions::luminous_flux),
      PACK_UNIT("lux", "lx", dimensions::luminous_flux / dimensions::area),
      PACK_UNIT(VSTR("celcius", "degree_celsius", "degree_Celsius", "celsius"), VSTR("degC", "°C"), dimensions::temperature, 1.0, constants::celcius_zero_kelvin, 1),
      // other
      PACK_UNIT("calorie", "cal", dimensions::energy, 4.184),
      PACK_UNIT("year", "yr", dimensions::time, constants::sec_per_year),
      PACK_UNIT("parsec", "pc", dimensions::length, constants::m_per_pc),
      PACK_UNIT("electronvolt", "eV", dimensions::energy, constants::J_per_eV),
      PACK_UNIT(VSTR("jansky"), VSTR("J", "j"), dimensions::specific_flux, constants::jansky_mks),
      PACK_UNIT("sievert", "Sv", dimensions::specific_energy),
      PACK_UNIT("molar", "M", dimensions::number_density, 100.0 / constants::amu_grams),
      (void*)NULL
    );
  
  /*!
    \brief Cached lookup table containing units that CANNOT be given SI
      unit prefixes. This includes imperial units and some constants.
   */
  static CachedLUT<Unit> _unprefixable_units(
    // Imperial units
    PACK_UNIT(VSTR("mil", "thou", "thousandth"), VSTR("mil"), dimensions::length, 1.0e-3 * constants::m_per_inch),
    PACK_UNIT("incl", "incl", dimensions::length, constants::m_per_inch),
    PACK_UNIT("feet", "ft", dimensions::length, constants::m_per_ft),
    PACK_UNIT("yard", "yd", dimensions::length, 0.9144),
    PACK_UNIT("mile", "mi", dimensions::length, 1609.344),
    PACK_UNIT("furlong", "fur", dimensions::length, constants::m_per_ft * 660.0),
    PACK_UNIT(VSTR("farenheit", "degree_fahrenheit", "degree_Fahrenheit"), VSTR("degF", "°F"), dimensions::temperature, constants::kelvin_per_rankine, constants::farenheit_zero_kelvin, 1),
    PACK_UNIT(VSTR("rankine", "degree_rankine"), VSTR("degR"), dimensions::temperature, constants::kelvin_per_rankine, 0, 1),
    PACK_UNIT("pound_force", "lbf", dimensions::force, constants::kg_per_pound * constants::standard_gravity_m_per_s2),
    PACK_UNIT(VSTR("pound", "pound_mass"), VSTR("lb", "lbm"), dimensions::mass, constants::kg_per_pound),
    PACK_UNIT("atmosphere", "atm", dimensions::pressure, constants::pascal_per_atm),
    PACK_UNIT("horsepower", "hp", dimensions::power, constants::watt_per_horsepower),
    PACK_UNIT("ounce", "oz", dimensions::mass, constants::kg_per_pound / 16.0),
    PACK_UNIT("ton", "ton", dimensions::mass, constants::kg_per_pound * 2000.0),
    PACK_UNIT("slug", "slug", dimensions::mass, constants::kg_per_pound * constants::standard_gravity_m_per_s2 / constants::m_per_ft),
    PACK_UNIT(VSTR("BTU", "british_thermal_unit"), VSTR("BTU"), dimensions::energy, 1055.0559),
    PACK_UNIT("pounds_per_square_inch", "psi", dimensions::pressure, constants::kg_per_pound * constants::standard_gravity_m_per_s2 / pow(constants::m_per_inch, 2)),
    PACK_UNIT("smoot", "smoot", dimensions::length, 1.7018),
    PACK_UNIT("percent", "%", dimensions::dimensionless, 0.01),
    PACK_UNIT("fraction", "100%", dimensions::dimensionless, 1.0),
    PACK_UNIT("minute", "min", dimensions::time, constants::sec_per_min),
    PACK_UNIT(VSTR("hour"), VSTR("hr", "h", "hrs"), dimensions::time, constants::sec_per_hr),
    PACK_UNIT(VSTR("day"), VSTR("day", "d"), dimensions::time, constants::sec_per_day),
    // Astronomy units
    PACK_UNIT("c", "c", dimensions::velocity, constants::speed_of_light_m_per_s),
    PACK_UNIT(VSTR("solar_mass", "solMass", "mass_sun"), VSTR("Msun", "Msol", "msun", "m_sun", "M_sun", "m_Sun"), dimensions::mass, constants::mass_sun_kg),
    PACK_UNIT(VSTR("solar_radius", "solRadius"), VSTR("Rsun", "Rsol", "rsun", "r_sun", "R_sun", "r_Sun"), dimensions::length, constants::m_per_rsun),
    PACK_UNIT(VSTR("solar_luminosity", "solLumin"), VSTR("Lsun", "Lsol", "lsun", "l_sun", "L_sun", "l_Sun"), dimensions::power, constants::luminosity_sun_watts),
    PACK_UNIT(VSTR("solar_temperature", "solTemperature"), VSTR("Tsun", "Tsol", "tsun", "t_sun", "T_sun", "t_Sun"), dimensions::temperature, constants::temp_sun_kelvin, 0, 1),
    PACK_UNIT(VSTR("solar_metallicity", "solMetallicity"), VSTR("Zsun", "Zsol", "zsun", "z_sun", "Z_sun", "z_Sun"), dimensions::dimensionless, constants::metallicity_sun),
    PACK_UNIT(VSTR("jupiter_mass"), VSTR("Mjup", "m_jup"), dimensions::mass, constants::mass_jupiter_kg),
    PACK_UNIT(VSTR("jupiter_radius"), VSTR("Rjup", "r_jup"), dimensions::length, constants::m_per_rjup),
    PACK_UNIT(VSTR("earth_mass"), VSTR("Mearth", "m_earth"), dimensions::mass, constants::mass_earth_kg),
    PACK_UNIT(VSTR("earth_radius"), VSTR("Rearth", "r_earth"), dimensions::length, constants::m_per_rearth),
    PACK_UNIT(VSTR("astronomical_unit"), VSTR("AU", "au"), dimensions::length, constants::m_per_au),
    PACK_UNIT("light_year", "ly", dimensions::length, constants::m_per_ly),
    PACK_UNIT("degree", "deg", dimensions::angle, M_PI / 180.0),
    PACK_UNIT("arcmin", "arcmin", dimensions::angle, M_PI / 10800.0),
    PACK_UNIT("arcsec", "arcsec", dimensions::angle, M_PI / 648000.0),
    PACK_UNIT("miliarcsec", "mas", dimensions::angle, M_PI / 648000000.0),
    PACK_UNIT("hourangle", "hourangle", dimensions::angle, M_PI / 12.0),
    PACK_UNIT("steradian", "sr", dimensions::solid_angle),
    PACK_UNIT(VSTR("latitude", "degree_latitude"), VSTR("lat"), dimensions::angle, -M_PI / 180.0, 90.0),
    PACK_UNIT(VSTR("longitude", "degree_longitude"), VSTR("long"), dimensions::angle, M_PI / 180.0, -180.0),
    // Physics
    PACK_UNIT("amu", "amu", dimensions::mass, constants::amu_kg),
    PACK_UNIT("angstrom", "Å", dimensions::length, constants::m_per_ang),
    PACK_UNIT("counts", "counts", dimensions::number),
    PACK_UNIT("photons", "photons", dimensions::number),
    PACK_UNIT("me", "me", dimensions::mass, constants::mass_electron_kg),
    PACK_UNIT("mp", "mp", dimensions::mass, constants::mass_hydrogen_kg),
    PACK_UNIT("rayleigh", "rayleigh", dimensions::count_intensity, 2.5e9 / M_PI),
    PACK_UNIT("lambert", "lambert", dimensions::luminance, 1.0e4 / M_PI),
    PACK_UNIT("nit", "nt", dimensions::luminance),
    // Planck
    PACK_UNIT("m_pl", "m_pl", dimensions::mass, constants::planck_mass_kg),
    PACK_UNIT("l_pl", "l_pl", dimensions::length, constants::planck_length_m),
    PACK_UNIT("t_pl", "t_pl", dimensions::time, constants::planck_time_s),
    PACK_UNIT("T_pl", "T_pl", dimensions::temperature, constants::planck_temperature_K),
    PACK_UNIT("q_pl", "q_pl", dimensions::charge, constants::planck_charge_C),
    PACK_UNIT("E_pl", "E_pl", dimensions::energy, constants::planck_energy_J),
    // Geometrized
    PACK_UNIT("m_geom", "m_geom", dimensions::mass, constants::mass_sun_kg),
    PACK_UNIT("l_geom", "l_geom", dimensions::length, constants::newton_mks * constants::mass_sun_kg / pow(constants::speed_of_light_m_per_s, 2)),
    PACK_UNIT("t_geom", "t_geom", dimensions::time, constants::newton_mks * constants::mass_sun_kg / pow(constants::speed_of_light_m_per_s, 3)),
    // Logarithmic
    // PACK_UNIT("bel", "B", dimensions::logarithmic, neper_per_bel),
    // PACK_UNIT("neper", "Np", dimensions::logarithmic),
    // misc
    PACK_UNIT("acre", "ac", dimensions::area, 4046.86),
    PACK_UNIT("are", "a", dimensions::area, 100.0),
    PACK_UNIT("hectare", "ha", dimensions::area, 10000.0),
    PACK_UNIT(VSTR(""), VSTR("", "n/a", "dimensionless"), dimensions::dimensionless, 1.0),
    (void*)NULL
  );

#undef PACK_UNIT
#undef VSTR

template<typename Encoding>
bool GenericUnit<Encoding>::from_table(const std::basic_string<typename Encoding::Ch> str) {
  size_t idx_beg, idx_end;
  std::basic_string<Ch> whitespace = get_whitespace();
  idx_beg = str.find_first_not_of(whitespace);
  idx_end = str.find_last_not_of(whitespace);
  if (idx_end >= str.size())
    idx_end = str.size() - 1;
  std::basic_string<Ch> substr;
  bool delta = false;
  if (str.size() == 0) {
    substr = str;
  } else {
    std::basic_string<Ch> delta_str = get_delta<Ch>();
    std::size_t found = str.find(delta_str, idx_beg);
    if (found != std::string::npos) {
      delta = true;
      idx_beg += delta_str.size();
    }
    substr = str.substr(idx_beg, idx_end + 1);
  }
  
  std::vector<const GenericUnit<Encoding>*> possibilities;
  const std::vector<GenericUnit<Encoding> >* prefix_units = _prefixable_units.template get<GenericUnit<Encoding> >();
  for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = prefix_units->begin(); it != prefix_units->end(); it++) {
    if (it->matches(substr))
      return from_table(*it, delta);
    it->prefix_matches(substr, possibilities);
  }
  const std::vector<GenericUnit<Encoding> >* unprefix_units = _unprefixable_units.template get<GenericUnit<Encoding> >();
  for (typename std::vector<GenericUnit<Encoding> >::const_iterator it = unprefix_units->begin(); it != unprefix_units->end(); it++)
    if (it->matches(substr))
      return from_table(*it, delta);
  if (possibilities.size() > 0) {
    const std::vector<GenericUnitPrefix<Encoding> >* prefixes = _unit_prefixes.template get<GenericUnitPrefix<Encoding> >();
    for (typename std::vector<const GenericUnit<Encoding>*>::const_iterator it = possibilities.begin(); it != possibilities.end(); it++)
      for (typename std::vector<GenericUnitPrefix<Encoding> >::const_iterator p = prefixes->begin(); p != prefixes->end(); p++)
	if ((*it)->matches(substr, *p))
	  return from_table(**it, delta, *p);
  }
  return false; // GCOVR_EXCL_LINE
}

namespace parser {

//! Identifier for what a token represents.
enum TokenType {
  kWhitespaceToken = 0,  //!< Spaces, newlines, tabs
  kOperatorToken = 1,    //!< Arithmetic operator (e.g. *, /, **)
  kWordToken = 2,        //!< Unit name/abbreviation
  kGroupToken = 3        //!< Group of units.
};
//! Identifier for how a token was finalized.
enum TokenFinalization {
  kTokenFinalizeNull,    //!< Token has not been finalized.
  kTokenFinalizeAlways,  //!<
  kTokenFinalizeBracket, //!<
  kTokenFinalizeSpace,   //!<
  kTokenFinalizeValue,   //!<
  kTokenFinalizeNext,    //!<
  kTokenFinalizeGroup,   //!<
  kTokenFinalizeFinal    //!<
};

//! Forward declaration
template<typename Encoding>
class GroupToken;


//! Base class for tokens when parsing unit strings.
/*!
  Provides a way to store information extracted from unit strings.
  \tparam Encoding Token encoding.
 */
template<typename Encoding>
class TokenBase {
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
private:
  TokenBase(const TokenBase<Encoding>& rhs);
public:
  //! Constructor from token type & parent.
  /*!
    \param[in] t0 Type of token being created.
    \param parent0 Parent token containing this sub-token.
   */
  TokenBase(const TokenType t0, TokenBase *parent0=NULL) : t(t0), units(), finalized(kTokenFinalizeNull), parent(parent0), value_(0.0), errorFlag(false) {}
  //! Destructor
  virtual ~TokenBase() {}
  //! Get the current token.
  virtual TokenBase<Encoding>* current_token() { return this; }
  //! Finalize this token if it has not been finalized.
  /*!
    \param x Identifier for how this token should be finalized.
    \returns Units represented by this token.
   */
  virtual GenericUnits<Encoding> finalize(TokenFinalization x) {
    if (!finalized)
      finalized = x;
    return units;
  }
  //! Reset this token by clearing the units and reseting the state
  void reset() {
    finalized = kTokenFinalizeNull;
    units.clear();
    value_ = 0.0;
  }
  //! Get the numeric value represented by the token
  /*!
    \pre is_numeric()
    \returns Numeric value for the token.
   */
  double value() {
    RAPIDJSON_ASSERT(is_numeric());
    finalize(kTokenFinalizeValue);
    return value_;
  }
  //! Check if the token represents a number.
  virtual bool is_numeric() { return false; }
  //! Set the error flag for this token and its parent token
  virtual void set_error() {
    errorFlag = true;
    if (parent)
      parent->set_error();
  }
  //! Append a character to this token.
  /*!
    \param c Character to append.
   */
  virtual void append(const Ch c) = 0;
  //! Output this token to an output stream.
  /*!
    \param os Output stream.
    \returns Output stream.
   */
  virtual std::ostream & display(std::ostream &os) const = 0;
  //! Copy assignment operator.
  TokenBase<Encoding>& operator=(const TokenBase<Encoding>& other);
  TokenType t;                  //!< Token type.
  GenericUnits<Encoding> units; //!< Units represented by this token
  TokenFinalization finalized;  //!< Finalization state.
  TokenBase<Encoding> *parent;  //!< Parent token.
  double value_;                //!< Numeric token value.
  bool errorFlag;               //!< Error state.
  template<typename Encoding2>
  friend std::ostream & operator << (std::ostream &os, const TokenBase<Encoding2>* x);
};
//! Output stream operator for TokenBase
/*!
  \tparam Encoding2 Encoding of the token string.
  \param os Output stream.
  \param[in] x Token to output.
  \returns Output stream.
 */
template<typename Encoding2>
inline std::ostream & operator << (std::ostream &os, const TokenBase<Encoding2>* x) {
  return x->display(os);
}

//! Token class for unit operators.
/*!
  Defined by arithmetic operations between units and/or scalars.
  \tparam Encoding Token encoding.
 */
template<typename Encoding>
class OperatorToken : public TokenBase<Encoding> {
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
public:
  //! Constructor
  /*!
    \param[in] op0 First character representing the operator.
    \param parent0 Parent token containing this sub-token.
   */
  OperatorToken(const Ch op0, TokenBase<Encoding> *parent0=NULL) : TokenBase<Encoding>(kOperatorToken, parent0), op(op0) { this->finalize(kTokenFinalizeAlways); }
  //! \copydoc TokenBase::append
  void append(const Ch c) OVERRIDE_CXX11 { RAPIDJSON_ASSERT(!c); (void)c; } // GCOVR_EXCL_LINE
  //! Perform the token operation between two sets of units.
  /*!
    This only applies to multiplication or division operations.
    \param[in] a Unit set to the left of the operator.
    \param[in] b Unit set to the right of the operator.
    \returns Resulting unit set.
    \pre (op == '*') || (op == '/')
   */
  GenericUnits<Encoding> operate(const GenericUnits<Encoding>& a, const GenericUnits<Encoding>& b) {
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
  //! Perform the token operation between a unit set and a scalar.
  /*!
    \param[in] a Unit set to the left of the operator.
    \param[in] b Scalar to the right of the operator.
    \returns Resulting unit set.
   */
  GenericUnits<Encoding> operate(const GenericUnits<Encoding>& a, const double& b) {
    RAPIDJSON_ASSERT(op == '^');
    return a.pow(b);
  }
  //! Perform the token operation between two scalars.
  /*!
    \param[in] a Scalar to the left of the operator.
    \param[in] b Scalar to the right of the operator.
    \returns Resulting scalar.
   */
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
  //! \copydoc TokenBase::is_numeric
  bool is_numeric() OVERRIDE_CXX11 { return true; }
  //!< Check if this token describes a exponent operator.
  bool is_exp() { return (op == '^'); }
  //!< Check if this token matches any of the provided operators.
  /*!
    \param[in] ops Operators to check this token against.
    \returns true if this token matches any of the operators in op.
   */
  bool matches(const std::vector<char> ops) {
    for (std::vector<char>::const_iterator iop = ops.begin(); iop != ops.end(); iop++)
      if (*iop == op)
	return true;
    return false;
  }
  //! \copydoc TokenBase::display
  std::ostream & display(std::ostream &os) const OVERRIDE_CXX11 {
    os << "OperatorToken(" << op << ")";
    return os;
  }
  Ch op;
};

//! Forward declaration
template<typename Encoding>
class NumberToken;
  
//! Token class for unit names/abbrieviations.
/*!
  Defined by unit names/abbrieviations.
  \tparam Encoding Token encoding.
 */
template<typename Encoding>
class WordToken : public TokenBase<Encoding> {
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
public:
  //! Constructor
  /*!
    \param[in] c First character representing the token.
    \param parent0 Parent token containing this sub-token.
   */
  WordToken(const Ch c, TokenBase<Encoding> *parent0=NULL) : TokenBase<Encoding>(kWordToken, parent0), word() {
    word.push_back(c);
  }
  //! \copydoc TokenBase::append
  void append(const Ch c) OVERRIDE_CXX11 {
    word.push_back(c);
  }
  //! \copydoc TokenBase::finalize
  GenericUnits<Encoding> finalize(TokenFinalization x) OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT(word.size());
    if (!(this->finalized))
      if (!this->units.add_unit(word))
	this->set_error();
      // this->units = GenericUnits<Encoding>({GenericUnit<Encoding>(word)});
    return TokenBase<Encoding>::finalize(x);
  }
  //! \copydoc TokenBase::display
  std::ostream & display(std::ostream &os) const OVERRIDE_CXX11 {
    os << "WordToken(" << convert_chars<Encoding,UTF8<char> >(word) << ")";
    return os;
  }
  std::basic_string<Ch> word;
  friend class NumberToken<Encoding>;
};

//! Token class for numbers.
/*!
  Defined by number strings.
  \tparam Encoding Token encoding.
 */
template<typename Encoding>
class NumberToken : public WordToken<Encoding> {
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
public:
  //! Constructor
  /*!
    \param[in] c First character representing the token.
    \param parent0 Parent token containing this sub-token.
   */
  NumberToken(const Ch c, TokenBase<Encoding> *parent0=NULL) : WordToken<Encoding>(c, parent0) {}
  //! \copydoc TokenBase::is_numeric
  bool is_numeric() OVERRIDE_CXX11 { return true; }
  //! \copydoc TokenBase::finalize
  GenericUnits<Encoding> finalize(TokenFinalization x) OVERRIDE_CXX11 {
    if (!(this->finalized)) {
      this->value_ = char_to_double<Ch>(this->word);
      this->units.add_factor(this->value_);
    }
    return TokenBase<Encoding>::finalize(x);
  }
  //! \copydoc TokenBase::display
  std::ostream & display(std::ostream &os) const OVERRIDE_CXX11 {
    os << "NumericToken(" << convert_chars<Encoding,UTF8<char> >(this->word) << ")";
    return os;
  }
};

//! Token class for groups of sub-tokens.
/*!
  Defined by sets of tokens.
  \tparam Encoding Token encoding.
 */
template<typename Encoding>
class GroupToken : public TokenBase<Encoding> {
  typedef typename Encoding::Ch Ch; //!< Character type from encoding.
public:
  //! Constructor
  /*!
    \param parent0 Parent token containing this sub-token.
   */
  GroupToken(TokenBase<Encoding> *parent0=NULL) : TokenBase<Encoding>(kGroupToken, parent0), tokens() {}
  //! Destructor
  ~GroupToken() OVERRIDE_CXX11 {
    for (size_t i = 0; i < tokens.size(); i++)
      delete tokens[i];
    tokens.clear();
  }
  //! \copydoc TokenBase::current_token
  TokenBase<Encoding>* current_token() OVERRIDE_CXX11 {
    if (tokens.size() == 0)
      return TokenBase<Encoding>::current_token();
    return tokens[tokens.size() - 1]->current_token();
  }
  //! Get the current token group.
  GroupToken<Encoding>* current_group() {
    int idx = (int)(tokens.size()) - 1;
    if ((idx >= 0) && (tokens[(size_t)idx]->t == kGroupToken)
	&& (!(tokens[(size_t)idx]->finalized))) {
      GroupToken<Encoding>* grp = static_cast<GroupToken<Encoding>*>(tokens[(size_t)idx]);
      return grp->current_group();
    }
    return this;
  }
  //! Append an operator to this token group.
  /*!
    \param[in] c First character in the operator token.
    \param dont_descend If true, the token will be appended to this group
      without checking if the token should be appended to a nested group.
    \returns Created operator token.
   */
  OperatorToken<Encoding>* append_op(const Ch c, bool dont_descend=false) {
    OperatorToken<Encoding>* op = new OperatorToken<Encoding>(c, this);
    append(op, dont_descend);
    return op;
  }
  //! \copydoc TokenBase::append
  void append(const Ch c) OVERRIDE_CXX11 {
    TokenBase<Encoding>* curr = current_token();
    if ((curr->t == kGroupToken) || curr->finalized) {
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
    if ((!curr->is_numeric()) &&
	(c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
	 c == '5' || c == '6' || c == '7' || c == '8' || c == '9')) {
      append_op('^');
      append(c);
      return;
    }
    RAPIDJSON_ASSERT(curr->t == kWordToken);
    WordToken<Encoding>* word = static_cast<WordToken<Encoding>*>(curr);
    word->append(c);
  }
  //! Append a token to this group
  /*!
    \param x Pointer to the token that should be appended and updated
      with parent information.
    \param dont_descend If true, the token will be appended to this group
      without checking if the token should be appended to a nested group.
    \returns Updated token.
   */
  TokenBase<Encoding>* append(TokenBase<Encoding>* x, bool dont_descend=false) {
    GroupToken<Encoding>* curr;
    if (dont_descend)
      curr = this;
    else {
      curr = current_group();
      if (curr->tokens.size() > 0) {
	TokenBase<Encoding>* prev = curr->current_token();
	if (!(prev->finalized))
	  prev->finalize(kTokenFinalizeNext);
	// Handle special case where exponent is set by having number
	// directly following the unit
	if (curr->tokens.size() >= 2) {
	  TokenBase<Encoding>* prev2 = curr->tokens[curr->tokens.size() - 2];
	  if (prev->t == kOperatorToken &&
	      prev2->t == kWordToken && !prev2->is_numeric() &&
	      prev2->finalized == kTokenFinalizeNext &&
	      x->t == kWordToken && x->is_numeric() &&
	      (static_cast<OperatorToken<Encoding>*>(prev)->op == '-' ||
	       static_cast<OperatorToken<Encoding>*>(prev)->op == '+')) {
	    OperatorToken<Encoding>* prev_op = static_cast<OperatorToken<Encoding>*>(prev);
	    if (prev_op->op == '-') {
	      prev2->reset();
	      static_cast<WordToken<Encoding>*>(x)->word.insert(0, 1, prev_op->op);
	      prev2->finalize(kTokenFinalizeNext);
	    }
	    prev_op->op = '^';
	  }
	}
	if ((x->t != kOperatorToken) && (prev->t != kOperatorToken))
	  curr->append_op('*', true);
      }
    }
    RAPIDJSON_ASSERT(!(curr->finalized));
    curr->tokens.push_back(x);
    x->parent = curr;
    return x;
  }
  //! Group operator sub-tokens and the tokens they operate on
  /*!
    \param[in] op Operator to group.
   */
  void group_operators(const char op) {
    std::vector<char> ops;
    ops.push_back(op);
    group_operators(ops);
  }
  //! Group operator sub-tokens and the tokens they operate on
  /*!
    \param[in] op1 First operator to group.
    \param[in] op2 Second operator to group.
   */
  void group_operators(const char op1, const char op2) {
    std::vector<char> ops;
    ops.push_back(op1);
    ops.push_back(op2);
    group_operators(ops);
  }
  //! Group operator sub-tokens and the tokens they operate on
  /*!
    \param[in] ops Set of operators that should be grouped.
   */
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
	    tokens[ii] = NULL;
	  }
	  tokens[i + 1] = (TokenBase<Encoding>*)(new_group);
	  new_group->finalize(kTokenFinalizeAlways);
	  exponents.push_back(i - 1);
	  exponents.push_back(i);
	  i++;
	}
      }
    }
    for (std::vector<size_t>::reverse_iterator it = exponents.rbegin(); it != exponents.rend(); it++)
      tokens.erase(tokens.begin() + (int)(*it));
  }
  //! \copydoc TokenBase::finalize
  GenericUnits<Encoding> finalize(TokenFinalization x) OVERRIDE_CXX11 {
    if ((tokens.size() == 0) || this->finalized)
      return this->units;
    // Group operators first in order of operations
    if (tokens.size() > 3) {
      group_operators('^');
      group_operators('*', '/');
      group_operators('+', '-');
    }
    // Complete operations from left to right
    GenericUnits<Encoding> out = tokens[0]->finalize(kTokenFinalizeGroup);
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
	  out = op->operate(out, tokens[i + 1]->finalize(kTokenFinalizeGroup));
	}
      }
    }
    this->units = out;
    return TokenBase<Encoding>::finalize(x);
  }
  //! \copydoc TokenBase::is_numeric
  bool is_numeric() OVERRIDE_CXX11 {
    for (typename std::vector<TokenBase<Encoding>*>::iterator it = tokens.begin(); it != tokens.end(); it++) {
      if (!((*it)->is_numeric()))
	return false;
    }
    return true;
  }
  //! \copydoc TokenBase::display
  std::ostream & display(std::ostream &os) const OVERRIDE_CXX11 {
    os << "GroupToken(";
    size_t i = 0;
    for (typename std::vector<TokenBase<Encoding>*>::const_iterator it = tokens.begin(); it != tokens.end(); it++, i++) {
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
GenericUnits<Encoding> GenericUnits<Encoding>::parse_units(const typename Encoding::Ch* str,
							   const size_t len,
							   const bool verbose) {
  if (verbose)
    std::cout << "parse_units(\"" << convert_chars<Encoding,UTF8<char> >(str) << "\")" << std::endl;
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
      token.current_group()->finalize(parser::kTokenFinalizeBracket);
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
	word->finalize(parser::kTokenFinalizeSpace);
      break;
    }
    case 'e': {
      if (((i + 1) < len) &&
	  ((str[i + 1] == '+') || (str[i + 1] == '-') ||
	   (str[i + 1] == '0') || (str[i + 1] == '1') ||
	   (str[i + 1] == '2') || (str[i + 1] == '3') ||
	   (str[i + 1] == '4') || (str[i + 1] == '5') ||
	   (str[i + 1] == '6') || (str[i + 1] == '7') ||
	   (str[i + 1] == '8') || (str[i + 1] == '9'))) {
	parser::TokenBase<Encoding>* curr = token.current_token();
	if ((curr->t == parser::kWordToken) && curr->is_numeric() &&
	    (!curr->finalized)) {
	  curr->append(str[i++]);
	  curr->append(str[i]);
	  break;
	}
      }
      // fall through to default
      RAPIDJSON_DELIBERATE_FALLTHROUGH;
    }
    case 'n': {
      // Special case of n/a
      if (((i + 2) < len) && (str[i + 1] == '/') && (str[i + 2] == 'a')) {
	parser::TokenBase<Encoding>* curr = token.current_token();
	if ((curr->t == parser::kGroupToken) || curr->finalized) {
	  token.append(str[i++]);
	  token.append(str[i++]);
	  token.append(str[i++]);
	  break;
	}
      }
      // fall through to default
      RAPIDJSON_DELIBERATE_FALLTHROUGH;
    }
    case '%': {
      parser::TokenBase<Encoding>* curr = token.current_token();
      if ((curr->t == parser::kWordToken) && !(curr->finalized) &&
	  curr->is_numeric()) {
	std::basic_string<Ch> hundred;
	hundred.push_back('1');
	hundred.push_back('0');
	hundred.push_back('0');
	if (static_cast<parser::WordToken<Encoding>*>(curr)->word == hundred &&
	    curr->parent && curr->parent->t == parser::kGroupToken) {
	  parser::GroupToken<Encoding>* curr_parent = static_cast<parser::GroupToken<Encoding>*>(curr->parent);
	  curr_parent->tokens.pop_back();
	  parser::TokenBase<Encoding>* old_curr = curr;
	  curr = new parser::WordToken<Encoding>(hundred[0], curr_parent);
	  for (size_t ii = 1; ii < hundred.size(); ii++)
	    curr->append(hundred[ii]);
	  curr_parent->tokens.push_back(curr);
	  delete old_curr;
	}
      }
      // fall through to default
      RAPIDJSON_DELIBERATE_FALLTHROUGH;
    }
    default:
      token.append(c);
    }
    if (verbose) {
      std::cout << c << ": ";
      token.display(std::cout);
      std::cout << std::endl;
    }
    if (token.errorFlag)
      break;
  }
  GenericUnits<Encoding> out = token.finalize(parser::kTokenFinalizeFinal);
  if (len == 0) {
    RAPIDJSON_ASSERT(!token.errorFlag);
    token.errorFlag = (!out.add_unit(std::basic_string<Ch>()));
  }
  if (token.errorFlag)
    out = GenericUnits<Encoding>();
  return out;
}

// Classes to allow operators to be passed
//! Bitwise flags describing binary operators
enum BinaryOpFlag {
  kBinaryOpFlagNull      , //!< No flag
  kBinaryOpFlagConvert   , //!< Operands must have compatible units
  kBinaryOpFlagReversible, //!< Operands are reversible
  kBinaryOpFlagScalar    , //!< RHS operand must be a scalar
  kBinaryOpFlagMax         //!< Max flag
};
//! IDs for binary operators
enum BinaryOpID {
  kBinaryOpIDNull,     //!< Base class
  kBinaryOpIDAdd,      //!< Add operands
  kBinaryOpIDSubtract, //!< Subtract RHS operand from LHS
  kBinaryOpIDMultiply, //!< Multiply operands
  kBinaryOpIDDivide,   //!< Divide LHS operand by RHS
  kBinaryOpIDModulo,   //!< Perform module on LHS by RHS
  kBinaryOpIDPower,    //!< Raise RHS operand to power of RHS
  kBinaryOpIDMax       //!< Max ID
};
struct ValueBinaryOp {
  ValueBinaryOp(BinaryOpID id_, int flags_ = 0) :
    id(id_), flags(flags_) {
    switch (id) {
    case kBinaryOpIDAdd: {
      flags |= ((1 << kBinaryOpFlagConvert) |
                (1 << kBinaryOpFlagReversible));
      break;
    }
    case kBinaryOpIDSubtract: {
      flags |= (1 << kBinaryOpFlagConvert);
      break;
    }
    case kBinaryOpIDMultiply: {
      flags |= (1 << kBinaryOpFlagReversible);
      break;
    }
    case kBinaryOpIDDivide: {
      break;
    }
    case kBinaryOpIDModulo: {
      // This is not really true, but ValueBinaryOp is not used for
      // defining operator%= for quantities, only scalars
      flags |= (1 << kBinaryOpFlagScalar); 
      break;
    }
    case kBinaryOpIDPower: {
      flags |= (1 << kBinaryOpFlagScalar);
      break;
    }
    default:
      RAPIDJSON_ASSERT(id != kBinaryOpIDNull &&
                       id != kBinaryOpIDMax);
    }
  }
  bool requires_convert() const {
    return (flags & (1 << kBinaryOpFlagConvert));
  }
  bool reversible() const {
    return (flags & (1 << kBinaryOpFlagReversible));
  }
  bool requires_scalar() const {
    return (flags & (1 << kBinaryOpFlagScalar));
  }
  template <typename T1, typename T2>
  T1& op_inplace(T1& lv, const T2& rv,
                 RAPIDJSON_DISABLEIF((
                   internal::AndExpr<
                   internal::NotExpr<internal::IsSame<T1,T2> >,
                   YGGDRASIL_IS_CASTABLE(T2, T1) >))) const {
    switch (id) {
    case kBinaryOpIDAdd: {
      lv += rv;
      break;
    }
    case kBinaryOpIDSubtract: {
      lv -= rv;
      break;
    }
    case kBinaryOpIDMultiply: {
      lv *= rv;
      break;
    }
    case kBinaryOpIDDivide: {
      lv /= rv;
      break;
    }
    case kBinaryOpIDModulo: {
      lv = _modulo(lv, rv);
      break;
    }
    // case kBinaryOpIDPower: {
    //   lv = _power(lv, rv);
    //   break;
    // }
    default:
      RAPIDJSON_ASSERT(id != kBinaryOpIDNull &&
                       id != kBinaryOpIDMax &&
                       id != kBinaryOpIDPower);
    }
    return lv;
  }
  
  template <typename T1, typename T2>
  T1& op_inplace(T1& lv, const T2& rv,
                 RAPIDJSON_ENABLEIF((
                   internal::AndExpr<
                   internal::NotExpr<internal::IsSame<T1,T2> >,
                   YGGDRASIL_IS_CASTABLE(T2, T1) >))) const {
    return op_inplace(lv, castPrecision<T2,T1>(rv));
  }
  template <typename T1, typename T2>
  T1 op(const T1& lv, const T2& rv,
        RAPIDJSON_DISABLEIF((
          internal::AndExpr<
          internal::NotExpr<internal::IsSame<T1,T2> >,
          YGGDRASIL_IS_CASTABLE(T2, T1) >))) const {
    T1 out = lv;
    op_inplace(out, rv);
    return out;
  }
  template <typename T1, typename T2>
  T1 op(const T1& lv, const T2& rv,
        RAPIDJSON_ENABLEIF((
          internal::AndExpr<
          internal::NotExpr<internal::IsSame<T1,T2> >,
          YGGDRASIL_IS_CASTABLE(T2, T1) >))) const {
    return op(lv, castPrecision<T2,T1>(rv));
  }
  BinaryOpID id; //!< ID for the operator
  int flags;     //!< Bitwise flags describing the operator

private:
  template<typename T>
  T _modulo(const T& lv, const T& rv,
            RAPIDJSON_ENABLEIF((YGGDRASIL_IS_FLOAT_TYPE(T)))) const {
    return fmod(lv, rv);
  }
  template<typename T>
  T _modulo(const T& lv, const T& rv,
            RAPIDJSON_DISABLEIF((YGGDRASIL_IS_FLOAT_TYPE(T)))) const {
    return lv % rv;
  }
  template<typename T>
  T _power(const T& lv, const T& rv,
           RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T)))) const {
    return std::pow(lv, rv);
  }
  template<typename T>
  T _power(const T& lv, const T& rv,
           RAPIDJSON_DISABLEIF((YGGDRASIL_IS_ANY_SCALAR(T)))) const {
    return lv.pow(rv);
  }
};

// Macros for defining operators/methods
#define QUANTITY_CASTABLE_FORWARD(T1, T2)   \
  YGGDRASIL_IS_CASTABLE(T1, T2)
#define QUANTITY_CASTABLE_REVERSE(T1, T2)               \
  internal::AndExpr<                                    \
  internal::NotExpr<YGGDRASIL_IS_CASTABLE(T1, T2) >,    \
   YGGDRASIL_IS_CASTABLE(T2, T1) >

/*!
  \brief Define operator for default class that is C++11 compatible
  \param DEF Default class.
  \param TYP Macro that will create full class type when provided a
    template type.
  \param OP Operator
 */
#if RAPIDJSON_HAS_CXX11
#define DEFINE_OP_DEFAULT_CLASS(DEF, TYP, OP)
#else
#define DEFINE_OP_DEFAULT_CLASS(DEF, TYP, OP)				\
  template<typename T2>							\
  TYP(T)& operator OP (const DEF<T2>& x) {				\
    if (!internal::IsSame<Encoding,UTF8<char> >::Value) {		\
      TYP(T2) tmp = x.template transcode<Encoding>();			\
      return *this OP tmp;						\
    }									\
    return *this OP *((TYP(T2)*)(&x));					\
  }
#endif
  

// Macros for delegating to inplace operators/methods
/*!
  \brief Create method from inplace version w/o arguments.
  \param TYP Macro that will create full class type when provided a
    template type.
  \param OP Method name.
  \param IOP Inplace method name.
 */
#define DELEGATE_TO_INPLACE_NOARGS_(TYP, OP, IOP)                       \
  TYP(T) OP() const {                                                   \
    TYP(T) out(*this);                                                  \
    out.IOP();                                                          \
    return out;                                                         \
  }
/*!
  \brief Create method from inplace version w/o casting arguments.
  \param TYP Macro that will create full class type when provided a
    template type.
  \param TYP2 Macro that will create a full class type when provided
    a template type for the rhs argument.
  \param OP Method name.
  \param IOP Inplace method name.
 */
#define DELEGATE_TO_INPLACE_NOCAST_(TYP, TYP2, OP, IOP)                 \
  template<typename T2>                                                 \
  TYP(T) OP(const TYP2(T2)& rhs) const {                                \
    TYP(T) out(*this);                                                  \
    out.IOP(rhs);                                                       \
    return out;                                                         \
  }
/*!
  \brief Create method from inplace version for quantity.
  \param TYP Macro that will create full class type when provided a
    template type.
  \param RHS Macro that will create full class type when provided a
    template type for the argument type.
  \param OP Method name.
  \param IOP Inplace method name.
 */
#define DELEGATE_TO_INPLACE_METHOD_QUANTITY_(TYP, RHS, OP, IOP)         \
  template<typename T2>                                                 \
  RAPIDJSON_ENABLEIF_RETURN((YGGDRASIL_IS_CASTABLE(T2, ScalarType)),    \
                            (TYP(ScalarType)))                          \
    OP(const RHS(T2)& rhs) const {                                      \
    TYP(ScalarType) out = this->_copy_for_op<ScalarType>(rhs);          \
    out.IOP(rhs);                                                       \
    return out;                                                         \
  }                                                                     \
  template<typename T2>                                                 \
  RAPIDJSON_ENABLEIF_RETURN((internal::AndExpr<                         \
                             internal::NotExpr<                         \
                             YGGDRASIL_IS_CASTABLE(T2, ScalarType) >,   \
                             YGGDRASIL_IS_CASTABLE(ScalarType, T2) >),  \
                            (TYP(T2)))                                  \
    OP(const RHS(T2)& rhs) const {                                      \
    TYP(T2) out = this->_copy_for_op<T2>(rhs);                          \
    out.IOP(rhs);                                                       \
    return out;                                                         \
  }
/*!
  \brief Create method from inplace version for scalar.
  \param TYP Macro that will create full class type when provided a
    template type.
  \param OP Method name.
  \param IOP Inplace method name.
 */
#define DELEGATE_TO_INPLACE_METHOD_SCALAR_(TYP, OP, IOP)                \
  DELEGATE_TO_INPLACE_METHOD_QUANTITY_(TYP, PASS, OP, IOP)
/*!
  \brief Create an operator friend function from the inplace version.
  \param TYP Macro that will create full class type when provided a
    template type for the lhs argument type.
  \param TYP2 Macro that will create full class type when provided a
    template type for the rhs argument type.
  \param OP Operator.
  \param IOP Inplace operator.
 */
#define DELEGATE_TO_INPLACE_OP_FRIEND_(TYP, TYP2, OP, IOP)              \
  template<typename T2>							\
  friend                                                                \
  RAPIDJSON_ENABLEIF_RETURN((YGGDRASIL_IS_ANY_SCALAR(T2)), (TYP(T)))    \
    operator OP(const TYP(T) lhs, const TYP2(T2)& rhs) {                \
    TYP(T) out(lhs);                                                    \
    out IOP rhs;                                                        \
    return out;								\
  }
/*!
  \brief Create an operator method from the inplace version for scalar.
  \param TYP Macro that will create full class type when provided a
    template type.
  \param OP Operator.
  \param IOP Inplace operator.
 */
#define DELEGATE_TO_INPLACE_OP_SCALAR_(TYP, OP, IOP)                    \
  DELEGATE_TO_INPLACE_METHOD_SCALAR_(TYP, operator OP, operator IOP)
/*!
  \brief Create an operator method from the inplace version for quantity.
  \param TYP Macro that will create full class type when provided a
    template type.
  \param RHS Macro that will create full class type when provided a
    template type for the argument type.
  \param OP Operator.
  \param IOP Inplace operator.
 */
#define DELEGATE_TO_INPLACE_OP_QUANTITY_(TYP, RHS, OP, IOP)             \
  DELEGATE_TO_INPLACE_METHOD_QUANTITY_(TYP, RHS, operator OP, operator IOP)
  
/*!
  \brief Create reverse operator method for scalar & quantity
  \param TYP Macro that will create full class type for rhs operand
    when provided a template type.
  \param OP Operator.
  \param BODY Contents of the operator.
 */
#define DELEGATE_OP_FRIEND_REVERSE_(TYP, OP, BODY)                      \
  template<typename T2>							\
  friend                                                                \
  RAPIDJSON_ENABLEIF_RETURN((YGGDRASIL_IS_ANY_SCALAR(T2)), (TYP(T)))    \
    operator OP(const T2 lhs, const TYP(T)& rhs) {                      \
    return BODY;                                                        \
  }

// Macros for inheritance from base class
/*!
  \brief Create an in place method w/o args from the base method
  \param CLS Child class.
  \param TYP Macro that will create the child class type when provided
    a template type.
  \param BASE Macro that will create the base class type when provided
    a template type.
  \param IOP Inplace method.
 */
#define INHERIT_INPLACE_NOARGS_(CLS, TYP, BASE, IOP) \
  CLS& IOP() {                                       \
    BASE(T)::IOP();                                  \
    return *this;                                    \
  }
/*!
  \brief Create an in place method w/o casting from the base method
  \param CLS Child class.
  \param TYP Macro that will create the child class type when provided
    a template type.
  \param TYP2 Macro that will create a full class type when provided
    a template type for the rhs argument.
  \param BASE Macro that will create the base class type when provided
    a template type.
  \param IOP Inplace method.
 */
#define INHERIT_INPLACE_NOCAST_(CLS, TYP, TYP2, BASE, IOP)              \
  template<typename T2>                                                 \
  CLS& IOP(const TYP2(T2)& rhs) {                                       \
    BASE(T)::IOP(rhs);                                                  \
    return *this;                                                       \
  }
/*!
  \brief Create an in place method from the base method
  \param CLS Child class.
  \param TYP Macro that will create the child class type when provided
    a template type.
  \param TYP2 Macro that will create a full class type when provided
    a template type for the rhs argument.
  \param BASE Macro that will create the base class type when provided
    a template type.
  \param IOP Inplace method.
 */
#define INHERIT_INPLACE_METHOD_(CLS, TYP, TYP2, BASE, IOP)              \
  template<typename T2>                                                 \
  RAPIDJSON_ENABLEIF_RETURN((YGGDRASIL_IS_CASTABLE(T2, ScalarType)),    \
                            (CLS&))                                     \
    IOP(const TYP2(T2)& rhs) {                                          \
    BASE(T)::IOP(rhs);                                                  \
    return *this;                                                       \
  }
/*!
  \brief Create a method with an in place version w/o args.
  \param CLS Child class.
  \param TYP Macro that will create the child class type when provided
    a template type.
  \param BASE Macro that will create the base class type when provided
    a template type.
  \param OP Method.
  \param IOP Inplace method.
 */
#define INHERIT_NOARGS_(CLS, TYP, BASE, OP, IOP) \
  INHERIT_INPLACE_NOARGS_(CLS, TYP, BASE, IOP)   \
  DELEGATE_TO_INPLACE_NOARGS_(TYP, OP, IOP)
/*!
  \brief Create a method with an in place version w/o casting args.
  \param CLS Child class.
  \param TYP Macro that will create the child class type when provided
    a template type.
  \param TYP2 Macro that will create a full class type when provided
    a template type for the rhs argument.
  \param BASE Macro that will create the base class type when provided
    a template type.
  \param OP Method.
  \param IOP Inplace method.
 */
#define INHERIT_NOCAST_(CLS, TYP, TYP2, BASE, OP, IOP)   \
  INHERIT_INPLACE_NOCAST_(CLS, TYP, TYP2, BASE, IOP)     \
  DELEGATE_TO_INPLACE_NOCAST_(TYP, TYP2, OP, IOP)
/*!
  \brief Create a method with an in place version.
  \param CLS Child class.
  \param TYP Macro that will create the child class type when provided
    a template type.
  \param TYP2 Macro that will create a full class type when provided
    a template type for the rhs argument.
  \param BASE Macro that will create the base class type when provided
    a template type.
  \param OP Method.
  \param IOP Inplace method.
 */
#define INHERIT_METHOD_SCALAR_(CLS, TYP, BASE, OP, IOP)    \
  INHERIT_INPLACE_METHOD_(CLS, TYP, PASS, BASE, IOP)       \
  DELEGATE_TO_INPLACE_METHOD_SCALAR_(TYP, OP, IOP)
/*!
  \brief Create an operator with an in place version.
  \param CLS Child class.
  \param TYP Macro that will create the child class type when provided
    a template type.
  \param TYP2 Macro that will create a full class type when provided
    a template type for the rhs argument.
  \param BASE Macro that will create the base class type when provided
    a template type.
  \param OP Operator.
  \param IOP Inplace operator.
 */
#define INHERIT_OP_QUANTITY_(CLS, TYP, BASE, OP, IOP)          \
  INHERIT_INPLACE_METHOD_(CLS, TYP, TYP, BASE, operator IOP)   \
  DELEGATE_TO_INPLACE_OP_QUANTITY_(TYP, TYP, OP, IOP)          \
  DELEGATE_TO_INPLACE_OP_QUANTITY_(BASE, BASE, OP, IOP)
#define INHERIT_OP_SCALAR_(CLS, TYP, BASE, OP, IOP)            \
  INHERIT_INPLACE_METHOD_(CLS, TYP, PASS, BASE, operator IOP)  \
  DELEGATE_TO_INPLACE_OP_SCALAR_(TYP, OP, IOP)

/*!
  \brief Inherit scalar constructors from the base class.
  \param CLS Child class.
  \param BASE Macro that will create the base class type when provided
    a template type.
 */
#define INHERIT_CONSTRUCTORS_(CLS, BASE)				\
  CLS() : BASE(T)() {}							\
  CLS(const CLS& rhs) : BASE(T)(rhs) {}					\
  CLS(const T& value) : BASE(T)(value) {}				\
  CLS(const T& value, const Ch* units) : BASE(T)(value, units) {}	\
  CLS(const T& value, const UnitsType& units) : BASE(T)(value, units) {}
/*!
  \brief Inherit array constructors from the base class.
  \param CLS Child class.
  \param BASE Macro that will create the base class type when provided
    a template type.
 */
#define INHERIT_CONSTRUCTORS_ARRAY_(CLS, BASE)				\
  INHERIT_CONSTRUCTORS_(CLS, BASE)					\
  CLS(const T* value, const SizeType& ndim, const SizeType* shape,	\
      const UnitsType& units = UnitsType()) :				\
  BASE(T)(value, ndim, shape, units) {}					\
  CLS(const T* value, const SizeType& len,				\
      const UnitsType& units = UnitsType()) :				\
    BASE(T)(value, len, units) {}					\
  CLS(const T* value, const SizeType& ndim, const SizeType* shape,	\
      const Ch* units) :						\
    BASE(T)(value, ndim, shape, units) {}				\
  template<SizeType N>							\
  CLS(const T (&value)[N], const UnitsType& units = UnitsType()) :	\
    BASE(T)(value, units) {}						\
  template<SizeType N>							\
  CLS(const T (&value)[N], const Ch* units) :				\
    BASE(T)(value, units) {}						\
  template<SizeType N, SizeType M>					\
  CLS(const T (&value)[N][M], const UnitsType& units = UnitsType()) :	\
    BASE(T)(value, units) {}						\
  template<SizeType N, SizeType M>					\
  CLS(const T (&value)[N][M], const Ch* units) :			\
    BASE(T)(value, units) {}
/*!
  \brief Macro for overriding methods that return the child class type.
  \param CLS Child class.
  \param TYP Macro that will create full class type when provided a
    template type.
  \param BASE Macro that will create the base class type when provided
    a template type.
 */
#define INHERIT_OPERATORS_(CLS, TYP, BASE)				\
  CLS& operator-() {                                                    \
    return (*this) * -1;                                                \
  }                                                                     \
  INHERIT_OP_QUANTITY_(CLS, TYP, BASE, +, +=)                           \
  INHERIT_OP_QUANTITY_(CLS, TYP, BASE, -, -=)                           \
  INHERIT_OP_QUANTITY_(CLS, TYP, BASE, *, *=)                           \
  INHERIT_OP_SCALAR_(CLS, TYP, BASE, *, *=)                             \
  DELEGATE_OP_FRIEND_REVERSE_(TYP, *, rhs * lhs)                        \
  INHERIT_OP_QUANTITY_(CLS, TYP, BASE, /, /=)                           \
  INHERIT_OP_SCALAR_(CLS, TYP, BASE, /, /=)                             \
  DELEGATE_OP_FRIEND_REVERSE_(TYP, /, rhs.pow(-1) * lhs)                \
  INHERIT_OP_QUANTITY_(CLS, TYP, BASE, %, %=)                           \
  INHERIT_OP_SCALAR_(CLS, TYP, BASE, %, %=)                             \
  INHERIT_NOCAST_(CLS, TYP, PASS, BASE, pow, pow_inplace)               \
  INHERIT_NOARGS_(CLS, TYP, BASE, floor, floor_inplace)                 \
  CLS& operator=(const CLS& rhs) {                                      \
    BASE(T)::operator=(rhs);                                            \
    return *this;                                                       \
  }                                                                     \
  TYP(T)* copy() const { return new TYP(T)(*this); }			\
  void* copy_void() const { return (void*)copy(); }			\
  CLS as(const char* units) const {					\
    CLS out(*this);							\
    out.convert_to(units);						\
    return out;								\
  }									\
  CLS as(const UnitsType& units) const {				\
    CLS out(*this);							\
    out.convert_to(units);						\
    return out;								\
  }									\
  CLS as_units_system(const UnitsType& units) const {			\
    CLS out(*this);							\
    out.convert_to_units_system(units);					\
    return out;								\
  }

// Macros for generating types from template
#define PASS(TT) TT
#define GENERIC_QUANTITY_ARRAY_TYPE(TT) GenericQuantityArray<TT, Encoding>
#define GENERIC_QUANTITY_TYPE(TT) GenericQuantity<TT, Encoding>
#define QUANTITY_ARRAY_TYPE(TT) QuantityArray<TT>
#define QUANTITY_TYPE(TT) Quantity<TT>

#if !RAPIDJSON_HAS_CXX11
//! Forward declaration
template<typename T>
class Quantity;
template<typename T>
class QuantityArray;
#endif

#if RAPIDJSON_HAS_CXX11
#define FRIEND_DEFAULT_(CLS)
#define CREATE_DEFAULT_(CLS, TYP, BASE, BASETYP, ENCODING, CTORS)	\
  template<typename T>							\
  using CLS = BASE<T, ENCODING>
#else // RAPIDJSON_HAS_CXX11
#define FRIEND_DEFAULT_(CLS)						\
  friend class CLS<T>;
#define CREATE_DEFAULT_(CLS, TYP, BASE, BASETYP, ENCODING, CTORS)	\
  template<typename T>							\
  class CLS : public BASE<T, ENCODING> {				\
  public:								\
    typedef ENCODING Encoding;						\
    typedef typename BASETYP(T)::Ch Ch;					\
    typedef typename BASETYP(T)::UnitsType UnitsType;			\
    typedef typename BASETYP(T)::ScalarType ScalarType;			\
    CTORS(CLS, BASETYP)							\
    INHERIT_OPERATORS_(CLS, TYP, BASETYP)				\
    TYP(T)& operator=(const BASETYP(T)& other) {			\
      BASETYP(T)::operator=(other);					\
      return *this;							\
    }									\
    operator BASETYP(T)() const {					\
      return BASETYP(T)(*static_cast<const BASETYP(T)*>(this));		\
    }									\
  }
#endif // RAPIDJSON_HAS_CXX11

//! Array quantity with units.
//! \tparam T Type of the underlying scalar.
//! \tparam Encoding Encoding used to store the unit strings.
template<typename T, typename Encoding>
class GenericQuantityArray {
public:
  typedef Encoding EncodingType;            //!< Encoding type from template parameter.
  typedef typename Encoding::Ch Ch;         //!< Character type from encoding.
  typedef GenericUnits<Encoding> UnitsType; //!< Units type.
  typedef T ScalarType;                     //!< Scalar type.
  //! \brief Empty constructor.
  GenericQuantityArray() : value_(), units_(), shape_() {}
  //! \brief Copy constructor.
  //! \param rhs QuantityArray to copy.
  GenericQuantityArray(const GenericQuantityArray<ScalarType, EncodingType>& rhs) :
    value_(), units_(rhs.units_), shape_()
  { _init(rhs.value_.data(), rhs.ndim(), rhs.shape_.data()); }
  //! \brief Copy constructor for array with different type/encoding.
  //! \tparam T2 Type of elements in rhs.
  //! \tparam Encoding2 Encoding of units in rhs.
  //! \param rhs QuantityArray to copy.
  template<typename T2, typename Encoding2>
  GenericQuantityArray(const GenericQuantityArray<T2, Encoding2>& rhs,
                       RAPIDJSON_DISABLEIF((internal::OrExpr<
                                            internal::AndExpr<
                                            internal::IsSame<T2, ScalarType>,
                                            internal::IsSame<Encoding2, EncodingType> >,
                                            internal::NotExpr<YGGDRASIL_IS_CASTABLE(T2, ScalarType) > >))) :
    value_(), units_(rhs.units()), shape_()
  { _init(rhs.value(), rhs.ndim(), rhs.shape()); }
  //! \brief Create a quantity.
  //! \param value Pointer to an array.
  //! \param ndim Number of dimensions in the array.
  //! \param shape Size of the array in each dimension.
  //! \param units Units instance.
  GenericQuantityArray(const ScalarType* value,
		       const SizeType& ndim, const SizeType* shape,
		       const UnitsType& units = UnitsType()) :
    value_(), units_(units), shape_()
  { _init(value, ndim, shape); }
  //! \brief Constructor for array with uniform values
  //! \param value Value that all elements of the array should be set to
  //! \param ndim Number of dimensions in the array.
  //! \param shape Size of the array in each dimension.
  //! \param units Units instance.
  GenericQuantityArray(const ScalarType& value,
		       const SizeType& ndim, const SizeType* shape,
		       const UnitsType& units = UnitsType()) :
    value_(), units_(units), shape_()
  { _init(value, ndim, shape); }
  //! \brief Create a scalar quantity.
  //! \param value Scalar value.
  //! \param units Units instance.
  GenericQuantityArray(const ScalarType value,
		       const UnitsType& units = UnitsType()) :
    value_(), units_(units), shape_()
  { SizeType len = 1; _init(&value, 1, &len); }
private:
  //! \brief Create a scalar quantity.
  //! \param value Scalar value.
  //! \param units Units string.
  GenericQuantityArray(const ScalarType value, const Ch* units) :
    value_(), units_(UnitsType(units)), shape_()
  { SizeType len = 1; _init(&value, 1, &len); }
public:
  //! \brief Create a quantity.
  //! \param value Pointer to an array.
  //! \param len Number of elements in the 1D array.
  //! \param units Units instance.
  GenericQuantityArray(const ScalarType* value, const SizeType& len,
		const UnitsType& units = UnitsType()) :
    value_(), units_(units), shape_()
  { _init(value, 1, &len); }
  //! \brief Create a quantity from units string.
  //! \param value Pointer to an array.
  //! \param ndim Number of dimensions in the array.
  //! \param shape Size of the array in each dimension.
  //! \param units Units string.
  GenericQuantityArray(const ScalarType* value, const SizeType& ndim, const SizeType* shape,
		const Ch* units) :
    value_(), units_(UnitsType(units)), shape_()
  { _init(value, ndim, shape); }
  //! \brief Create a quantity without units.
  //! \tparam N Number of elements in the array.
  //! \param value 1D array.
  //! \param units Units instance.
  template<SizeType N>
  GenericQuantityArray(const ScalarType (&value)[N], const UnitsType& units = UnitsType()) :
    value_(), units_(units), shape_()
  { SizeType len = N; _init(&(value[0]), 1, &len); }
  //! \brief Constructor from units string.
  //! \tparam N Number of elements in the array.
  //! \param value 1D array.
  //! \param units Units string.
  template<SizeType N>
  GenericQuantityArray(const ScalarType (&value)[N], const Ch* units) :
    value_(), units_(UnitsType(units)), shape_()
  { SizeType len = N; _init(&(value[0]), 1, &len); }
  //! \brief Create a quantity without units.
  //! \tparam N Number of elements in the array in dimension 1.
  //! \tparam M Number of elements in the array in dimension 2.
  //! \param value 1D array.
  //! \param units Units instance.
  template<SizeType N, SizeType M>
  GenericQuantityArray(const ScalarType (&value)[N][M], const UnitsType& units = UnitsType()) :
    value_(), units_(units), shape_()
  { SizeType shape[] = {N, M}; _init(&(value[0][0]), 2, &(shape[0])); }
  //! \brief Constructor from units string.
  //! \tparam N Number of elements in the array in dimension 1.
  //! \tparam M Number of elements in the array in dimension 2.
  //! \param value 1D array.
  //! \param units Units string.
  template<SizeType N, SizeType M>
  GenericQuantityArray(const ScalarType (&value)[N][M], const Ch* units) :
    value_(), units_(UnitsType(units)), shape_()
  { SizeType shape[] = {N, M}; _init(&(value[0][0]), 2, &(shape[0])); }
  
  //! \brief Destructor.
  ~GenericQuantityArray() {
    value_.clear();
    shape_.clear();
  }
  //! \brief Copy assignment.
  //! \param rhs GenericQuantityArray to copy.
  GenericQuantityArray& operator=(const GenericQuantityArray& rhs) {
    this->~GenericQuantityArray();
    new (this) GenericQuantityArray(rhs);
    return *this;
  }
  //! \brief Copy assignment from array with different type/encoding.
  //! \tparam T2 Type of elements in rhs.
  //! \tparam Encoding2 Encoding of units in rhs.
  //! \param rhs GenericQuantityArray to copy.
  template<typename T2, typename Encoding2>
  RAPIDJSON_DISABLEIF_RETURN((internal::AndExpr<
                              internal::IsSame<T2, ScalarType>,
                              internal::IsSame<Encoding2, EncodingType> >),
                             (GenericQuantityArray&))
  operator=(const GenericQuantityArray<T2, Encoding2>& rhs) {
    this->~GenericQuantityArray();
    new (this) GenericQuantityArray(rhs);
    return *this;
  }
#if RAPIDJSON_HAS_CXX11_RVALUE_REFS
  //! \brief Move constructor in C++11
  //! \param rhs Right-hand side reference to move.
  // GenericQuantityArray(GenericQuantityArray&& rhs) :
  //   value_
  //! \brief Move assignment in C++11.
  //! \param rhs Right-hand side reference to move.
  GenericQuantityArray& operator=(GenericQuantityArray&& rhs) {
    return *this = static_cast<GenericQuantityArray&>(rhs);
  }
#endif // RAPIDJSON_HAS_CXX11_RVALUE_REFS
  //! \brief Print instance information to an output stream.
  //! \param os Output stream.
  template<typename Ch2>
  void display(std::basic_ostream<Ch2>& os) const {
    os << 'Q' << 'u' << 'a' << 'n' << 't' << 'i' << 't' << 'y' <<
      'A' << 'r' << 'r' << 'a' << 'y' << '(';
    _write_array(os);
    os << ',' << ' ';
    os << units_;
    os << ')';
  }
  //! \brief Get the quantity array as a string.
  //! \return QuantityArray string.
  std::basic_string<Ch> str() const {
    std::basic_stringstream<Ch> ss;
    ss << *this;
    return ss.str();
  }
  //! \brief Create a copy of the quantity with units in a different encoding.
  //! \tparam DestEncoding Encoding that the copy should use.
  //! \return Copy w/ DestEncoding.
  template<typename DestEncoding>
  GenericQuantityArray<ScalarType, DestEncoding> transcode() const {
    GenericUnits<DestEncoding> new_units = units_.template transcode<DestEncoding>();
    return GenericQuantityArray<ScalarType, DestEncoding>(value(), ndim(), shape(), new_units);
  }
private:
  std::vector<SizeType> _index(const SizeType idx) const {
    SizeType ndim_ = ndim();
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
  template<typename T2>
  void _init(const T2*, const SizeType, const SizeType*,
	     RAPIDJSON_DISABLEIF((YGGDRASIL_IS_CASTABLE(T2, ScalarType)))) {
    RAPIDJSON_ASSERT(((YGGDRASIL_IS_CASTABLE(T2, ScalarType)::Value)));
  }
  template<typename T2>
  void _init(const T2* value, const SizeType ndim, const SizeType* shape,
	     RAPIDJSON_ENABLEIF((YGGDRASIL_IS_CASTABLE(T2, ScalarType)))) {
    RAPIDJSON_ASSERT(ndim > 0);
    // Shape
    shape_.resize(ndim);
    for (SizeType i = 0; i < ndim; i++)
      shape_[i] = shape[i];
    // Value
    SizeType N = nelements();
    value_.resize(N);
    for (SizeType i = 0; i < N; i++)
      value_[i] = castPrecision<T2,ScalarType>(value[i]);
  }
  template<typename T2>
  void _init(const T2&, const SizeType, const SizeType*,
	     RAPIDJSON_DISABLEIF((YGGDRASIL_IS_CASTABLE(T2, ScalarType)))) {
    RAPIDJSON_ASSERT(((YGGDRASIL_IS_CASTABLE(T2, ScalarType)::Value)));
  }
  template<typename T2>
  void _init(const T2& value, const SizeType ndim, const SizeType* shape,
	     RAPIDJSON_ENABLEIF((YGGDRASIL_IS_CASTABLE(T2, ScalarType)))) {
    RAPIDJSON_ASSERT(ndim > 0);
    // Shape
    shape_.resize(ndim);
    for (SizeType i = 0; i < ndim; i++)
      shape_[i] = shape[i];
    // Value
    SizeType N = nelements();
    value_.resize(N);
    for (SizeType i = 0; i < N; i++)
      value_[i] = castPrecision<T2,ScalarType>(value);
  }
  template<typename Ch2>
  void _write_array(std::basic_ostream<Ch2>& os) const {
    SizeType ndim_ = ndim();
    SizeType N = nelements();
    std::vector<SizeType> idx;
    for (SizeType i = 0; i < N; i++) {
      idx = _index(i);
      for (SizeType j = 0; j < ndim_; j++) {
	if (idx[j] == 0) {
	  if (j == 0) {
	    if (i == 0)
	      os << '[';
	  } else if (idx[j - 1] == 0) {
	    os << '[';
	  } else {
	    os << ']' << ',' << ' ' << '[';
	  }
	}
      }
      if (idx[ndim_ - 1] > 0)
	os << ',' << ' ';
      os << value_[i];
    }
    for (SizeType i = 0; i < ndim_; i++)
      os << ']';
  }
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
  template<typename T2>
  void raw_set_delta_from_add(const GenericQuantityArray<T2, Encoding>& x,
			      std::string op) {
    // if (op == std::string("%"))
    //   return;
    if ((units_.has_difference() && x.units_.is_difference()) ||
	(units_.is_difference() && x.units_.has_difference()))
      units_.set_delta(kInactiveDelta);
    else if (op == std::string("-") && units_.has_difference())
      units_.set_delta(kActiveDelta);
  }
public:
  //! \brief Get the quantity value without units.
  //! \return Value.
  const ScalarType* value() const { return value_.data(); }
  //! \brief Get the units instance.
  //! \return Units.
  UnitsType units() const { return units_; }
  //! \brief Get the number of dimensions in the array.
  //! \return Number of dimensions.
  SizeType ndim() const { return static_cast<SizeType>(shape_.size()); }
  //! \brief Get the size of the array in each dimension.
  //! \return Array shape.
  const SizeType* shape() const { return shape_.data(); }
  //! \brief Set the quantity value.
  //! \param new_value New quantity value.
  //! \brief Get the total number of elements in the array.
  //! \return The number of elements.
  SizeType nelements() const {
    SizeType out = 0;
    SizeType ndim_ = ndim();
    if (ndim_ > 0) {
      out = 1;
      for (SizeType i = 0; i < ndim_; i++)
	out = out * shape_[i];
    }
    return out;
  }
  //! Update the value array.
  //! \tparam T2 Type of elements in new_value.
  //! \param new_value Pointer to an array.
  //! \param ndim Number of dimensions in the array.
  //! \param shape Size of the array in each dimension.
  template<typename T2>
  void set_value(const T2* new_value, SizeType ndim, SizeType* shape) {
    _init(new_value, ndim, shape);
  }
  //! \brief Set the quantity units without conversion.
  //! \param new_units New quantity units.
  void set_units(const UnitsType& new_units) {
    units_ = new_units;
  }
  //! \brief Get the units string.
  //! \return Units string.
  std::basic_string<Ch> unitsStr() const { return units_.str(); }
  //! \brief Check if another quantity array has the same shape.
  //! \param x QuantityArray for comparison.
  //! \return true if the shapes are equivalent, false otherwise.
  template<typename T2>
  bool is_same_shape(const GenericQuantityArray<T2, Encoding>& x) const {
    SizeType ndim_ = ndim();
    if (ndim_ != x.ndim()) return false;
    for (SizeType i = 0; i < ndim_; i++)
      if (shape_[i] != x.shape()[i]) return false;
    return true;
  }
  //! \brief Check if two quantities are identical. The units must be
  //!   identical, not just compatible.
  //! \param x QuantityArray for comparison.
  //! \return true if the two quantities are identical, false otherwise.
  template<typename T2>
  bool operator==(const GenericQuantityArray<T2, Encoding>& x) const {
    if (units_ != x.units()) return false;
    if (!is_same_shape(x)) return false;
    for (SizeType i = 0; i < nelements(); i++)
      if (!(internal::values_eq(value_[i], x.value()[i]))) return false;
    return true;
  }
  //! \brief Check if two quantities are not identical.
  //! \param x QuantityArray for comparison.
  //! \return true if the two quantities are not identical, false otherwise.
  template<typename T2>
  bool operator!=(const GenericQuantityArray<T2, Encoding>& x) const { return (!(*this==x)); }
  //! \brief Less than comparison operator.
  //! \param x Quantity for comparison.
  //! \return true if less than, false otherwise.
  template<typename T2>
  bool operator<(const GenericQuantityArray<T2, Encoding>& x) const {
    if (!is_same_shape(x)) return false;
    if (units_ != x.units()) {
      if (is_compatible(x.units()))
	return *this < x.as(units_);
      return false;
    }
    for (SizeType i = 0; i < nelements(); i++)
      if (!(internal::values_lt(value_[i], x.value()[i]))) return false;
    return true;
  }
  //! \brief Greater than comparison operator.
  //! \param x QuantityArray for comparison.
  //! \return true if greater than, false otherwise.
  template<typename T2>
  bool operator>(const GenericQuantityArray<T2, Encoding>& x) const {
    if (!is_same_shape(x)) return false;
    if (units_ != x.units()) {
      if (is_compatible(x.units()))
	return *this > x.as(units_);
      return false;
    }
    for (SizeType i = 0; i < nelements(); i++)
      if (!(internal::values_gt(value_[i], x.value()[i]))) return false;
    return true;
  }
  //! \brief Less than or equal to comparison operator.
  //! \param x QuantityArray for comparison.
  //! \return true if less than or equal to, false otherwise.
  template<typename T2>
  bool operator<=(const GenericQuantityArray<T2, Encoding>& x) const { return (!(*this > x)); }
  //! \brief Greater than or equal to comparison operator.
  //! \param x QuantityArray for comparison.
  //! \return true if greater than or equal to, false otherwise.
  template<typename T2>
  bool operator>=(const GenericQuantityArray<T2, Encoding>& x) const { return (!(*this < x)); }

  // ARITHMETIC OPERATORS
  //! \brief Get the negative of the array
  //! \return Negative.
  GenericQuantityArray& operator-() {
    return (*this) * -1;
  }
  //! \brief Multiply by another array or scalar inplace.
  //! \tparam T2 Type of RHS operand.
  //! \param rhs Right-hand side operand (scalar or array of the same shape).
  //! \return Result of multiplication.
  //!
  //! If rhs has the same shape as this array, elements will be
  //!   multiplied on an element-by-element basis. If rhs is a single
  //!   element, each element in this array will be multiplied by it.
  //!   If this array is a single element and rhs is not, this array
  //!   will be modified into a uniform array with the same shape as rhs
  //!   before performing the operation.
  template<typename T2>
  GenericQuantityArray& operator*=(const T2& rhs) {
    _apply_binary_op_inplace_combine(rhs, kBinaryOpIDMultiply);
    return *this;
  }
  //! \brief Multiply by another array or scalar.
  //! \tparam T2 Type of RHS operand.
  //! \param rhs Right-hand side operand (scalar or array of the same shape).
  //! \return Result of multiplication.
  //!
  //! If rhs has the same shape as lhs, elements will be multiplied on
  //!   an element-by-element basis. If rhs or lhs is a single element,
  //!   the result will be eqivalent to multiplying two arrays with the
  //!   same shape where the unitary array is replaced by a uniform
  //!   array of the same shape. Multiplication of arrays with
  //!   different shapes (outside of scalars) is not permitted.
  DELEGATE_TO_INPLACE_OP_SCALAR_(GENERIC_QUANTITY_ARRAY_TYPE, *, *=)
  DELEGATE_TO_INPLACE_OP_QUANTITY_(GENERIC_QUANTITY_ARRAY_TYPE,
                                   GENERIC_QUANTITY_ARRAY_TYPE, *, *=)
  /*!
    \brief Multiply a scalar by a quantity array.
    \tparam T2 Type of lhs scalar operand.
    \param lhs Left-hand side scalar operand.
    \param rhs Right-hand side quantity array operand.
    \return Result of multiplication.
   */
  DELEGATE_OP_FRIEND_REVERSE_(GENERIC_QUANTITY_ARRAY_TYPE, *, rhs * lhs)
  DEFINE_OP_DEFAULT_CLASS(QuantityArray, GENERIC_QUANTITY_ARRAY_TYPE, *=)
  DEFINE_OP_DEFAULT_CLASS(Quantity, GENERIC_QUANTITY_ARRAY_TYPE, *=)
  //! \brief Divide by another array or scalar inplace.
  //! \tparam T2 Type of RHS operand.
  //! \param rhs Right-hand side operand (scalar or array of the same shape).
  //! \return Result of division.
  //!
  //! If rhs has the same shape as this array, elements will be divided
  //!   on an element-by-element basis. If rhs is a single element, each
  //!   element in this array will be divided by it. If this array is
  //!   a single element and rhs is not, this array will be modified into
  //!   a uniform array with the same shape as rhs before performing the
  //!   operation.
  template<typename T2>
  GenericQuantityArray& operator/=(const T2& rhs) {
    _apply_binary_op_inplace_combine(rhs, kBinaryOpIDDivide);
    return *this;
  }
  //! \brief Divide by another array or scalar.
  //! \tparam T2 Type of RHS operand.
  //! \param rhs Right-hand side operand (scalar or array of the same shape).
  //! \return Result of division.
  //!
  //! If rhs has the same shape as lhs, elements will be divided on
  //!   an element-by-element basis. If rhs or lhs is a single element,
  //!   the result will be eqivalent to dividing two arrays with the
  //!   same shape where the unitary array is replaced by a uniform
  //!   array of the same shape. Division of arrays with
  //!   different shapes (outside of scalars) is not permitted.
  DELEGATE_TO_INPLACE_OP_SCALAR_(GENERIC_QUANTITY_ARRAY_TYPE, /, /=)
  DELEGATE_TO_INPLACE_OP_QUANTITY_(GENERIC_QUANTITY_ARRAY_TYPE,
                                   GENERIC_QUANTITY_ARRAY_TYPE, /, /=)
  /*!
    \brief Divide a scalar by a quantity array.
    \tparam T2 Type of lhs scalar operand.
    \param lhs Left-hand side scalar operand.
    \param rhs Right-hand side quantity array operand.
    \return Result of division.
   */
  DELEGATE_OP_FRIEND_REVERSE_(GENERIC_QUANTITY_ARRAY_TYPE, /,
                              rhs.pow(-1) * lhs)
  DEFINE_OP_DEFAULT_CLASS(QuantityArray, GENERIC_QUANTITY_ARRAY_TYPE, /=)
  DEFINE_OP_DEFAULT_CLASS(Quantity, GENERIC_QUANTITY_ARRAY_TYPE, /=)
  //! \brief Modulo by a scalar in place.
  //! \tparam T2 Scalar type.
  //! \param rhs Scalar to modulo by.
  //! \return Result of modulo.
  template<typename T2>
  GenericQuantityArray& operator%=(const T2& rhs) {
    _apply_binary_op_inplace_combine(rhs, kBinaryOpIDModulo);
    return *this;
  }
  //! \brief Modulo operator.
  //! \tparam T2 Type of rhs of operator.
  //! \param rhs Right hand side of operator.
  //! \return Result of modulo.
  DELEGATE_TO_INPLACE_OP_SCALAR_(GENERIC_QUANTITY_ARRAY_TYPE, %, %=)
  DELEGATE_TO_INPLACE_OP_QUANTITY_(GENERIC_QUANTITY_ARRAY_TYPE,
                                   GENERIC_QUANTITY_ARRAY_TYPE, %, %=)
  DEFINE_OP_DEFAULT_CLASS(QuantityArray, GENERIC_QUANTITY_ARRAY_TYPE, %=)
  DEFINE_OP_DEFAULT_CLASS(Quantity, GENERIC_QUANTITY_ARRAY_TYPE, %=)
  //! \brief Add a quantity with compatible units & shape in place.
  //! \tparam T2 Type of elements in rhs.
  //! \tparam Encoding2 Encoding of units in rhs array.
  //! \param rhs Array to add.
  //! \return Result of addition.
  //!
  //! If rhs has the same shape as this array, elements will be added on
  //!   an element-by-element basis. If rhs is a single element, it will
  //!   be added to each element in this array. If this array is a single
  //!   element and rhs is not, this array will be modified into a
  //!   a uniform array with the same shape as rhs before performing the
  //!   operation.
  template<typename T2, typename Encoding2>
  GenericQuantityArray& operator+=(const GenericQuantityArray<T2, Encoding2>& rhs) {
    _apply_binary_op_inplace_convert(rhs, kBinaryOpIDAdd);
    return *this;
  }
  DELEGATE_TO_INPLACE_OP_QUANTITY_(GENERIC_QUANTITY_ARRAY_TYPE,
                                   GENERIC_QUANTITY_ARRAY_TYPE, +, +=)
  DEFINE_OP_DEFAULT_CLASS(QuantityArray, GENERIC_QUANTITY_ARRAY_TYPE, +=)
  DEFINE_OP_DEFAULT_CLASS(Quantity, GENERIC_QUANTITY_ARRAY_TYPE, +=)
  //! \brief Subtract a quantity with compatible units & shape in place.
  //! \tparam T2 Type of elements in rhs.
  //! \tparam Encoding2 Encoding of units in rhs array.
  //! \param rhs Array to subtract.
  //! \return Result of subtraction.
  //!
  //! If rhs has the same shape as this array, elements will be subtracted
  //!   on an element-by-element basis. If rhs is a single element, it
  //!   will be subtracted to each element in this array. If this
  //!   array is a single element and rhs is not, this array will be
  //!   modified into a a uniform array with the same shape as rhs
  //!   before performing the operation.
  template<typename T2, typename Encoding2>
  GenericQuantityArray& operator-=(const GenericQuantityArray<T2, Encoding2>& rhs) {
    _apply_binary_op_inplace_convert(rhs, kBinaryOpIDSubtract);
    return *this;
  }
  DELEGATE_TO_INPLACE_OP_QUANTITY_(GENERIC_QUANTITY_ARRAY_TYPE,
                                   GENERIC_QUANTITY_ARRAY_TYPE, -, -=)
  DEFINE_OP_DEFAULT_CLASS(QuantityArray, GENERIC_QUANTITY_ARRAY_TYPE, -=)
  DEFINE_OP_DEFAULT_CLASS(Quantity, GENERIC_QUANTITY_ARRAY_TYPE, -=)
  //! \brief Perform floor operation in place.
  //! \return Result of floor.
  GenericQuantityArray& floor_inplace() {
    if (YGGDRASIL_IS_FLOAT_TYPE(ScalarType)::Value) {
      SizeType N = nelements();
      for (SizeType i = 0; i < N; i++) {
	value_[i] = internal::value_floor(value_[i]);
      }
    }
    return *this;
  }
  //! \brief Perform floor operation on the quantity value.
  //! \return Result of floor.
  GenericQuantityArray floor() const {
    GenericQuantityArray<T, Encoding> out(*this);
    out.floor_inplace();
    return out;
  }
  //! \brief Perform power operation in place.
  //! \param x Power to raise this quantity to.
  template<typename T2>
  GenericQuantityArray& pow_inplace(const T2& x,
				    RAPIDJSON_DISABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T2)))) {
    units_.pow_inplace(x);
    double factor = units_.pull_factor();
    if (canCast<double, ScalarType>()) {
      for (SizeType i = 0; i < nelements(); i++) {
        value_[i] = castPrecision<double,ScalarType>(factor * std::pow(value_[i], x));
      }
    } else if (isInt<ScalarType>()) {
      for (SizeType i = 0; i < nelements(); i++) {
        value_[i] = castPrecision<int,ScalarType>(static_cast<int>(factor * std::pow(value_[i], x)));
      }
    } else if (isUint<ScalarType>()) {
      for (SizeType i = 0; i < nelements(); i++) {
        value_[i] = castPrecision<unsigned int,ScalarType>(static_cast<unsigned int>(factor * std::pow(value_[i], x)));
      }
    }
    return *this;
  }
  //! \brief Raise this quantity to a power.
  //! \param x Power to raise this quantity to.
  //! \return Resulting quantity.
  template<typename T2>
  GenericQuantityArray<T, Encoding> pow(const T2& x,
                                        RAPIDJSON_DISABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T2)))) const {
    GenericQuantityArray<T, Encoding> out(*this);
    out.pow_inplace(x);
    return out;
  }
  //! \brief Explicity copy.
  //! \return Copy.
  GenericQuantityArray<ScalarType, Encoding>* copy() const {
    return new GenericQuantityArray<ScalarType, Encoding>(*this);
  }
  //! \brief Explicity copy and cast to void pointer.
  //! \return Copy.
  void* copy_void() const { return (void*)copy(); }
  //! \brief Get the dimensions of this quantity's units.
  //! \return The dimensions of the units.
  Dimension dimension() const { return units_.dimension(); }
  //! \brief Determine if the quantity's units are dimensionless.
  //! \return true if the units are dimensionless, false otherwise.
  bool is_dimensionless() const { return units_.is_dimensionless(); }
  //! \brief Check if another quantity has compatible units with the same
  //!   dimensions.
  //! \param x QuantityArray for comparison.
  //! \return true if the units are compatible, false otherwise.
  template<typename T2>
  bool is_compatible(const GenericQuantityArray<T2, Encoding>& x) const {
    return (dimension() == x.dimension() &&
	    units_.get_delta() == x.units_.get_delta());
  }
  //! \brief Check if a set of units is compatible.
  //! \param x Units for comparison.
  //! \return true if the units are compatible, false otherwise.
  bool is_compatible(const GenericUnits<Encoding>& x) const {
    return (dimension() == x.dimension() &&
	    units_.get_delta() == x.get_delta());
  }
  //! \brief Check if another quantity is equivalent to this one, allowing
  //!    for the possibility that it has different, but compatible, units.
  //! \param x QuantityArray for comparison.
  //! \return true if the two quantities are equivalent, false otherwise.
  template<typename T2>
  bool equivalent_to(const GenericQuantityArray<T2, Encoding>& x) {
    if (!(is_compatible(x)))
      return false;
    return (*this==x.as(units_));
  }
  //! \brief Apply a conversion factor to each element in the array.
  //! \param factor Scale and offset to be applied.
  void apply_conversion_factor(std::vector<double> factor) {
    SizeType N = nelements();
    for (SizeType i = 0; i < N; i++)
      value_[i] = do_conv<ScalarType>(value_[i], factor[0], factor[1]);
  }
  //! \brief Convert the quantity to a different set of units. The new units
  //!   must be compatible with the current ones.
  //! \param units New units.
  void convert_to(const UnitsType& units) {
    std::vector<double> factor = units_.convert_to(units);
    apply_conversion_factor(factor);
  }
  //! \brief Convert quantity to the system used by another set of units in
  //!   place.
  //! \param unitSystem Unit system to convert to.
  void convert_to_units_system(const UnitsType& unitSystem) {
    // UnitsType new_units = units_.as_units_system(unitSystem);
    // std::vector<double> factor = units_.convert_to(new_units);
    std::vector<double> factor = units_.convert_to_units_system(unitSystem);
    apply_conversion_factor(factor);
  }
  //! \brief Create a new quantity by converting this one to a new set of
  //!   compatible units.
  //! \param units New units.
  //! \return New quantity.
  GenericQuantityArray as(const char* units0) const {
    UnitsType units(units0);
    return as(units);
  }
  //! \brief Create a new quantity by converting this one to a new set of
  //!   compatible units.
  //! \param units New units.
  //! \return New quantity.
  GenericQuantityArray as(const UnitsType& units) const {
    GenericQuantityArray out(*this);
    out.convert_to(units);
    return out;
  }
  //! \brief Create a new quantity by converting to a new units system.
  //! \param units New unit system.
  //! \return New quantity.
  GenericQuantityArray as_units_system(const UnitsType& units) const {
    GenericQuantityArray out(*this);
    out.convert_to_units_system(units);
    return out;
  }

private:
  friend class GenericQuantity<ScalarType, EncodingType>;
  std::vector<T> value_;
  UnitsType units_;
  std::vector<SizeType> shape_;
  template<typename Ch2, typename U, typename Encoding2>
  friend std::basic_ostream<Ch2> & operator << (std::basic_ostream<Ch2> &os, const GenericQuantityArray<U,Encoding2> &x);

  bool _is_singular() const { return false; }

  template<typename Tout, typename T2>
  GenericQuantityArray<Tout, Encoding>
  _copy_for_op(const T2&,
               RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T2)))) const {
    GenericQuantityArray<Tout, Encoding> out(*this);
    return out;
  }
  template<typename Tout, typename T2, typename Encoding2>
  GenericQuantityArray<Tout, Encoding>
  _copy_for_op(const GenericQuantityArray<T2, Encoding2>& rhs) const {
    GenericQuantityArray<Tout, Encoding> out;
    if (this->nelements() == 1) {
      GenericQuantityArray<Tout, Encoding> out(
        value_[0], rhs.ndim(), rhs.shape(), units_);
      RAPIDJSON_ASSERT(out.is_same_shape(rhs));
      return out;
    } else {
      GenericQuantityArray<Tout, Encoding> out(*this);
      RAPIDJSON_ASSERT(out.is_same_shape(rhs) || (rhs.nelements() == 1));
      return out;
    }
  }

  template<typename T2>
  T _apply_binary_op_inplace_units(const GenericQuantityArray<T2, Encoding>& x,
                                   const ValueBinaryOp& op) {
    RAPIDJSON_ASSERT(!op.requires_scalar());
    RAPIDJSON_ASSERT((op.id == kBinaryOpIDMultiply) ||
                     (op.id == kBinaryOpIDDivide));
    switch (op.id) {
    case (kBinaryOpIDMultiply): {
      units_ *= x.units();
      break;
    }
    case (kBinaryOpIDDivide): {
      units_ /= x.units();
      break;
    }
    default: {
      return castPrecision<double,T>(1.0);
    }
    }
    double factor = units_.pull_factor();
    return castPrecision<double,T>(factor);
  }
  
  template<typename T2>
  bool _apply_binary_op_inplace(const T2& x, const ValueBinaryOp& op,
                                bool direct=false,
                                RAPIDJSON_ENABLEIF((YGGDRASIL_IS_CASTABLE(T2, ScalarType)))) {
    RAPIDJSON_ASSERT(direct || !op.requires_convert());
    if (op.requires_convert() && !direct)
      return false;
    T x_cast = castPrecision<T2,ScalarType>(x);
    for (SizeType i = 0; i < nelements(); i++) {
      op.op_inplace(value_[i], x_cast);
    }
    return true;
  }

  template<typename T2>
  bool _apply_binary_op_inplace(const GenericQuantityArray<T2, Encoding>& x,
                                const ValueBinaryOp& op,
                                bool direct=false,
                                RAPIDJSON_ENABLEIF((YGGDRASIL_IS_CASTABLE(T2, ScalarType)))) {
    RAPIDJSON_ASSERT(is_same_shape(x) ||   // (nelements() == 1) ||
                     (x.nelements() == 1));
    RAPIDJSON_ASSERT(direct || !op.requires_convert());
    if (op.requires_convert() && !direct)
      return false;
    SizeType N = nelements();
    if (is_same_shape(x)) {
      for (SizeType i = 0; i < N; i++) {
        op.op_inplace(value_[i], x.value()[i]);
      }
    // } else if (N == 1) {
    //   T value0 = value_[0];
    //   *this = x;
    //   N = nelements();
    //   for (SizeType i = 0; i < N; i++) {
    //     value_[i] = value0;
    //   }
    //   return _apply_binary_op_inplace(x, op, direct);
    } else if (x.nelements() == 1) {
      return _apply_binary_op_inplace(x.value()[0], op, true);
    } else {
      return false;
    }
    return true;
  }

  template<typename Tout, typename T2>
  GenericQuantityArray<Tout, EncodingType>
  _apply_binary_op_combine(const T2& x, const ValueBinaryOp& op,
                           RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T2)))) const {
    GenericQuantityArray<Tout, EncodingType> out = this->_copy_for_op<Tout>(x);
    out._apply_binary_op_inplace_combine(x, op);
    return out;
  }
  template<typename Tout, typename T2, typename Encoding2>
  GenericQuantityArray<Tout, EncodingType>
  _apply_binary_op_combine(const GenericQuantityArray<T2, Encoding2>& x,
                           const ValueBinaryOp& op) const {
    GenericQuantityArray<Tout, EncodingType> out = this->_copy_for_op<Tout>(x);
    out._apply_binary_op_inplace_combine(x, op);
    return out;
  }
  template<typename Tout, typename T2, typename Encoding2>
  GenericQuantityArray<Tout, EncodingType>
  _apply_binary_op_convert(const GenericQuantityArray<T2, Encoding2>& x,
                           const ValueBinaryOp& op) const {
    GenericQuantityArray<Tout, EncodingType> out = this->_copy_for_op<Tout>(x);
    out._apply_binary_op_inplace_convert(x, op);
    return out;
  }

  // Inplace
  template<typename T2>
  bool _apply_binary_op_inplace_combine(const T2& x, const ValueBinaryOp& op,
                                        RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T2)))) {
    return _apply_binary_op_inplace(x, op);
  }

  // template<typename T2, typename Encoding2>
  // bool _apply_binary_op_inplace_combine(const GenericQuantityArray<T2, Encoding2>& x,
  //                                       const ValueBinaryOp& op,
  //                                       RAPIDJSON_DISABLEIF((internal::IsSame<Encoding, Encoding2>))) {
  //   GenericQuantityArray<T2, Encoding> tmp = x.template transcode<Encoding>();
  //   return _apply_binary_op_inplace_combine(tmp, op);
  // }
  template<typename T2, typename Encoding2>
  bool _apply_binary_op_inplace_combine(const GenericQuantityArray<T2, Encoding2>& x,
                                        const ValueBinaryOp& op,
                                        RAPIDJSON_ENABLEIF((YGGDRASIL_IS_CASTABLE(T2, ScalarType)))) {
    RAPIDJSON_ASSERT(!op.requires_convert());
    if (op.id == kBinaryOpIDModulo) {
      GenericQuantityArray<ScalarType, Encoding> val = *this / x;
      val.floor_inplace();
      val *= x;
      *this -= val;
      return true;
    }
    T factor = _apply_binary_op_inplace_units(x, op);
    bool out = _apply_binary_op_inplace(x, op);
    for (SizeType i = 0; i < nelements(); i++) {
      value_[i] *= factor;
    }
    return out;
  }

  // template<typename T2, typename Encoding2>
  // bool _apply_binary_op_inplace_convert(const GenericQuantityArray<T2, Encoding2>& x,
  //                                       const ValueBinaryOp& op,
  //                                       bool direct=false,
  //                                       RAPIDJSON_DISABLEIF((internal::IsSame<Encoding, Encoding2>))) {
  //   GenericQuantityArray<T2, Encoding> tmp = x.template transcode<Encoding>();
  //   return _apply_binary_op_inplace_convert(tmp, op, direct);
  // }
  template<typename T2, typename Encoding2>
  bool _apply_binary_op_inplace_convert(const GenericQuantityArray<T2, Encoding2>& x,
                                        const ValueBinaryOp& op,
                                        bool direct=false,
                                        RAPIDJSON_ENABLEIF((YGGDRASIL_IS_CASTABLE(T2, ScalarType)))) {
    RAPIDJSON_ASSERT(op.requires_convert());
    if (!direct) {
      GenericQuantityArray<T2, Encoding2> x2 = x.as_units_system(units_);
      bool out = _apply_binary_op_inplace_convert(x2, op, true);
      if (out) {
        if ((units_.has_difference() && x2.units().is_difference()) ||
            (units_.is_difference() && x2.units().has_difference()))
          units_.set_delta(kInactiveDelta);
        else if ((op.id == kBinaryOpIDSubtract) &&
                 units_.has_difference())
          units_.set_delta(kActiveDelta);
      }
      return out;
    }
    return _apply_binary_op_inplace(x, op, direct);
  }
  
  FRIEND_DEFAULT_(QuantityArray)
};
//! Output stream operator for GenericQuantityArray
/*!
  \tparam Ch Type used to represent characters in the output stream.
  \tparam T Type of elements in x.
  \tparam Encoding Encoding of the units in x.
  \param os Output stream.
  \param[in] x Array to output.
  \returns Output stream.
 */
template<typename Ch, typename T, typename Encoding>
inline std::basic_ostream<Ch> & operator << (std::basic_ostream<Ch> &os, const GenericQuantityArray<T, Encoding> &x) {
  x._write_array(os);
  os << ' ' << x.units_;
  return os;
}

//! GenericQuantityArray with UTF8 encoding
CREATE_DEFAULT_(QuantityArray, QUANTITY_ARRAY_TYPE,
		GenericQuantityArray, GENERIC_QUANTITY_ARRAY_TYPE,
		UTF8<char>, INHERIT_CONSTRUCTORS_ARRAY_);
  
//! Scalar quantity with units.
//! \tparam T Type of the underlying scalar.
//! \tparam Encoding Encoding used to store the unit strings.
template<typename T, typename Encoding>
class GenericQuantity : public GenericQuantityArray<T, Encoding> {
public:
  typedef GenericQuantityArray<T, Encoding> Base;
  typedef Encoding EncodingType;            //!< Encoding type from template parameter.
  typedef typename Encoding::Ch Ch;         //!< Character type from encoding.
  typedef GenericUnits<Encoding> UnitsType; //!< Units type.
  typedef T ScalarType;                     //!< Scalar type.
  //! \brief Empty constructor.
  GenericQuantity() : Base(_initialize_value<ScalarType>()) {}
  //! \brief Copy constructor.
  //! \param rhs GenericQuantity to copy.
  GenericQuantity(const GenericQuantity<ScalarType, EncodingType>& rhs) :
    Base(rhs) {}
  //! \brief Copy constructor for quantity with different type/encoding.
  //! \tparam T2 Type of elements in rhs.
  //! \tparam Encoding2 Encoding of units in rhs.
  //! \param rhs GenericQuantity to copy.
  template<typename T2, typename Encoding2>
  GenericQuantity(const GenericQuantity<T2, Encoding2>& rhs,
                  RAPIDJSON_DISABLEIF((internal::OrExpr<
                                       internal::AndExpr<
                                       internal::IsSame<T2, ScalarType>,
                                       internal::IsSame<Encoding2, EncodingType> >,
                                       internal::NotExpr<YGGDRASIL_IS_CASTABLE(T2, ScalarType) > >))) :
    Base(rhs) {}
  //! \brief Create a quantity without units.
  //! \param value Scalar value.
  GenericQuantity(const ScalarType& value) :
    Base(value) {}
  //! \brief Constructor from units string.
  //! \param value Scalar value.
  //! \param units Units string.
  GenericQuantity(const ScalarType& value, const Ch* units) :
    Base(value, units) {}
  //! \brief Constructor from units string.
  //! \param value Scalar value.
  //! \param units Units instance.
  GenericQuantity(const ScalarType& value, const UnitsType& units) :
    Base(value, units) {}
  //! \brief Copy assignment from array with different type/encoding.
  //! \tparam T2 Type of elements in rhs.
  //! \tparam Encoding2 Encoding of units in rhs.
  //! \param rhs GenericQuantityArray to copy.
  template<typename T2, typename Encoding2>
  RAPIDJSON_DISABLEIF_RETURN((internal::AndExpr<
                              internal::IsSame<T2, ScalarType>,
                              internal::IsSame<Encoding2, EncodingType> >),
                             (GenericQuantity&))
  operator=(const GenericQuantity<T2, Encoding2>& rhs) {
    GenericQuantityArray<ScalarType, EncodingType>::operator=(rhs);
    return *this;
  }
  //! \brief Print instance information to an output stream.
  //! \param os Output stream.
  template<typename Ch2>
  void display(std::basic_ostream<Ch2>& os) const {
    os << 'Q' << 'u' << 'a' << 'n' << 't' << 'i' << 't' << 'y' <<
      '(' << value() << ',' << ' ' << '\"';
    os << this->units();
    os << '\"' << ')';
  }
  //! \brief Get the quantity value without units.
  //! \return Value.
  ScalarType value() const { return Base::value()[0]; }
  //! \brief Set the quantity value.
  //! \param new_value New quantity value.
  template<typename T2>
  void set_value(const T2& new_value,
		 RAPIDJSON_ENABLEIF((YGGDRASIL_IS_CASTABLE(T2, ScalarType))))
  {
    SizeType N = 1;
    Base::set_value(&new_value, 1, &N);
  }
  //! \brief Arithmetic operators
  INHERIT_OPERATORS_(GenericQuantity, GENERIC_QUANTITY_TYPE, GENERIC_QUANTITY_ARRAY_TYPE)
  
private:
  template<typename T1>
  static T1 _initialize_value(RAPIDJSON_DISABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T1))))
  { return (T1)(0); }
  template<typename T1>
  static T1 _initialize_value(RAPIDJSON_ENABLEIF((YGGDRASIL_IS_COMPLEX_TYPE(T1))))
  { return T1(0.0, 0.0); }
  
  template<typename Ch2, typename U, typename Encoding2>
  friend std::basic_ostream<Ch2> & operator << (std::basic_ostream<Ch2> &os, const GenericQuantity<U,Encoding2> &x);

  bool _is_singular() const { return true; }

  template<typename Tout, typename T2>
  GenericQuantity<Tout, Encoding>
  _copy_for_op(const T2&,
               RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T2)))) const {
    GenericQuantity<Tout, Encoding> out(*this);
    return out;
  }
  template<typename Tout, typename T2, typename Encoding2>
  GenericQuantity<Tout, Encoding>
  _copy_for_op(const GenericQuantity<T2, Encoding2>&) const {
    GenericQuantity<Tout, Encoding> out(*this);
    return out;
  }
  template<typename Tout, typename T2, typename Encoding2>
  GenericQuantityArray<Tout, Encoding>
  _copy_for_op(const GenericQuantityArray<T2, Encoding2>& rhs) const {
    return GenericQuantityArray<ScalarType, Encoding>::template _copy_for_op<Tout>(rhs);
  }
  
  FRIEND_DEFAULT_(Quantity)

  template<typename Tout, typename T2>
  GenericQuantity<Tout, EncodingType>
  _apply_binary_op_combine(const T2& x, const ValueBinaryOp& op,
                           RAPIDJSON_ENABLEIF((YGGDRASIL_IS_CASTABLE(T2, ScalarType)))) const {
    GenericQuantity<Tout, EncodingType> out = this->_copy_for_op<Tout>(x);
    out._apply_binary_op_inplace_combine(x, op);
    return out;
  }
  template<typename Tout, typename T2, typename Encoding2>
  GenericQuantity<Tout, EncodingType>
  _apply_binary_op_combine(const GenericQuantity<T2, Encoding2>& x,
                           const ValueBinaryOp& op) const {
    GenericQuantity<Tout, EncodingType> out = this->_copy_for_op<Tout>(x);
    out._apply_binary_op_inplace_combine(x, op);
    return out;
  }
  template<typename Tout, typename T2, typename Encoding2>
  GenericQuantity<Tout, EncodingType>
  _apply_binary_op_convert(const GenericQuantity<T2, Encoding2>& x,
                           const ValueBinaryOp& op) const {
    GenericQuantity<Tout, EncodingType> out = this->_copy_for_op<Tout>(x);
    out._apply_binary_op_inplace_convert(x, op);
    return out;
  }
  
};
//! Output stream operator for GenericQuantity
/*!
  \tparam Ch Type used to represent characters in the output stream.
  \tparam T Type of elements in x.
  \tparam Encoding Encoding of the units in x.
  \param os Output stream.
  \param[in] x Quantity to output.
  \returns Output stream.
 */
template<typename Ch, typename T, typename Encoding>
inline std::basic_ostream<Ch> & operator << (std::basic_ostream<Ch> &os, const GenericQuantity<T, Encoding> &x) {
  os << x.value() << ' ' << x.units();
  return os;
}

//! GenericQuantity with UTF8 encoding
CREATE_DEFAULT_(Quantity, QUANTITY_TYPE,
		GenericQuantity, GENERIC_QUANTITY_TYPE,
		UTF8<char>, INHERIT_CONSTRUCTORS_);

  //! Perform unit conversion on raw array bytes.
  /*!
    \tparam T Type of elements in the source array.
    \tparam Encoding Encoding of the source units.
    \param[in] src_bytes Pointer to the raw bytes containing the source
      array.
    \param[in] src_units Units that the source array is in.
    \param[out] dst_bytes Pointer to pre-allocated memory where the
      converted array should be stored.
    \param[in] dst_units Units that the source array should be converted
      to in the destination.
    \param[in] nbytes Size of the source array in bytes.
    \param nelements Number of elements of type T in the source array.
      If not provided, the number of elements will be determined from
      nbytes.
    \pre nelements == (nbytes / (SizeType)sizeof(T))
    \pre !(nbytes % (SizeType)sizeof(T))
   */
  template <typename T, typename Encoding>
  void changeUnits(const unsigned char* src_bytes,
		   const GenericUnits<Encoding>& src_units,
		   unsigned char* dst_bytes,
		   const GenericUnits<Encoding>& dst_units,
		   const SizeType nbytes,
		   SizeType nelements=0) {
    if (nelements == 0)
      nelements = nbytes / (SizeType)sizeof(T);
    else
      RAPIDJSON_ASSERT(nelements == (nbytes / (SizeType)sizeof(T)));
    RAPIDJSON_ASSERT(!(nbytes % (SizeType)sizeof(T)));
    GenericQuantityArray<T, Encoding> qa((T*)src_bytes, nelements, src_units);
    qa.convert_to(dst_units);
    memcpy(dst_bytes, qa.value(), (size_t)nelements * sizeof(T));
  }

  //! Perform unit conversion on raw array bytes base on a subtype.
  /*!
    \tparam Encoding Encoding of the source units.
    \param[in] subtype Subtype of elements in the source array.
    \param[in] precision Number of bytes in each element of the source
      array.
    \param[in] src_bytes Pointer to the raw bytes containing the source
      array.
    \param[in] src_units Units that the source array is in.
    \param[out] dst_bytes Pointer to pre-allocated memory where the
      converted array should be stored.
    \param[in] dst_units Units that the source array should be converted
      to in the destination.
    \param[in] nbytes Size of the source array in bytes.
    \param nelements Number of elements of type T in the source array.
      If not provided, the number of elements will be determined from
      nbytes.
    \pre nelements == (nbytes / (SizeType)sizeof(T))
    \pre !(nbytes % (SizeType)sizeof(T))
   */
  template <typename Encoding>
  void changeUnits(YggSubType subtype, SizeType precision,
		   const unsigned char* src_bytes,
		   const GenericUnits<Encoding>& src_units,
		   unsigned char* dst_bytes,
		   const GenericUnits<Encoding>& dst_units,
		   const SizeType nbytes,
		   const SizeType nelements=0) {
    SWITCH_SUBTYPE(subtype, precision, changeUnits, PACK_MACRO(Encoding),
		   (src_bytes, src_units, dst_bytes, dst_units, nbytes, nelements),
		   RAPIDJSON_ASSERT(false));
  }
  
#undef ARRAY_ARRAY_OP
#undef ARRAY_SCALAR_OP
#undef PACK_LUT
#undef DEFINE_OP_DEFAULT_CLASS
#undef DELEGATE_TO_INPLACE_OP_SCALAR_
#undef DELEGATE_TO_INPLACE_OP_QUANTITY_
#undef DELEGATE_TO_INPLACE_OP_FRIEND_
#undef DELEGATE_OP_FRIEND_REVERSE_
#undef DELEGATE_TO_INPLACE_METHOD_SCALAR_
#undef DELEGATE_TO_INPLACE_METHOD_QUANTIY_
#undef DELEGATE_TO_INPLACE_NOARGS_
#undef DELEGATE_TO_INPLACE_NOCAST_
#undef INHERIT_INPLACE_NOARGS_
#undef INHERIT_INPLACE_NOCAST_
#undef INHERIT_INPLACE_METHOD_
#undef INHERIT_NOARGS_
#undef INHERIT_NOCAST_
#undef INHERIT_METHOD_
#undef INHERIT_OP_
#undef INHERIT_CONSTRUCTORS_
#undef INHERIT_CONSTRUCTORS_ARRAY_
#undef INHERIT_OPERATORS_
#undef PASS
#undef GENERIC_QUANTITY_ARRAY_TYPE
#undef GENERIC_QUANTITY_TYPE
#undef QUANTITY_ARRAY_TYPE
#undef QUANTITY_TYPE
#undef FRIEND_DEFAULT_
#undef CREATE_DEFAULT_

} // namespace units

#undef OVERRIDE_CXX11
  
#endif // RAPIDJSON_YGGDRASIL

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_UNITS_H_
