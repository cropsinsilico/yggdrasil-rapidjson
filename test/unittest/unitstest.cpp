// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied. See the License for the 
// specific language governing permissions and limitations under the License.

#ifdef RAPIDJSON_YGGDRASIL

#define RAPIDJSON_SCHEMA_VERBOSE 0
#define RAPIDJSON_HAS_STDSTRING 1

#include "unittest.h"
#include "rapidjson/schema.h"
#include "rapidjson/units.h"
#include "rapidjson/encodings.h"
#include "rapidjson/error/error.h"
#include "rapidjson/error/en.h"

#ifdef __clang__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(variadic-macros)
#elif defined(_MSC_VER)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(4822) // local class member function does not have a body
#endif

using namespace rapidjson;

template<typename T>
inline bool compare(const T& a, const T& b,
                    RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T)))) {
  return internal::values_eq(a, b);
}
template<typename T>
inline bool compare(const T& a, const T& b,
                    RAPIDJSON_DISABLEIF((YGGDRASIL_IS_ANY_SCALAR(T)))) {
  return (a == b);
}
template<typename T>
inline bool compare_not(const T& a, const T& b,
                        RAPIDJSON_ENABLEIF((YGGDRASIL_IS_ANY_SCALAR(T)))) {
  return (!internal::values_eq(a, b));
}
template<typename T>
inline bool compare_not(const T& a, const T& b,
                        RAPIDJSON_DISABLEIF((YGGDRASIL_IS_ANY_SCALAR(T)))) {
  return (a != b);
}

#define CHECK_QUANTITY_EQUIVALENCE(a, b, expected)			\
  {									\
    if (a.equivalent_to(b) != expected) {				\
      std::cerr << #a << " = " << a << ", " << #b << " = " << b;        \
      if (a.is_compatible(b)) {						\
	std::cerr << ", " << #b << ".as(" << #a << ".units) = " << b.as(a.units()) << ", " << #a << ".value - " << #b << ".value = " << std::abs(b.value() - a.as(b.units()).value()) << ", " << #b << ".value - " << #a << ".value = " << std::abs(a.value() - b.as(a.units()).value()) << ", " << internal::values_eq(b.value(), a.as(b.units()).value()) << ", " << internal::values_eq(a.value(), b.as(a.units()).value()); \
      }	else if (expected) {						\
	std::cerr << ", not compat, " << #a << ".dim = " << a.dimension() << ", " << #b << ".dim = " << b.dimension(); \
      }									\
      std::cerr << std::endl;						\
    }									\
    EXPECT_TRUE(a.equivalent_to(b) == expected);			\
  }

#define CHECK_QUANTITY_DIRECT_EQUALITY(a, b, expected)			\
  {									\
    if (compare(a, b) != expected) {                                    \
      std::cerr << #a << " = " << a << ", " << #b << " = " << b << std::endl; \
    }									\
    EXPECT_TRUE(compare(a, b) == expected);                             \
    EXPECT_FALSE(compare_not(a, b) == expected);                        \
  }

#define COMPARE_UNITS(av, au, bv, bu, expected, direct)			\
  {									\
    units::Quantity<double> a(av, au);			\
    units::Quantity<double> b(bv, bu);			\
    CHECK_QUANTITY_EQUIVALENCE(a, b, expected);				\
    CHECK_QUANTITY_EQUIVALENCE(b, a, expected);				\
    CHECK_QUANTITY_DIRECT_EQUALITY(a, b, direct);			\
    CHECK_QUANTITY_DIRECT_EQUALITY(b, a, direct);			\
    if (expected) {							\
      EXPECT_TRUE(internal::values_eq(a.value(), b.as(a.units()).value())); \
      EXPECT_TRUE(internal::values_eq(b.value(), a.as(b.units()).value())); \
    }									\
  }

TEST(Unit, Base) {
  units::Quantity<double> x(1.0, "kg");
  std::cout << "x = " << x << ", dim = " << x.units().dimension() << std::endl;
  COMPARE_UNITS(1.0, "g", 0.001, "kg", true, false);
  COMPARE_UNITS(1.0, "gram", 0.001, "kilogram", true, false);
  COMPARE_UNITS(1.0, "grams", 0.001, "kilograms", true, false);
  COMPARE_UNITS(1.0, "g", 1.0, "kg", false, false);
  COMPARE_UNITS(1.0, "cm", 1.0, "g", false, false);
  COMPARE_UNITS(1.0, "cm", 1.0, "cm/s", false, false);
  COMPARE_UNITS(1.0, "g**2", 1e-6, "kg^2", true, false);
  COMPARE_UNITS(1.0, "hp", 745.69987158227022, "W", true, false);
  COMPARE_UNITS(1.0, "km/s", 2236.936292054402, "mi/hr", true, false);
  COMPARE_UNITS(0.0, "°C", 273.15, "K", true, false);
  COMPARE_UNITS(32.0, "°F", 0.0, "°C", true, false);
  COMPARE_UNITS(41.0, "°F", 5.0, "°C", true, false);
  COMPARE_UNITS(1.0, "Δ°C", 1.0, "ΔK", true, false);
  COMPARE_UNITS(1.0, "Δ°C", 9.0/5.0, "Δ°F", true, false);
  COMPARE_UNITS(1.0, "km s", 1.0, "km*s", true, true);
  COMPARE_UNITS(1.0, "g**(1+1)", 1.0, "g^2", true, true);
  COMPARE_UNITS(1.0, "g**(3-1)", 1.0, "g^2", true, true);
  COMPARE_UNITS(1.0, "g**(6/3)", 1.0, "g^2", true, true);
  COMPARE_UNITS(1.0, "g**(1*2)", 1.0, "g^2", true, true);
  COMPARE_UNITS(1.0, "(km**2)(s**-1)", 1.0, "km**2/s", true, true);
  COMPARE_UNITS(1.0, "(km*A)**2/((s**2)(g**3))", 1.0, "(km^2)*(A^2)*(s^-2)*(g^-3)", true, true);
  COMPARE_UNITS(1.0, "%", 1.0, "percent", true, true);
  COMPARE_UNITS(0.01, "100%", 1.0, "percent", true, false);
  const UTF8<char>::Ch test_units[] = "(km*A)**2/((s**2)(g**3))";
  units::Units(test_units, strlen(test_units), true);
};

#define CACHE_VAR(T, name)                      \
  T name##0 = name;                             \
  UNUSED(name##0)

#define CHECK_METHOD(op, a, b, cT, cv)                                  \
  {                                                                     \
    cT expected cv;                                                     \
    cT actual = a.op(b);                                                \
    CHECK_QUANTITY_DIRECT_EQUALITY(a ## 0, a, true);                    \
    CHECK_QUANTITY_EQUIVALENCE(expected, actual, true);                 \
    CHECK_QUANTITY_DIRECT_EQUALITY(expected, actual, true);             \
  }
#define CHECK_IMETHOD(op, a, b, cT, cv)                                 \
  {                                                                     \
    cT expected cv;                                                     \
    cT actual(a);                                                       \
    actual.op ## _inplace(b);                                           \
    CHECK_QUANTITY_DIRECT_EQUALITY(a ## 0, a, true);                    \
    CHECK_QUANTITY_EQUIVALENCE(expected, actual, true);                 \
    CHECK_QUANTITY_DIRECT_EQUALITY(expected, actual, true);             \
  }
#define CHECK_OP(op, a, b, cT, cv)                                      \
  {                                                                     \
    cT expected cv;                                                     \
    cT actual = a op b;                                                 \
    CHECK_QUANTITY_DIRECT_EQUALITY(a ## 0, a, true);                    \
    CHECK_QUANTITY_EQUIVALENCE(expected, actual, true);                 \
    CHECK_QUANTITY_DIRECT_EQUALITY(expected, actual, true);             \
  }
#define CHECK_IOP(op, a, b, cT, cv)                                     \
  {                                                                     \
    cT expected cv;                                                     \
    cT actual(a);                                                       \
    actual op ## = b;                                                   \
    CHECK_QUANTITY_DIRECT_EQUALITY(a ## 0, a, true);                    \
    CHECK_QUANTITY_EQUIVALENCE(expected, actual, true);                 \
    CHECK_QUANTITY_DIRECT_EQUALITY(expected, actual, true);             \
  }

#define OP_SETUP                                                \
  double sA = 1.0;                                              \
  int sB = 2;                                                   \
  units::Quantity<double> qA(1.0, "kg");                        \
  units::Quantity<double> qB(2.0, "cm");                        \
  units::Quantity<int> qC(1, "m");                              \
  units::Quantity<int> qD(1, "s");                              \
  units::Quantity<double> qF(5.0, "g");                         \
  units::QuantityArray<double> qarrA({1, 2, 3, 4}, "kg");       \
  units::QuantityArray<double> qarrB({1, 2, 3, 4}, "cm");       \
  units::QuantityArray<int> qarrC({1, 1, 1, 1}, "m");           \
  units::QuantityArray<int> qarrD({1, 1, 1, 1}, "s");           \
  units::QuantityArray<double> qarrF({5, 5, 5, 5}, "g");        \
  CACHE_VAR(double, sA);                                        \
  CACHE_VAR(int, sB);                                           \
  CACHE_VAR(units::Quantity<double>, qA);                       \
  CACHE_VAR(units::Quantity<double>, qB);                       \
  CACHE_VAR(units::Quantity<int>, qC);                          \
  CACHE_VAR(units::Quantity<int>, qD);                          \
  CACHE_VAR(units::Quantity<double>, qF);                       \
  CACHE_VAR(units::QuantityArray<double>, qarrA);               \
  CACHE_VAR(units::QuantityArray<double>, qarrB);               \
  CACHE_VAR(units::QuantityArray<int>, qarrC);                  \
  CACHE_VAR(units::QuantityArray<int>, qarrD);                  \
  CACHE_VAR(units::QuantityArray<double>, qarrF)
#define OP_SETUP_DELTA                                          \
  units::Quantity<double> qA(1.0, "degC");                      \
  units::Quantity<double> qB(1.0, "ΔdegC");                     \
  units::Quantity<double> qC(1.0, "K");                         \
  units::Quantity<double> qD(1.0, "ΔK");                        \
  units::QuantityArray<double> qarrA({1, 2, 3, 4}, "degC");     \
  units::QuantityArray<double> qarrB({1, 2, 3, 4}, "ΔdegC");    \
  units::QuantityArray<double> qarrC({1, 2, 3, 4}, "K");        \
  units::QuantityArray<double> qarrD({1, 2, 3, 4}, "ΔK");       \
  CACHE_VAR(units::Quantity<double>, qA);                       \
  CACHE_VAR(units::Quantity<double>, qB);                       \
  CACHE_VAR(units::Quantity<double>, qC);                       \
  CACHE_VAR(units::Quantity<double>, qD);                       \
  CACHE_VAR(units::QuantityArray<double>, qarrA);               \
  CACHE_VAR(units::QuantityArray<double>, qarrB);               \
  CACHE_VAR(units::QuantityArray<double>, qarrC);               \
  CACHE_VAR(units::QuantityArray<double>, qarrD)
  
TEST(Unit, Multiplication) {
  OP_SETUP;
  // Quantity + Scalar
  {
    CHECK_OP(*, qA, sA, units::Quantity<double>, (1.0, "kg"));
    CHECK_OP(*, sA, qA, units::Quantity<double>, (1.0, "kg"));
    CHECK_IOP(*, qA, sA, units::Quantity<double>, (1.0, "kg"));
    CHECK_OP(*, qA, sB, units::Quantity<double>, (2.0, "kg"));
    CHECK_OP(*, sB, qA, units::Quantity<double>, (2.0, "kg"));
    CHECK_IOP(*, qA, sB, units::Quantity<double>, (2.0, "kg"));
  }
  // Quantity + Quantity
  {
    CHECK_OP(*, qA, qA, units::Quantity<double>, (1.0, "kg**2"));
    CHECK_OP(*, qA, qB, units::Quantity<double>, (2.0, "kg*cm"));
    CHECK_OP(*, qB, qA, units::Quantity<double>, (2.0, "cm*kg"));
    CHECK_IOP(*, qA, qB, units::Quantity<double>, (2.0, "kg*cm"));
    CHECK_OP(*, qA, qC, units::Quantity<double>, (1.0, "kg*m"));
    CHECK_OP(*, qC, qA, units::Quantity<double>, (1.0, "kg*m"));
    CHECK_IOP(*, qA, qC, units::Quantity<double>, (1.0, "kg*m"));
  }
  // QuantityArray + Scalar
  {
    CHECK_OP(*, qarrA, sA, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg"));
    CHECK_OP(*, sA, qarrA, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg"));
    CHECK_IOP(*, qarrA, sA, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg"));
    CHECK_OP(*, qarrA, sB, units::QuantityArray<double>, ({2, 4, 6, 8}, "kg"));
    CHECK_OP(*, sB, qarrA, units::QuantityArray<double>, ({2, 4, 6, 8}, "kg"));
    CHECK_IOP(*, qarrA, sB, units::QuantityArray<double>, ({2, 4, 6, 8}, "kg"));
  }
  // QuantityArray + QuantityArray
  {
    CHECK_OP(*, qarrA, qarrA, units::QuantityArray<double>, ({1, 4, 9, 16}, "kg**2"));
    CHECK_OP(*, qarrA, qarrB, units::QuantityArray<double>, ({1, 4, 9, 16}, "kg*cm"));
    CHECK_OP(*, qarrB, qarrA, units::QuantityArray<double>, ({1, 4, 9, 16}, "cm*kg"));
    CHECK_IOP(*, qarrA, qarrB, units::QuantityArray<double>, ({1, 4, 9, 16}, "kg*cm"));
    CHECK_OP(*, qarrA, qarrC, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg*m"));
    CHECK_OP(*, qarrC, qarrA, units::QuantityArray<double>, ({1, 2, 3, 4}, "m*kg"));
    CHECK_IOP(*, qarrA, qarrC, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg*m"));
  }
  // QuantityArray + Quantity
  {
    CHECK_OP(*, qarrA, qA, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg**2"));
    CHECK_OP(*, qA, qarrA, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg**2"));
    CHECK_OP(*, qarrA, qB, units::QuantityArray<double>, ({2, 4, 6, 8}, "kg*cm"));
    CHECK_OP(*, qB, qarrA, units::QuantityArray<double>, ({2, 4, 6, 8}, "cm*kg"));
    CHECK_IOP(*, qarrA, qB, units::QuantityArray<double>, ({2, 4, 6, 8}, "kg*cm"));
    CHECK_OP(*, qarrA, qC, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg*m"));
    CHECK_OP(*, qC, qarrA, units::QuantityArray<double>, ({1, 2, 3, 4}, "m*kg"));
    CHECK_IOP(*, qarrA, qC, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg*m"));
  }
}

TEST(Unit, Division) {
  OP_SETUP;
  // Quantity + Scalar
  {
    CHECK_OP(/, qA, sA, units::Quantity<double>, (1.0, "kg"));
    CHECK_OP(/, sA, qA, units::Quantity<double>, (1.0, "kg**-1"));
    CHECK_IOP(/, qA, sA, units::Quantity<double>, (1.0, "kg"));
    CHECK_OP(/, qA, sB, units::Quantity<double>, (0.5, "kg"));
    CHECK_OP(/, sB, qA, units::Quantity<double>, (2.0, "kg**-1"));
    CHECK_IOP(/, qA, sB, units::Quantity<double>, (0.5, "kg"));
  }
  // Quantity + Quantity
  {
    CHECK_OP(/, qA, qA, units::Quantity<double>, (1.0, ""));
    CHECK_OP(/, qA, qB, units::Quantity<double>, (0.5, "kg/cm"));
    CHECK_OP(/, qB, qA, units::Quantity<double>, (2.0, "cm/kg"));
    CHECK_IOP(/, qA, qB, units::Quantity<double>, (0.5, "kg/cm"));
    CHECK_OP(/, qA, qC, units::Quantity<double>, (1.0, "kg/m"));
    CHECK_OP(/, qC, qA, units::Quantity<double>, (1.0, "m/kg"));
    CHECK_IOP(/, qA, qC, units::Quantity<double>, (1.0, "kg/m"));
  }
  // QuantityArray + Scalar
  {
    CHECK_OP(/, qarrA, sA, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg"));
    CHECK_OP(/, sA, qarrA, units::QuantityArray<double>, ({1, 0.5, 1.0/3.0, 0.25}, "kg**-1"));
    CHECK_IOP(/, qarrA, sA, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg"));
    CHECK_OP(/, qarrA, sB, units::QuantityArray<double>, ({0.5, 1, 1.5, 2}, "kg"));
    CHECK_OP(/, sB, qarrA, units::QuantityArray<double>, ({2, 1, 2.0/3.0, 0.5}, "kg**-1"));
    CHECK_IOP(/, qarrA, sB, units::QuantityArray<double>, ({0.5, 1, 1.5, 2}, "kg"));
  }
  // QuantityArray + QuantityArray
  {
    CHECK_OP(/, qarrA, qarrA, units::QuantityArray<double>, ({1, 1, 1, 1}, ""));
    CHECK_OP(/, qarrA, qarrB, units::QuantityArray<double>, ({1, 1, 1, 1}, "kg/cm"));
    CHECK_OP(/, qarrB, qarrA, units::QuantityArray<double>, ({1, 1, 1, 1}, "cm/kg"));
    CHECK_IOP(/, qarrA, qarrB, units::QuantityArray<double>, ({1, 1, 1, 1}, "kg/cm"));
    CHECK_OP(/, qarrA, qarrC, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg/m"));
    CHECK_OP(/, qarrC, qarrA, units::QuantityArray<double>, ({1, 0.5, 1.0/3.0, 0.25}, "m/kg"));
    CHECK_IOP(/, qarrA, qarrC, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg/m"));
  }
  // QuantityArray + Quantity
  {
    CHECK_OP(/, qarrA, qA, units::QuantityArray<double>, ({1, 2, 3, 4}, ""));
    CHECK_OP(/, qA, qarrA, units::QuantityArray<double>, ({1, 0.5, 1.0/3.0, 0.25}, ""));
    CHECK_OP(/, qarrA, qB, units::QuantityArray<double>, ({0.5, 1, 1.5, 2}, "kg/cm"));
    CHECK_OP(/, qB, qarrA, units::QuantityArray<double>, ({2, 1, 2.0/3.0, 0.5}, "cm/kg"));
    CHECK_IOP(/, qarrA, qB, units::QuantityArray<double>, ({0.5, 1, 1.5, 2}, "kg/cm"));
    CHECK_OP(/, qarrA, qC, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg/m"));
    CHECK_OP(/, qC, qarrA, units::QuantityArray<double>, ({1, 0.5, 1.0/3.0, 0.25}, "m/kg"));
    CHECK_IOP(/, qarrA, qC, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg/m"));
  }
}

TEST(Unit, Addition) {
  OP_SETUP;
  // Quantity + Quantity
  {
    CHECK_OP(+, qA, qA, units::Quantity<double>, (2.0, "kg"));
    CHECK_OP(+, qA, qF, units::Quantity<double>, (1.005, "kg"));
    CHECK_OP(+, qF, qA, units::Quantity<double>, (1005, "g"));
    CHECK_IOP(+, qA, qF, units::Quantity<double>, (1.005, "kg"));
    CHECK_OP(+, qB, qC, units::Quantity<double>, (102.0, "cm"));
    CHECK_OP(+, qC, qB, units::Quantity<double>, (1.02, "m"));
    CHECK_IOP(+, qB, qC, units::Quantity<double>, (102.0, "cm"));
  }
  // QuantityArray + QuantityArray
  {
    CHECK_OP(+, qarrA, qarrA, units::QuantityArray<double>, ({2, 4, 6, 8}, "kg"));
    CHECK_OP(+, qarrA, qarrF, units::QuantityArray<double>, ({1.005, 2.005, 3.005, 4.005}, "kg"));
    CHECK_OP(+, qarrF, qarrA, units::QuantityArray<double>, ({1005, 2005, 3005, 4005}, "g"));
    CHECK_IOP(+, qarrA, qarrF, units::QuantityArray<double>, ({1.005, 2.005, 3.005, 4.005}, "kg"));
    CHECK_OP(+, qarrB, qarrC, units::QuantityArray<double>, ({101, 102, 103, 104}, "cm"));
    CHECK_OP(+, qarrC, qarrB, units::QuantityArray<double>, ({1.01, 1.02, 1.03, 1.04}, "m"));
    CHECK_IOP(+, qarrB, qarrC, units::QuantityArray<double>, ({101, 102, 103, 104}, "cm"));
  }
  // QuantityArray + Quantity
  {
    CHECK_OP(+, qarrA, qA, units::QuantityArray<double>, ({2, 3, 4, 5}, "kg"));
    CHECK_OP(+, qA, qarrA, units::QuantityArray<double>, ({2, 3, 4, 5}, "kg"));
    CHECK_OP(+, qarrA, qF, units::QuantityArray<double>, ({1.005, 2.005, 3.005, 4.005}, "kg"));
    CHECK_OP(+, qF, qarrA, units::QuantityArray<double>, ({1005, 2005, 3005, 4005}, "g"));
    CHECK_IOP(+, qarrA, qF, units::QuantityArray<double>, ({1.005, 2.005, 3.005, 4.005}, "kg"));
    CHECK_OP(+, qarrB, qC, units::QuantityArray<double>, ({101, 102, 103, 104}, "cm"));
    CHECK_OP(+, qC, qarrB, units::QuantityArray<double>, ({1.01, 1.02, 1.03, 1.04}, "m"));
    CHECK_IOP(+, qarrB, qC, units::QuantityArray<double>, ({101, 102, 103, 104}, "cm"));
  }
}

TEST(Unit, Subtraction) {
  OP_SETUP;
  // Quantity + Quantity
  {
    CHECK_OP(-, qA, qA, units::Quantity<double>, (0.0, "kg"));
    CHECK_OP(-, qA, qF, units::Quantity<double>, (0.995, "kg"));
    CHECK_OP(-, qF, qA, units::Quantity<double>, (-995, "g"));
    CHECK_IOP(-, qA, qF, units::Quantity<double>, (0.995, "kg"));
    CHECK_OP(-, qB, qC, units::Quantity<double>, (-98, "cm"));
    CHECK_OP(-, qC, qB, units::Quantity<double>, (0.98, "m"));
    CHECK_IOP(-, qB, qC, units::Quantity<double>, (-98, "cm"));
  }
  // QuantityArray + QuantityArray
  {
    CHECK_OP(-, qarrA, qarrA, units::QuantityArray<double>, ({0, 0, 0, 0}, "kg"));
    CHECK_OP(-, qarrA, qarrF, units::QuantityArray<double>, ({0.995, 1.995, 2.995, 3.995}, "kg"));
    CHECK_OP(-, qarrF, qarrA, units::QuantityArray<double>, ({-995, -1995, -2995, -3995}, "g"));
    CHECK_IOP(-, qarrA, qarrF, units::QuantityArray<double>, ({0.995, 1.995, 2.995, 3.995}, "kg"));
    CHECK_OP(-, qarrB, qarrC, units::QuantityArray<double>, ({-99, -98, -97, -96}, "cm"));
    CHECK_OP(-, qarrC, qarrB, units::QuantityArray<double>, ({0.99, 0.98, 0.97, 0.96}, "m"));
    CHECK_IOP(-, qarrB, qarrC, units::QuantityArray<double>, ({-99, -98, -97, -96}, "cm"));
  }
  // QuantityArray + Quantity
  {
    CHECK_OP(-, qarrA, qA, units::QuantityArray<double>, ({0, 1, 2, 3}, "kg"));
    CHECK_OP(-, qA, qarrA, units::QuantityArray<double>, ({0, -1, -2, -3}, "kg"));
    CHECK_OP(-, qarrA, qF, units::QuantityArray<double>, ({0.995, 1.995, 2.995, 3.995}, "kg"));
    CHECK_OP(-, qF, qarrA, units::QuantityArray<double>, ({-995, -1995, -2995, -3995}, "g"));
    CHECK_IOP(-, qarrA, qF, units::QuantityArray<double>, ({0.995, 1.995, 2.995, 3.995}, "kg"));
    CHECK_OP(-, qarrB, qC, units::QuantityArray<double>, ({-99, -98, -97, -96}, "cm"));
    CHECK_OP(-, qC, qarrB, units::QuantityArray<double>, ({0.99, 0.98, 0.97, 0.96}, "m"));
    CHECK_IOP(-, qarrB, qC, units::QuantityArray<double>, ({-99, -98, -97, -96}, "cm"));
  }
}

TEST(Unit, Modulus) {
  OP_SETUP;
  // Quantity + Scalar
  {
    CHECK_OP(%, qA, sA, units::Quantity<double>, (0.0, "kg"));
    CHECK_IOP(%, qA, sA, units::Quantity<double>, (0.0, "kg"));
    CHECK_OP(%, qA, sB, units::Quantity<double>, (1.0, "kg"));
    CHECK_IOP(%, qA, sB, units::Quantity<double>, (1.0, "kg"));
  }
  // Quantity + Quantity
  {
    CHECK_OP(%, qA, qA, units::Quantity<double>, (0.0, "kg"));
    CHECK_OP(%, qA, qF, units::Quantity<double>, (0.0, "kg"));
    CHECK_OP(%, qF, qA, units::Quantity<double>, (5.0, "g"));
    CHECK_IOP(%, qA, qF, units::Quantity<double>, (0.0, "kg"));
    CHECK_OP(%, qB, qC, units::Quantity<double>, (2.0, "cm"));
    CHECK_OP(%, qC, qB, units::Quantity<double>, (0.0, "m"));
    CHECK_IOP(%, qB, qC, units::Quantity<double>, (2.0, "cm"));
  }
  // QuantityArray + Scalar
  {
    CHECK_OP(%, qarrA, sA, units::QuantityArray<double>, ({0, 0, 0, 0}, "kg"));
    CHECK_IOP(%, qarrA, sA, units::QuantityArray<double>, ({0, 0, 0, 0}, "kg"));
    CHECK_OP(%, qarrA, sB, units::QuantityArray<double>, ({1, 0, 1, 0}, "kg"));
    CHECK_IOP(%, qarrA, sB, units::QuantityArray<double>, ({1, 0, 1, 0}, "kg"));
  }
  // QuantityArray + QuantityArray
  {
    CHECK_OP(%, qarrA, qarrA, units::QuantityArray<double>, ({0, 0, 0, 0}, "kg"));
    CHECK_OP(%, qarrA, qarrF, units::QuantityArray<double>, ({0, 0, 0, 0}, "kg"));
    CHECK_OP(%, qarrF, qarrA, units::QuantityArray<double>, ({5, 5, 5, 5}, "g"));
    CHECK_IOP(%, qarrA, qarrF, units::QuantityArray<double>, ({0, 0, 0, 0}, "kg"));
    CHECK_OP(%, qarrB, qarrC, units::QuantityArray<double>, ({1, 2, 3, 4}, "cm"));
    CHECK_OP(%, qarrC, qarrB, units::QuantityArray<double>, ({0, 0, 0.01, 0}, "m"));
    CHECK_IOP(%, qarrB, qarrC, units::QuantityArray<double>, ({1, 2, 3, 4}, "cm"));
  }
  // QuantityArray + Quantity
  {
    CHECK_OP(%, qarrA, qA, units::QuantityArray<double>, ({0, 0, 0, 0}, "kg"));
    CHECK_OP(%, qA, qarrA, units::QuantityArray<double>, ({0, 1, 1, 1}, "kg"));
    CHECK_OP(%, qarrA, qF, units::QuantityArray<double>, ({0, 0, 0, 0}, "kg"));
    CHECK_OP(%, qF, qarrA, units::QuantityArray<double>, ({5, 5, 5, 5}, "g"));
    CHECK_IOP(%, qarrA, qF, units::QuantityArray<double>, ({0, 0, 0, 0}, "kg"));
    CHECK_OP(%, qarrB, qC, units::QuantityArray<double>, ({1, 2, 3, 4}, "cm"));
    CHECK_OP(%, qC, qarrB, units::QuantityArray<double>, ({0, 0, 0.01, 0}, "m"));
    CHECK_IOP(%, qarrB, qC, units::QuantityArray<double>, ({1, 2, 3, 4}, "cm"));
  }
}


TEST(Unit, Delta) {
  OP_SETUP_DELTA;
  // Quantity + Quantity
  {
    CHECK_OP(+, qA, qA, units::Quantity<double>, (2.0, "degC"));
    CHECK_OP(-, qA, qA, units::Quantity<double>, (0.0, "ΔdegC"));
    CHECK_OP(+, qA, qD, units::Quantity<double>, (2.0, "degC"));
    CHECK_OP(+, qB, qC, units::Quantity<double>, (-271.15, "degC"));
    CHECK_OP(-, qA, qC, units::Quantity<double>, (273.15, "ΔdegC"));
    CHECK_OP(-, qC, qA, units::Quantity<double>, (-273.15, "ΔK"));
  }
  // QuantityArray + QuantityArray
  {
    CHECK_OP(+, qarrA, qarrA, units::QuantityArray<double>, ({2, 4, 6, 8}, "degC"));
    CHECK_OP(-, qarrA, qarrA, units::QuantityArray<double>, ({0, 0, 0, 0}, "ΔdegC"));
    CHECK_OP(+, qarrA, qarrD, units::QuantityArray<double>, ({2, 4, 6, 8}, "degC"));
    CHECK_OP(+, qarrB, qarrC, units::QuantityArray<double>, ({-271.15, -269.15, -267.15, -265.15}, "degC"));
    CHECK_OP(-, qarrA, qarrC, units::QuantityArray<double>, ({273.15, 273.15, 273.15, 273.15}, "ΔdegC"));
    CHECK_OP(-, qarrC, qarrA, units::QuantityArray<double>, ({-273.15, -273.15, -273.15, -273.15}, "ΔK"));
  }
  // QuantityArray + Quantity
  {
    CHECK_OP(+, qarrA, qA, units::QuantityArray<double>, ({2, 3, 4, 5}, "degC"));
    CHECK_OP(+, qA, qarrA, units::QuantityArray<double>, ({2, 3, 4, 5}, "degC"));
    CHECK_OP(-, qarrA, qA, units::QuantityArray<double>, ({0, 1, 2, 3}, "ΔdegC"));
    CHECK_OP(-, qA, qarrA, units::QuantityArray<double>, ({0, -1, -2, -3}, "ΔdegC"));
    CHECK_OP(+, qarrA, qD, units::QuantityArray<double>, ({2, 3, 4, 5}, "degC"));
    CHECK_OP(+, qD, qarrA, units::QuantityArray<double>, ({275.15, 276.15, 277.15, 278.15}, "K"));
    CHECK_OP(+, qarrB, qC, units::QuantityArray<double>, ({-271.15, -270.15, -269.15, -268.15}, "degC"));
    CHECK_OP(+, qC, qarrB, units::QuantityArray<double>, ({2, 3, 4, 5}, "K"));
    CHECK_OP(-, qarrA, qC, units::QuantityArray<double>, ({273.15, 274.15, 275.15, 276.15}, "ΔdegC"));
    CHECK_OP(-, qC, qarrA, units::QuantityArray<double>, ({-273.15, -274.15, -275.15, -276.15}, "ΔK"));
  }
};

TEST(Unit, Power) {
  OP_SETUP;
  // Quantity
  {
    CHECK_METHOD(pow, qA, sA, units::Quantity<double>, (1.0, "kg"));
    CHECK_METHOD(pow, qA, sB, units::Quantity<double>, (1.0, "kg**2"));
    CHECK_METHOD(pow, qA, -1, units::Quantity<double>, (1.0, "kg**-1"));
    CHECK_IMETHOD(pow, qA, -1, units::Quantity<double>, (1.0, "kg**-1"));
    CHECK_METHOD(pow, qB, sA, units::Quantity<double>, (2.0, "cm"));
    CHECK_METHOD(pow, qB, sB, units::Quantity<double>, (4.0, "cm**2"));
    CHECK_METHOD(pow, qB, -1, units::Quantity<double>, (0.5, "cm**-1"));
    CHECK_IMETHOD(pow, qB, -1, units::Quantity<double>, (0.5, "cm**-1"));
    CHECK_METHOD(pow, qC, sA, units::Quantity<int>, (1L, "m"));
    CHECK_METHOD(pow, qC, sB, units::Quantity<int>, (1L, "m**2"));
  }
  // QuantityArray
  {
    CHECK_METHOD(pow, qarrA, sA, units::QuantityArray<double>, ({1, 2, 3, 4}, "kg"));
    CHECK_METHOD(pow, qarrA, sB, units::QuantityArray<double>, ({1, 4, 9, 16}, "kg**2"));
    CHECK_METHOD(pow, qarrA, -1, units::QuantityArray<double>, ({1, 0.5, 1.0/3.0, 0.25}, "kg**-1"));
    CHECK_IMETHOD(pow, qarrA, -1, units::QuantityArray<double>, ({1, 0.5, 1.0/3.0, 0.25}, "kg**-1"));
    CHECK_METHOD(pow, qarrB, sA, units::QuantityArray<double>, ({1, 2, 3, 4}, "cm"));
    CHECK_METHOD(pow, qarrB, sB, units::QuantityArray<double>, ({1, 4, 9, 16}, "cm**2"));
    CHECK_METHOD(pow, qarrB, -1, units::QuantityArray<double>, ({1, 0.5, 1.0/3.0, 0.25}, "cm**-1"));
    CHECK_IMETHOD(pow, qarrB, -1, units::QuantityArray<double>, ({1, 0.5, 1.0/3.0, 0.25}, "cm**-1"));
    CHECK_METHOD(pow, qarrC, sA, units::QuantityArray<int>, ({1, 1, 1, 1}, "m"));
    CHECK_METHOD(pow, qarrC, sB, units::QuantityArray<int>, ({1, 1, 1, 1}, "m**2"));
  }
}

TEST(Unit, Floor) {
  units::Quantity<double> qA(1.1, "kg");
  units::QuantityArray<double> qarrA({1.1, 2.1, 3.1, 4.1}, "kg");
  CACHE_VAR(units::Quantity<double>, qA);
  CACHE_VAR(units::QuantityArray<double>, qarrA);
  // Quantity
  {
    CHECK_METHOD(floor, qA, , units::Quantity<double>, (1.0, "kg"));
    CHECK_IMETHOD(floor, qA, , units::Quantity<double>, (1.0, "kg"));
  }
  // QuantityArray
  {
    CHECK_METHOD(floor, qarrA, , units::QuantityArray<double>, ({1, 2, 3, 4}, "kg"));
    CHECK_IMETHOD(floor, qarrA, , units::QuantityArray<double>, ({1, 2, 3, 4}, "kg"));
  }
}

TEST(Unit, Exponent) {
  units::Quantity<double> a(1.0, "kg");
  units::Quantity<double> b(1.0, "kg**2");
  units::Quantity<double> c(1.0, "kg^2");
  units::Quantity<double> d = a.pow(2);
  units::Quantity<double> e(1.0, "kg2");
  CHECK_QUANTITY_EQUIVALENCE(d, b, true);
  CHECK_QUANTITY_EQUIVALENCE(d, c, true);
  CHECK_QUANTITY_EQUIVALENCE(e, b, true);
  units::Quantity<double> x(1.0, "g2 km s-2");
  units::Quantity<double> y(1.0, "(g**2)*km*(s**-2)");
  CHECK_QUANTITY_EQUIVALENCE(x, y, true);
};


#if defined(_MSC_VER) || defined(__clang__)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_YGGDRASIL
