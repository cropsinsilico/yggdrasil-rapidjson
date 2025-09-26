// Example of using Yggdrasil units

#ifdef RAPIDJSON_YGGDRASIL

#include "rapidjson/units.h"         // Units
#include "rapidjson/internal/meta.h" // values_eq for floating point comparison
#include "rapidjson/document.h"      // RAPIDJSON_DEFAULT_ALLOCATOR

using namespace rapidjson;
using namespace std;

int main(int, char*[]) {
    ////////////////////////////////////////////////////////////////////////////
    // 1. Create scalars/arrays with units
    {
      // Scalar
      units::Quantity<float> x(5.0, "cm");
      assert(internal::values_eq(x.value(), (float)5.0));
      assert(x.unitsStr() == "cm");

      // Underlying type can be any number type
      units::Quantity<uint8_t> y(3, "s");
      assert(y.value() == (uint8_t)3);
      assert(y.unitsStr() == "s");
    }
    {
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
    }

    {
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
    }

    ////////////////////////////////////////////////////////////////////////////
    // 2. Perform arithmetic operations that track the units
    {
      // Quantity + Scalar
      units::Quantity<double> x(5.0, "cm");
      int y = 2;

      // Multiplication
      units::Quantity<double> zmul1 = x * y;
      assert(internal::values_eq(zmul1.value(), 10.0));
      assert(internal::values_eq(x.value(), 5.0));
      assert(zmul1.unitsStr() == "cm");
      units::Quantity<double> zmul2 = y * x; // Added in v 1.1.0.1
      assert(internal::values_eq(zmul2.value(), 10.0));
      assert(internal::values_eq(x.value(), 5.0));
      assert(zmul2.unitsStr() == "cm");

      // Division
      units::Quantity<double> zdiv1 = x / y;
      assert(internal::values_eq(zdiv1.value(), 2.5));
      assert(internal::values_eq(x.value(), 5.0));
      assert(zdiv1.unitsStr() == "cm");
      units::Quantity<double> zdiv2 = y / x;
      assert(internal::values_eq(zdiv2.value(), 0.4));
      assert(internal::values_eq(x.value(), 5.0));
      assert(zdiv2.unitsStr() == "cm**-1");

      // Modulus
      units::Quantity<double> zmod = x % y;
      assert(internal::values_eq(zmod.value(), 1.0));
      assert(zmod.unitsStr() == "cm");
      
      // Power
      units::Quantity<double> zpow = x.pow(y);
      assert(internal::values_eq(zpow.value(), 25.0));
      assert(zpow.unitsStr() == "cm**2");
      
      // Inplace multiplication
      x *= y;
      assert(internal::values_eq(x.value(), 10.0));
      
      // Inplace division
      x /= y;
      assert(internal::values_eq(x.value(), 5.0));

      // Inplace modulus
      x %= y;
      assert(internal::values_eq(x.value(), 1.0));
    }
    {
      // Quantity + Quantity
      units::Quantity<double> x(5.0, "cm");
      units::Quantity<double> y(2.5, "s");

      // Division
      units::Quantity<double> z = x / y;
      assert(internal::values_eq(z.value(), 2.0));
      assert(z.unitsStr() == "cm*(s**-1)");

      // For addition, subtraction, modulus values are converted first
      // and must be compatible
      assert(!x.is_compatible(y));
      // x + y would cause an error
      units::Quantity<double> a(6.0, "cm");
      units::Quantity<double> b(1.0, "m");
      assert(a.is_compatible(b));

      // Addition
      units::Quantity<double> c = a + b;
      assert(internal::values_eq(c.value(), 106.0));
      assert(c.unitsStr() == "cm");

      // Subtraction
      units::Quantity<double> d = b - a;
      assert(internal::values_eq(d.value(), 0.94));
      assert(d.unitsStr() == "m");

      // Modulus
      units::Quantity<double> e = b % a;
      assert(internal::values_eq(e.value(), 0.04));
      assert(e.unitsStr() == "m");

      // Operations can also be performed in place
      c += b;
      assert(internal::values_eq(c.value(), 206.0));
      assert(c.unitsStr() == "cm");
      c -= a;
      assert(internal::values_eq(c.value(), 200.0));
      assert(c.unitsStr() == "cm");
      c %= a;
      assert(internal::values_eq(c.value(), 2.0));
      assert(c.unitsStr() == "cm");
    }
    {
      // QuantityArray + QuantityArray
      units::QuantityArray<double> x({1.0, 2.0, 3.0, 4.0}, "cm");
      units::QuantityArray<double> y({2.0, 2.0, 3.0, 3.0}, "s");

      // Multiplication
      units::QuantityArray<double> zmul = x * y;
      assert(zmul.nelements() == 4);
      assert(internal::values_eq(zmul.value()[1], 4.0));
      assert(zmul.unitsStr() == "cm*s");
      
      // Division
      units::QuantityArray<double> zdiv = x / y;
      assert(zdiv.nelements() == 4);
      assert(internal::values_eq(zdiv.value()[1], 1.0));
      assert(zdiv.unitsStr() == "cm*(s**-1)");

      // Modulus
      units::QuantityArray<double> zmod = x / y;
      assert(zmod.nelements() == 4);
      assert(internal::values_eq(zmod.value()[1], 1.0));
      assert(zmod.unitsStr() == "cm*(s**-1)");

      // Like scalars, arrays must have compatible units for addition,
      // subtraction & modulus
      units::QuantityArray<double> a({1.0, 2.0, 3.0, 4.0}, "cm");
      units::QuantityArray<double> b({2.0, 2.0, 3.0, 3.0}, "m");

      // Addition
      units::QuantityArray<double> c = a + b;
      assert(c.nelements() == 4);
      assert(internal::values_eq(c.value()[1], 202.0));
      assert(c.unitsStr() == "cm");
      
      // Subtraction
      units::QuantityArray<double> d = a - b;
      assert(d.nelements() == 4);
      assert(internal::values_eq(d.value()[1], -198.0));
      assert(d.unitsStr() == "cm");
      
      // Modulus
      units::QuantityArray<double> e = b % a;
      assert(e.nelements() == 4);
      assert(internal::values_eq(e.value()[1], 0.0));
      assert(e.unitsStr() == "m");
      
    }
    {
      // QuantityArray + Quantity
      units::QuantityArray<double> x({1.0, 2.0, 3.0, 4.0}, "cm");
      units::Quantity<double> y(2.5, "s");

      // Multiplication
      units::QuantityArray<double> zmul1 = x * y;
      assert(zmul1.nelements() == 4);
      assert(internal::values_eq(zmul1.value()[1], 5.0));
      assert(zmul1.unitsStr() == "cm*s");
      units::QuantityArray<double> zmul2 = y * x;
      assert(zmul2.nelements() == 4);
      assert(internal::values_eq(zmul2.value()[1], 5.0));
      assert(zmul2.unitsStr() == "s*cm");
      
      // Division
      units::QuantityArray<double> zdiv1 = x / y;
      assert(zdiv1.nelements() == 4);
      assert(internal::values_eq(zdiv1.value()[1], 0.8));
      assert(zdiv1.unitsStr() == "cm*(s**-1)");
      units::QuantityArray<double> zdiv2 = y / x;
      assert(zdiv2.nelements() == 4);
      assert(internal::values_eq(zdiv2.value()[1], 1.25));
      assert(zdiv2.unitsStr() == "s*(cm**-1)");

      // Addition, subtraction, & modulus
      units::QuantityArray<double> a({1.0, 2.0, 3.0, 4.0}, "cm");
      units::Quantity<double> b(2.0, "m");
      
      // Addition
      units::QuantityArray<double> c = a + b;
      assert(c.nelements() == 4);
      assert(internal::values_eq(c.value()[1], 202.0));
      assert(c.unitsStr() == "cm");
      
      // Subtraction
      units::QuantityArray<double> d = b - a;
      assert(d.nelements() == 4);
      assert(internal::values_eq(d.value()[1], 1.98));
      assert(d.unitsStr() == "m");
      
      // Modulus
      units::QuantityArray<double> e = b % a;
      assert(e.nelements() == 4);
      assert(internal::values_eq(e.value()[1], 0.0));
      assert(e.unitsStr() == "m");
      
    }
  
    ////////////////////////////////////////////////////////////////////////////
    // 3. Comparison
    {
      units::Quantity<double> x(1.0, "cm");
      units::Quantity<double> y(1.0, "mm");
      assert(x > y);
    }

    ////////////////////////////////////////////////////////////////////////////
    // 4. Explicit conversion
    {
      units::Quantity<double> x(1.0, "cm");

      // In-place conversion
      x.convert_to("m");
      assert(internal::values_eq(x.value(), 0.01));
      assert(x.unitsStr() == "m");

      // Copy conversion
      units::Quantity<double> y = x.as("cm");
      assert(internal::values_eq(x.value(), 0.01));
      assert(x.unitsStr() == "m");
      assert(internal::values_eq(y.value(), 1.0));
      assert(y.unitsStr() == "cm");
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // 4. Unit system conversion
    {
      units::Units cgs("cm g s");
      units::Units mks("m kg s");
      units::Quantity<double> x(1.0, "cm*(min**-1)");
      units::Quantity<double> y = x.as_units_system(cgs);
      assert(internal::values_eq(y.value(), 0.016666666666666666));
      assert(y.unitsStr() == "cm*(s**-1)");
      units::Quantity<double> z = x.as_units_system(mks);
      assert(internal::values_eq(z.value(), 0.00016666666666666666));
      assert(z.unitsStr() == "m*(s**-1)");
    }
    
    return 0;
}

#else // RAPIDJSON_YGGDRASIL

int main(int, char*[]) {
  return 0;
}

#endif // RAPIDJSON_YGGDRASIL
