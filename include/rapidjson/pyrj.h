#ifndef RAPIDJSON_PYTHON_H_
#define RAPIDJSON_PYTHON_H_

#ifdef _OPENMP
#include <omp.h>
#endif

#ifndef RAPIDJSON_FORCE_IMPORT_ARRAY
#define NO_IMPORT_ARRAY
#endif // RAPIDJSON_FORCE_IMPORT_ARRAY
#define PY_ARRAY_UNIQUE_SYMBOL rapidjson_ARRAY_API
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

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

#include <string>
#include <stdexcept>
#include <iostream>
#ifdef WIN32
#include <stdlib.h>
#endif // WIN32
#include "rapidjson.h"

RAPIDJSON_NAMESPACE_BEGIN

/*!
  @brief Initialize Numpy arrays if it is not initalized.
 */
static inline
void init_numpy_API() {
#ifndef RAPIDJSON_DONT_IMPORT_NUMPY
#ifdef RAPIDJSON_FORCE_IMPORT_ARRAY
  std::string err = "";
#ifdef _OPENMP
#pragma omp critical (numpy)
  {
#endif
  if (PY_ARRAY_UNIQUE_SYMBOL == NULL) { // GCOVR_EXCL_START
    if (_import_array() < 0)
      PyErr_Print();
  } // GCOVR_EXCL_STOP
#ifdef _OPENMP
  }
#endif
  if (err.length() > 0)
    throw std::runtime_error(err); // GCOVR_EXCL_LINE
#endif // RAPIDJSON_FORCE_IMPORT_ARRAY
#endif // RAPIDJSON_DONT_IMPORT_NUMPY
}


/*!
  @brief Initialize Python if it is not initialized.
 */
static inline
void init_python_API() {
  std::string err = "";
#ifndef RAPIDJSON_YGGDRASIL_PYTHON
#ifdef _OPENMP
#pragma omp critical (python)
  {
#endif
  if (!(Py_IsInitialized())) {
#ifdef WIN32
    // Work around for https://github.com/ContinuumIO/anaconda-issues/issues/11374
    _putenv_s("CONDA_PY_ALLOW_REG_PATHS", "1");
#endif // WIN32
    char *name = getenv("YGG_PYTHON_EXEC");
    if (name != NULL) {
      wchar_t *wname = Py_DecodeLocale(name, NULL);
      if (wname == NULL) {
	err = "Error decoding YGG_PYTHON_EXEC"; // GCOVR_EXCL_LINE
      } else {
	Py_SetProgramName(wname);
	PyMem_RawFree(wname);
      }
    }
    if (err.length() == 0) {
      Py_Initialize();
      if (!(Py_IsInitialized()))
	err = "Failed to initialize Python"; // GCOVR_EXCL_LINE
      else
	init_numpy_API();
    }
  }
#ifdef _OPENMP
  }
#endif
  if (err.length() > 0)
    throw std::runtime_error(err); // GCOVR_EXCL_LINE
#endif // RAPIDJSON_YGGDRASIL_PYTHON
}


inline
bool isPythonInitialized() {
  bool out = false;
#ifdef RAPIDJSON_FORCE_IMPORT_ARRAY
  out = (Py_IsInitialized() && (PY_ARRAY_UNIQUE_SYMBOL != NULL));
#else
  out = (Py_IsInitialized());
#endif
  if (!out)
    std::cerr << "Python is not initialized." << std::endl;
  return out;
}


/*!
  @brief Initialize Python if it is not initialized.
  @param[in] error_prefix std::string Prefix that should be added to error messages.
 */
inline
void initialize_python(const std::string error_prefix="") {
  try {
    init_python_API();
  } catch (std::exception& e) {
    throw std::runtime_error(error_prefix + "initialize_python: " + e.what()); // GCOVR_EXCL_LINE
  }
}


/*!
  @brief Finalize Python.
  @param[in] error_prefix Prefix to add to error messages.
 */
inline
void finalize_python(const std::string error_prefix="") {
#ifndef RAPIDJSON_YGGDRASIL_PYTHON
  try {
    if (Py_IsInitialized())
      Py_Finalize();
  } catch (std::exception& e) {
    throw std::runtime_error(error_prefix + "finalize_python: " + e.what()); // GCOVR_EXCL_LINE
  }
#endif // RAPIDJSON_YGGDRASIL_PYTHON
}


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
  RAPIDJSON_ASSERT(isPythonInitialized());
  if (!isPythonInitialized())
    return NULL;
  PyObject* out = PyImport_ImportModule(module_name);
  if (out == NULL) { // GCOVR_EXCL_START
#ifndef RAPIDJSON_YGGDRASIL_PYTHON
    PyErr_Print();
#endif // RAPIDJSON_YGGDRASIL_PYTHON
    if (!(ignore_error))
      throw std::runtime_error(error_prefix + "import_python_module: Failed to import Python model '" + module_name + "'");
  } // GCOVR_EXCL_STOP
  return out;
}


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
    return py_module; // GCOVR_EXCL_LINE
  PyObject *out = PyObject_GetAttrString(py_module, class_name);
  Py_DECREF(py_module);
  if (out == NULL) { // GCOVR_EXCL_START
#ifndef RAPIDJSON_YGGDRASIL_PYTHON
    PyErr_Print();
#endif // RAPIDJSON_YGGDRASIL_PYTHON
    if (!(ignore_error))
      throw std::runtime_error(error_prefix + "import_python_class: Failed to import Python class/function/object '" + class_name + "'");
  } // GCOVR_EXCL_STOP
  return out;
}

inline
PyObject* import_python_object(const char* mod_class,
			       const std::string error_prefix="",
			       const bool ignore_error=false) {
  char module_name[100] = "";
  char class_name[100] = "";
  if (sscanf(mod_class, "%[^:]:%s", module_name, class_name) != 2) {
    if (!(ignore_error))
      throw std::runtime_error(error_prefix + "import_python_object: Failed to import Python object '" + mod_class + "'"); // GCOVR_EXCL_LINE
    return NULL;
  }
  Py_ssize_t module_name_len = (Py_ssize_t)strlen(module_name);
  bool is_file = ((module_name_len > 3) && (strncmp(module_name + ((size_t)module_name_len - 3), ".py", 3) == 0));
  if (is_file) {
    PyObject* path = PyUnicode_FromStringAndSize(module_name, module_name_len - 3);
    if (path == NULL)
      return NULL;
    PyObject* os_path = PyImport_ImportModule("os.path");
    if (os_path == NULL) {
      Py_DECREF(path);
      return NULL;
    }
    PyObject* path_split = PyObject_GetAttrString(os_path, "split");
    if (path_split == NULL) {
      Py_DECREF(path);
      Py_DECREF(os_path);
      return NULL;
    }
    PyObject* split_args = PyTuple_Pack(1, path);
    if (split_args == NULL) {
      Py_DECREF(path);
      Py_DECREF(path_split);
      Py_DECREF(os_path);
      return NULL;
    }
    PyObject* path_parts = PyObject_Call(path_split, split_args, NULL);
    Py_DECREF(split_args);
    Py_DECREF(path);
    Py_DECREF(path_split);
    if (path_parts == NULL)
      return NULL;
    PyObject* path_dir = PyTuple_GetItem(path_parts, 0);
    if (path_dir == NULL) {
      Py_DECREF(path_parts);
      return NULL;
    }
    PyObject* sys_path = PySys_GetObject("path");
    if (sys_path == NULL) {
      Py_DECREF(path_parts);
      return NULL;
    }
    int res = PyList_Append(sys_path, path_dir);
    if (res < 0) {
      Py_DECREF(path_parts);
      return NULL;
    }
    PyObject* path_base = PyTuple_GetItem(path_parts, 1);
    if (path_base == NULL) {
      Py_DECREF(path_parts);
      return NULL;
    }
    Py_ssize_t tmp_size = 0;
    const char* tmp = PyUnicode_AsUTF8AndSize(path_base, &tmp_size);
    if ((tmp == NULL) || (tmp_size >= 100)) {
      Py_DECREF(path_parts);
      return NULL;
    }
    memcpy(module_name, tmp, (size_t)tmp_size);
    module_name[tmp_size] = '\0';
    Py_DECREF(path_parts);
  }
  PyObject* out = import_python_class(module_name, class_name, error_prefix, ignore_error);
  if (is_file) {
    PyObject* sys_path = PySys_GetObject("path");
    if (sys_path == NULL)
      return NULL;
    PyObject_CallMethod(sys_path, "pop", "n", Py_SIZE(sys_path) - 1);
  }
  return out;
}


RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_PYTHON_H_
