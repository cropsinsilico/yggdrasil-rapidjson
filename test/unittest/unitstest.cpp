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

#define RAPIDJSON_SCHEMA_VERBOSE 0
#define RAPIDJSON_HAS_STDSTRING 1

#include "unittest.h"
#include "rapidjson/schema.h"
#include "rapidjson/units.h"
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

#define CHECK_QUANTITY_EQUIVALENCE(a, b, expected)			\
  {									\
    if (a.equivalent_to(b) != expected) {				\
      std::cerr << "a = " << a << ", b = " << b;			\
      if (a.is_compatible(b)) {						\
	std::cerr << ", b.as(a.units) = " << b.as(a.units()) << ", a.value - b.value = " << std::abs(b.value() - a.as(b.units()).value()) << ", " << (std::abs(b.value() - a.as(b.units()).value()) < DBL_EPSILON); \
      }									\
      std::cerr << std::endl;						\
    }									\
    EXPECT_TRUE(a.equivalent_to(b) == expected);			\
  }

#define CHECK_QUANTITY_DIRECT_EQUALITY(a, b, expected)			\
  {									\
    if ((a==b) != expected) {						\
      std::cerr << "a = " << a << ", b = " << b << std::endl;		\
    }									\
    EXPECT_TRUE((a==b) == expected);					\
    EXPECT_FALSE((a!=b) == expected);					\
  }

#define COMPARE_UNITS(av, au, bv, bu, expected, direct)			\
  {									\
    units::Quantity<double, char> a(av, au);				\
    units::Quantity<double, char> b(bv, bu);				\
    CHECK_QUANTITY_EQUIVALENCE(a, b, expected);				\
    CHECK_QUANTITY_EQUIVALENCE(b, a, expected);				\
    CHECK_QUANTITY_DIRECT_EQUALITY(a, b, direct);			\
    CHECK_QUANTITY_DIRECT_EQUALITY(b, a, direct);			\
    if (expected) {							\
      EXPECT_TRUE(std::abs(a.value() - b.as(a.units()).value()) < DBL_EPSILON);	\
      EXPECT_TRUE(std::abs(b.value() - a.as(b.units()).value()) < DBL_EPSILON);	\
    }									\
  }

TEST(Unit, Base) {
  units::Quantity<double, char> x(1.0, "kg");
  std::cout << x << std::endl;
  COMPARE_UNITS(1.0, "g", 0.001, "kg", true, false);
  COMPARE_UNITS(1.0, "gram", 0.001, "kilogram", true, false);
  COMPARE_UNITS(1.0, "grams", 0.001, "kilograms", true, false);
  COMPARE_UNITS(1.0, "g", 1.0, "kg", false, false);
  COMPARE_UNITS(1.0, "cm", 1.0, "g", false, false);
  COMPARE_UNITS(1.0, "g**2", 1e-6, "kg^2", true, false);
  COMPARE_UNITS(1.0, "hp", 745.69987158227022, "W", true, false);
  COMPARE_UNITS(1.0, "km/s", 2236.936292054402, "mi/hr", true, false);
  COMPARE_UNITS(1.0, "km s", 1.0, "km*s", true, true);
  COMPARE_UNITS(1.0, "g**(1+1)", 1.0, "g^2", true, true);
  COMPARE_UNITS(1.0, "g**(3-1)", 1.0, "g^2", true, true);
  COMPARE_UNITS(1.0, "(km**2)(s**-1)", 1.0, "km**2/s", true, true);
  COMPARE_UNITS(1.0, "(km*A)**2/((s**2)(g**3))", 1.0, "(km^2)*(A^2)*(s^-2)*(g^-3)", true, true);
};

#define UNIT_OPERATOR(au, op, bu, cu)				\
  {								\
    units::Quantity<double, char> a(1.0, #au);			\
    units::Quantity<double, char> b(1.0, #bu);			\
    units::Quantity<double, char> c(1.0, #cu);			\
    units::Quantity<double, char> d = a op b;			\
    CHECK_QUANTITY_EQUIVALENCE(c, d, true);			\
  }

TEST(Unit, MultDiv) {
  UNIT_OPERATOR(kg, *, kg, kg**2);
  UNIT_OPERATOR(kg, *, cm, kg*cm);
  UNIT_OPERATOR(kg, /, cm, kg/cm);
  UNIT_OPERATOR(kg, *, cm**-1, kg/cm);
  UNIT_OPERATOR(kg, /, cm**-1, kg*cm);
  UNIT_OPERATOR(cm/s, *, kg, kg*cm/s);
};

TEST(Unit, Exponent) {
  units::Quantity<double, char> a(1.0, "kg");
  units::Quantity<double, char> b(1.0, "kg**2");
  units::Quantity<double, char> c(1.0, "kg^2");
  units::Quantity<double, char> d = a.pow(2);
  CHECK_QUANTITY_EQUIVALENCE(d, b, true);
  CHECK_QUANTITY_EQUIVALENCE(d, c, true);
};

#if defined(_MSC_VER) || defined(__clang__)
RAPIDJSON_DIAG_POP
#endif
