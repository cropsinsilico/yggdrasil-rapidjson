#ifndef RAPIDJSON_PYTHON_H_
#define RAPIDJSON_PYTHON_H_

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
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

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#include <iostream>

RAPIDJSON_NAMESPACE_BEGIN

/*!
  @brief Initialize Numpy arrays if it is not initalized.
 */
static inline
void init_numpy_API() {
  std::string err = "";
#ifdef _OPENMP
#pragma omp critical (numpy)
  {
#endif
  if (PyArray_API == NULL) {
    if (_import_array() < 0) {
      PyErr_Print();
      err = "Failed to _import_array";
    }
  }
#ifdef _OPENMP
  }
#endif
  if (err.length() > 0)
    throw std::runtime_error(err);
};


/*!
  @brief Initialize Python if it is not initialized.
 */
static inline
void init_python_API() {
  std::string err = "";
#ifdef _OPENMP
#pragma omp critical (python)
  {
#endif
  if (!(Py_IsInitialized())) {
    char *name = getenv("YGG_PYTHON_EXEC");
    if (name != NULL) {
      wchar_t *wname = Py_DecodeLocale(name, NULL);
      if (wname == NULL) {
	err = "Error decoding YGG_PYTHON_EXEC";
      } else {
	Py_SetProgramName(wname);
	PyMem_RawFree(wname);
      }
    }
    if (err.length() == 0) {
      Py_Initialize();
      if (!(Py_IsInitialized()))
	err = "Failed to initialize Python";
    }
    if (err.length() == 0) {
      init_numpy_API();
    }
  }
#ifdef _OPENMP
  }
#endif
  if (err.length() > 0)
    throw std::runtime_error(err);
};

/*!
  @brief Initialize Python if it is not initialized.
  @param[in] error_prefix std::string Prefix that should be added to error messages.
 */
inline
void initialize_python(const std::string error_prefix="") {
  try {
    init_python_API();
  } catch (std::exception& e) {
    throw std::runtime_error(error_prefix + "initialize_python: " + e.what());
  }
};


/*!
  @brief Finalize Python.
  @param[in] error_prefix Prefix to add to error messages.
 */
inline
void finalize_python(const std::string error_prefix="") {
  try {
    if (Py_IsInitialized())
      Py_Finalize();
  } catch (std::exception& e) {
    throw std::runtime_error(error_prefix + "finalize_python: " + e.what());
  }
};


/*!
  @brief Try to import a Python module, throw an error if it fails.
  @param[in] module_name const char* Name of the module to import (absolute path).
  @param[in] error_prefix std::string Prefix that should be added to error messages.
  @param[in] ignore_error bool If True and there is an error, a null pointer
  will be returned.
  @returns PyObject* Pointer to the Python module object.
 */
inline
PyObject* import_python_module(const char* module_name,
			       const std::string error_prefix="",
			       const bool ignore_error=false) {
  initialize_python(error_prefix);
  PyObject* out = PyImport_ImportModule(module_name);
  if (out == NULL) {
    PyErr_Print();
    if (!(ignore_error))
      throw std::runtime_error(error_prefix + "import_python_module: Failed to import Python model '" + module_name + "'");
  }
  return out;
};


/*!
  @brief Try to import a Python class, throw an error if it fails.
  @param[in] module_name const char* Name of the module to import (absolute path).
  @param[in] class_name const char* Name of the class to import from the specified module.
  @param[in] error_prefix std::string Prefix that should be added to error messages.
  @param[in] ignore_error bool If True and there is an error, a null pointer
  will be returned.
  @returns PyObject* Pointer to the Python class object.
 */
inline
PyObject* import_python_class(const char* module_name,
			      const char* class_name,
			      const std::string error_prefix="",
			      const bool ignore_error=false) {
  PyObject *py_module = import_python_module(module_name,
					     error_prefix,
					     ignore_error);
  if (py_module == NULL)
    return py_module;
  PyObject *out = PyObject_GetAttrString(py_module, class_name);
  Py_DECREF(py_module);
  if (out == NULL) {
    PyErr_Print();
    if (!(ignore_error))
      throw std::runtime_error(error_prefix + "import_python_class: Failed to import Python class/function/object '" + class_name + "'");
  }
  return out;
};

inline
PyObject* import_python_object(const char* mod_class,
			       const std::string error_prefix="",
			       const bool ignore_error=false) {
  char module_name[100] = "";
  char class_name[100] = "";
  if (sscanf(mod_class, "%[^:]:%s", module_name, class_name) != 2) {
    if (!(ignore_error))
      throw std::runtime_error(error_prefix + "import_python_object: Failed to import Python object '" + class_name + "'");
    return NULL;
  }
  return import_python_class(module_name, class_name, error_prefix, ignore_error);
};


RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_PYTHON_H_
