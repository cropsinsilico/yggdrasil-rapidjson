# Yggdrasil Types

YggdrasilRapidJSON adds support for serializing/deserializing several additional types not supported by RapidJSON. For these types, the data is serialized as a JSON string by encoding the underlying data and a schema describing it in base64.

In this section, we will use excerpts from `example/yggdrasil/yggdrasil.cpp`.

## Scalars {#Scalars}

While RapidJSON has support for some scalars in the form of number & string primitive types, only some precisions are supported and information about the precision is lost during serialization in the standard JSON format. YggdrasilRapidJSON expands on this by allowing for

* scalars with explicit precision (e.g. uint8_t, float16_t)
* complex numbers
* strings with arbitrary encoding.

Scalars are identified by their subtype and precision. Allowed subtypes include:

* `kYggIntSubType` Signed integers (precisions 1, 2, 4, 8)
* `kYggUintSubType` Unsigned integers (precisions 1, 2, 4, 8)
* `kYggFloatSubType` Floating point numbers (precisions 2, 4, 8; 16 if compiled with YGGDRASIL_LONG_DOUBLE_AVAILABLE)
* `kYggComplexSubType` Complex numbers (precisions, 8, 16; 32 if compiled with YGGDRASIL_LONG_DOUBLE_AVAILABLE)
* `kYggStringSubType` Strings with specified encoding. Any precision.

### Scalars with precisions not supported by RapidJSON

~~~~~~~~~~cpp
#include "rapidjson/document.h"

Document document(kObjectType);

{
    uint8_t x = 25;
    document.AddMember("uint8", x, document.GetAllocator());
}
{
    assert(document["uint8"].IsScalar());
    assert(document["uint8"].IsScalar<uint8_t>()); // Check type
    uint8_t x = document["uint8"].GetScalar<uint8_t>();
    assert(x == 25);
}
~~~~~~~~~~

Scalars can also be accessed by requesting the value in types with higher precisions.

~~~~~~~~~~cpp
{
    uint32_t y = document["uint8"].GetScalar<uint32_t>();
    assert(y == 25);
}
~~~~~~~~~~

YggdrasilRapidJSON has added methods to the `Document` class to allow users to access properties of scalars.

~~~~~~~~~~cpp
{
    assert(document["uint8"].GetPrecision() == 1);
    assert(document["uint8"].GetSubTypeCode() == kYggUintSubType);
}
~~~~~~~~~~

### Complex numbers

The JSON standard does not support complex numbers as a type so YggdrasilRapidJSON adds support for complex numbers of varying precision.

~~~~~~~~~~cpp
{
    std::complex<float> x(3.0F, 6.0F);
    document.AddMember("complex", x, document.GetAllocator());
}
{
    assert(document["complex"].IsScalar());
    assert(document["complex"].IsScalar<std::complex<float>>());
    std::complex<float> x = document["complex"].GetScalar<std::complex<float> >();
    assert(internal::values_eq(x.real(), 3.4F));
    assert(internal::values_eq(x.imag(), 6.2F));
}
~~~~~~~~~~

### Strings with specific encoding

~~~~~~~~~~cpp
{
    StringBuffer os;
    os.Clear();
    UTF8<>::Encode(os, 0x0370);
    Value v(os.GetString(), static_cast<SizeType>(os.GetLength()),
            "UCS4", 4, document.GetAllocator());
    document.AddMember("Encoded", v, document.GetAllocator());
}
{
    assert(document["Encoded"].IsScalar());
    assert(document["Encoded"].HasEncoding());
    const Value& encoding = document["Encoded"].GetEncoding();
    assert(std::strcmp(encoding.GetString(), "UCS4") == 0);
}
~~~~~~~~~~

## NDArrays {#NDarrays}

Arrays can have any number of dimension and elements of any of the scalar subtypes. They can be passed as stack variables

~~~~~~~~~~cpp
{
    float array2d[8][3] = {
      {0.0, 0.0, 0.0},
      {0.0, 0.0, 1.0},
      {0.0, 1.0, 1.0},
      {0.0, 1.0, 0.0},
      {1.0, 0.0, 0.0},
      {1.0, 0.0, 1.0},
      {1.0, 1.0, 1.0},
      {1.0, 1.0, 0.0}};
    Value v(array2d, document.GetAllocator());
    document.AddMember("Array2D", v, document.GetAllocator());
}
~~~~~~~~~~

or heap, with the shape and number of dimensions passed explicitly.

~~~~~~~~~~cpp
{
    Document::AllocatorType& allocator = document.GetAllocator();
    SizeType shape[3] = {2, 3, 3};
    SizeType nelements = 2 * 3 * 3;
    float* array3d = (float*)allocator.Malloc(nelements * sizeof(float));
    for (SizeType i = 0; i < nelements; i++) {
      array3d[i] = (float)(i * 0.05);
    }
    Value v(array3d, shape, 3, allocator);
    document.AddMember("Array3D", v, allocator);
    allocator.Free(array3d);
}
~~~~~~~~~~

In both cases, the memory inside the array is copied and the documents can be accessed in the same way.

~~~~~~~~~~cpp
{
    // Array on stack
    assert(document["Array2D"].IsNDArray());
    assert(document["Array2D"].IsNDArray<float>()); // Check type
    assert(!document["Array2D"].IsNDArray<uint8_t>());
    assert(document["Array2D"].GetNDim() == 2);
    const Value& shape = document["Array2D"].GetShape();
    assert(shape[0] == 8);
    assert(document["Array2D"].GetNElements() == (8 * 3));
    float dest_stack[8][3];
    document["Array2D"].GetNDArray(dest_stack);
    assert(internal::values_eq(dest_stack[6][2], (float)1.0));
    // Can also get the array in a version allocated on heap
    float* dest_heap = document["Array2D"].GetNDArray<float>(document.GetAllocator());
    assert(internal::values_eq(dest_heap[6 * 3 + 2], (float)1.0));
}
{
    // Array on heap
    assert(document["Array3D"].IsNDArray());
    assert(document["Array3D"].IsNDArray<float>()); // Check type
    assert(!document["Array3D"].IsNDArray<uint8_t>());
    assert(document["Array3D"].GetNDim() == 3);
    const Value& shape = document["Array3D"].GetShape();
    assert(shape[0] == 2);
    SizeType nelements = 2 * 3 * 3;
    assert(document["Array3D"].GetNElements() == nelements);
    float dest_stack[2][3][3];
    document["Array3D"].GetNDArray(dest_stack);
    assert(internal::values_eq(dest_stack[1][2][2], (float)0.85));
    // Can also get the array in a version allocated on heap
    float* dest_heap = document["Array3D"].GetNDArray<float>(document.GetAllocator());
    assert(internal::values_eq(dest_heap[nelements - 1], (float)0.85));
}
~~~~~~~~~~

## Scalars/NDArrays with Units {#Quantities}

~~~~~~~~~~cpp
{
    units::Quantity<double> scalar(0.5, "cm/s");
    document.AddMember("ScalarUnits", scalar, document.GetAllocator());
}
{
    float array2d[8][3] = {
      {0.0, 0.0, 0.0},
      {0.0, 0.0, 1.0},
      {0.0, 1.0, 1.0},
      {0.0, 1.0, 0.0},
      {1.0, 0.0, 0.0},
      {1.0, 0.0, 1.0},
      {1.0, 1.0, 1.0},
      {1.0, 1.0, 0.0}};
    units::QuantityArray<float> array(array2d, "cm/s");
    document.AddMember("ArrayUnits", array, document.GetAllocator());
}
~~~~~~~~~~

~~~~~~~~~~cpp
{
    assert(document["ScalarUnits"].IsScalar());
    assert(document["ScalarUnits"].IsScalar<double>());
    assert(document["ScalarUnits"].HasUnits());
    assert(internal::values_eq(document["ScalarUnits"].GetScalar<double>(), 0.5));
    assert(internal::values_eq(document["ScalarUnits"].GetScalar<double>("mm/s"), 5.0));
    units::Quantity<double> scalar = document["ScalarUnits"].GetScalarQuantity<double>();
    assert(internal::values_eq(scalar.value(), 0.5));
}
{
    assert(document["ArrayUnits"].IsNDArray());
    assert(document["ArrayUnits"].IsNDArray<float>()); // Check type
    assert(!document["ArrayUnits"].IsNDArray<uint8_t>());
    assert(document["ArrayUnits"].GetNDim() == 2);
    const Value& shape = document["ArrayUnits"].GetShape();
    assert(shape[0] == 8);
    assert(document["ArrayUnits"].GetNElements() == (8 * 3));
    float dest_stack[8][3];
    document["ArrayUnits"].GetNDArray(dest_stack);
    assert(internal::values_eq(dest_stack[6][2], 1.0));
    float dest_stack_mmps[8][3];
    document["ArrayUnits"].GetNDArray(dest_stack_mmps, "mm/s");
    assert(internal::values_eq(dest_stack_mmps[6][2], (float)10.0));
    units::QuantityArray<float> array = document["ArrayUnits"].GetArrayQuantity<float>(document.GetAllocator());
    assert(internal::values_eq(array.value()[6 * 3 + 2], (float)1.0));
}
~~~~~~~~~~

## Python types

### Python classes

### Python functions

### Python instances

## 3D Geometries

### ObjWavefront

### Ply