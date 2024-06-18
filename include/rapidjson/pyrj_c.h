#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES // required for M_PI in units
#endif

#ifdef __cplusplus
#include <cmath> // required before Python to allow use of hypot w/ MSVC
#else
#include <math.h>
#endif // __cplusplus

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

#ifdef YGGDRASIL_DISABLE_PYTHON_C_API

#ifndef PyObject
#define PyObject void*
#endif
#ifndef npy_intp
#define npy_intp int
#endif

#else // YGGDRASIL_DISABLE_PYTHON_C_API

#ifdef RAPIDJSON_FORCE_IMPORT_ARRAY
#ifdef NO_IMPORT_ARRAY
#undef NO_IMPORT_ARRAY
#endif
#else // RAPIDJSON_FORCE_IMPORT_ARRAY
#ifndef NO_IMPORT_ARRAY
#define NO_IMPORT_ARRAY
#endif
#endif // RAPIDJSON_FORCE_IMPORT_ARRAY
#ifndef PY_ARRAY_UNIQUE_SYMBOL
#define PY_ARRAY_UNIQUE_SYMBOL rapidjson_ARRAY_API
#endif
#ifndef NPY_NO_DEPRECATED_API
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#endif

#ifndef CHECK_UNICODE_NO_NUMPY
#ifdef RAPIDJSON_DONT_IMPORT_NUMPY
#define CHECK_UNICODE_NO_NUMPY(x) PyUnicode_Check(x)
#else
#define CHECK_UNICODE_NO_NUMPY(x) PyUnicode_Check(x) && !PyArray_CheckScalar(x)
#endif
#endif

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#include <numpy/arrayobject.h>
#include <numpy/ndarrayobject.h>
#include <numpy/npy_common.h>
#define _DEBUG
#else
#include <Python.h>
#include <numpy/arrayobject.h>
#include <numpy/ndarrayobject.h>
#include <numpy/npy_common.h>
#endif

#if NPY_ABI_VERSION < 0x02000000
#define PyDataType_FIELDS(descr) ((descr)->fields)
#define PyDataType_NAMES(descr) ((descr)->names)
/* #define PyDataType_SUBARRAY(descr) */
#define PyDataType_ELSIZE(descr) ((descr)->elsize)
#define PyDataType_SET_ELSIZE(descr, size) ((descr)->elsize = size)
#define PyArray_DescrProto PyArray_Descr
#define _PyArray_LegacyDescr PyArray_Descr
#endif

#endif // YGGDRASIL_DISABLE_PYTHON_C_API

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

