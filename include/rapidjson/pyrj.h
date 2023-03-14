#ifndef RAPIDJSON_PYTHON_H_
#define RAPIDJSON_PYTHON_H_

#ifdef _OPENMP
#include <omp.h>
#endif


#ifdef YGGDRASIL_DISABLE_PYTHON_C_API

#ifndef PyObject
#define PyObject void*
#endif

RAPIDJSON_NAMESPACE_BEGIN

inline
bool isPythonInitialized() {
  std::cerr << "The Python C API was disabled" << std::endl;
  return false;
}

RAPIDJSON_NAMESPACE_END

#else // YGGDRASIL_DISABLE_PYTHON_C_API

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
#include <vector>
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
#if PY_MAJOR_VERSION > 3 ||			\
  (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 8)
    PyStatus status;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    if (name != NULL) {
      status = PyConfig_SetBytesString(&config, &config.program_name, name);
      if (PyStatus_Exception(status)) {
	err = "Failed to set program name"; // GCOVR_EXCL_LINE
	goto done;
      }
    }
    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
      err = "Failed to initialize Python"; // GCOVR_EXCL_LINE
      goto done;
    }
#else
    if (name != NULL) {
      wchar_t *wname = Py_DecodeLocale(name, NULL);
      if (wname == NULL) {
	err = "Error decoding YGG_PYTHON_EXEC"; // GCOVR_EXCL_LINE
	goto done;
      } else {
	Py_SetProgramName(wname);
	PyMem_RawFree(wname);
      }
    }
    Py_Initialize();
#endif // Py_Config
    if (!(Py_IsInitialized())) {
      err = "Failed to initialize Python"; // GCOVR_EXCL_LINE
      goto done;
    }
    init_numpy_API();
  done:
#if PY_MAJOR_VERSION > 3 ||			\
  (PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 8)
    PyConfig_Clear(&config);
#else
    {}
#endif
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

#ifdef RAPIDJSON_YGGDRASIL_PYTHON
#define PYTHON_ERROR_CLEANUP_(method)				\
  cleanup:							\
  if (ignore_error) {						\
    PyErr_Clear();						\
  } else {							\
    if (PyErr_Occurred() != NULL) {				\
      throw std::runtime_error(error_prefix + #method ": Python error occured"); \
    }								\
  }								\
  return out
#else // RAPIDJSON_YGGDRASIL_PYTHON
#define PYTHON_ERROR_CLEANUP_(method)				\
  cleanup:							\
  if (ignore_error) {						\
    PyErr_Clear();						\
  } else {							\
    if (PyErr_Occurred() != NULL) {				\
      PyErr_Print();						\
      throw std::runtime_error(error_prefix + #method ": Python error occured"); \
    }								\
  }								\
  return out
#endif // RAPIDJSON_YGGDRASIL_PYTHON


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
  PyObject* out = NULL;
  RAPIDJSON_ASSERT(isPythonInitialized());
  if (!isPythonInitialized())
    goto cleanup;
  out = PyImport_ImportModule(module_name);
  PYTHON_ERROR_CLEANUP_(import_python_module);
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
  PyObject* out = NULL;
  PyObject *py_module = import_python_module(module_name,
					     error_prefix,
					     ignore_error);
  if (py_module == NULL)
    goto cleanup;
  out = PyObject_GetAttrString(py_module, class_name);
  Py_DECREF(py_module);
  PYTHON_ERROR_CLEANUP_(import_python_class);
}

inline
PyObject* import_python_object(const char* mod_class,
			       const std::string error_prefix="",
			       const bool ignore_error=false) {
  PyObject* out = NULL;
  char file_name[256] = "";
  char module_name[256] = "";
  char class_name[100] = "";
  size_t mod_class_len = strlen(mod_class);
  size_t iColon1 = mod_class_len, iColon2 = mod_class_len;
  size_t file_size = 0, module_size = 0, class_size = 0;
  bool ignore_error_1st = ignore_error;
  RAPIDJSON_ASSERT(mod_class_len <= 256);
  if (mod_class_len > 256) {
    if (!(ignore_error))
      throw std::runtime_error(error_prefix + "import_python_object: Name of module is greater that 256 characters"); // GCOVR_EXCL_LINE
    return NULL;
  }
  for (size_t i = 0; i < mod_class_len; i++) {
    if (i > 1 && mod_class[i] == ':') {
      if (iColon1 == mod_class_len) {
	iColon1 = i;
      } else if (iColon2 == mod_class_len) {
	iColon2 = i;
      } else {
	if (!(ignore_error))
	  throw std::runtime_error(error_prefix + "import_python_object: Name of module has more than 3 colons in it: " + mod_class); // GCOVR_EXCL_LINE
	return NULL;
      }
    }
  }
  if (iColon2 < mod_class_len) {
    file_size = iColon1;
    module_size = iColon2 - (iColon1 + 1);
    class_size = mod_class_len - (iColon2 + 1);
    memcpy(file_name, mod_class, file_size * sizeof(char));
    memcpy(module_name, mod_class + iColon1 + 1, module_size * sizeof(char));
    memcpy(class_name, mod_class + iColon2 + 1, class_size * sizeof(char));
    file_name[file_size] = '\0';
    module_name[module_size] = '\0';
    class_name[class_size] = '\0';
  } else if (iColon1 < mod_class_len) {
    module_size = iColon1;
    class_size = mod_class_len - (iColon1 + 1);
    memcpy(module_name, mod_class, module_size * sizeof(char));
    memcpy(class_name, mod_class + iColon1 + 1, class_size * sizeof(char));
    module_name[module_size] = '\0';
    class_name[class_size] = '\0';
  } else {
    if (!(ignore_error))
      throw std::runtime_error(error_prefix + "import_python_object: Failed to import Python object '" + mod_class + "'"); // GCOVR_EXCL_LINE
    return NULL;
  }
  bool ends_with_py = ((module_size > 3) && (strncmp(module_name + (module_size - 3), ".py", 3) == 0));
  if (ends_with_py) {
    if (file_size > 0) {
      if (!(ignore_error))
	throw std::runtime_error(error_prefix + "import_python_object: File specified and module is file: " + mod_class); // GCOVR_EXCL_LINE
      return NULL;
    }
    strcpy(file_name, module_name);
    file_size = module_size;
    module_size = 0;
  } else {
    ends_with_py = ((file_size > 3) &&
		    (strncmp(file_name + (file_size - 3), ".py", 3) == 0));
  }
  PyObject* path_add = NULL;
  if (file_size > 0) {
    if (ends_with_py)
      file_size -= 3;
    PyObject* path = PyUnicode_FromStringAndSize(file_name, (Py_ssize_t)file_size);
    if (path == NULL)
      goto cleanup;
    PyObject* os_path = PyImport_ImportModule("os.path");
    if (os_path == NULL) {
      Py_DECREF(path);
      goto cleanup;
    }
    PyObject* path_abspath = PyObject_GetAttrString(os_path, "abspath");
    if (path_abspath == NULL) {
      Py_DECREF(path);
      Py_DECREF(os_path);
      goto cleanup;
    }
    PyObject* path_abs = PyObject_CallFunction(path_abspath, "(O)", path);
    Py_DECREF(path_abspath);
    Py_DECREF(path);
    path = path_abs;
    path_abs = NULL;
    PyObject* path_split = PyObject_GetAttrString(os_path, "split");
    Py_DECREF(os_path);
    if (path_split == NULL) {
      Py_DECREF(path);
      goto cleanup;
    }
    PyObject* path_parts = PyObject_CallFunction(path_split, "(O)", path);
    Py_DECREF(path_split);
    if (path_parts == NULL) {
      Py_DECREF(path);
      goto cleanup;
    }
    PyObject* path_dir = PyTuple_GetItem(path_parts, 0);
    if (path_dir == NULL) {
      Py_DECREF(path);
      Py_DECREF(path_parts);
      goto cleanup;
    }
    if (ends_with_py || module_size == 0) {
      path_add = path_dir;
    } else {
      path_add = path;
    }
    Py_INCREF(path_add); // Before path or path_dir (via path_parts) decref'd
    Py_DECREF(path);
    PyObject* path_base = PyTuple_GetItem(path_parts, 1);
    if (path_base == NULL) {
      Py_DECREF(path_parts);
      goto cleanup;
    }
    Py_ssize_t tmp_size = 0;
    const char* tmp = PyUnicode_AsUTF8AndSize(path_base, &tmp_size);
    if ((tmp == NULL) || (tmp_size >= 100)) {
      Py_DECREF(path_parts);
      goto cleanup;
    }
    if (module_size == 0) {
      module_size = static_cast<size_t>(tmp_size);
      memcpy(module_name, tmp, module_size);
      module_name[module_size] = '\0';
    }
    Py_DECREF(path_parts);
    ignore_error_1st = true;
  }
  out = import_python_class(module_name, class_name, error_prefix, ignore_error_1st);
  // Removing added path makes the object un-picklable
  if (out == NULL && file_size > 0) {
    // Add file path
    RAPIDJSON_ASSERT(path_add != NULL);
    if (path_add == NULL) {
      return NULL;
    }
    PyObject* sys_path = PySys_GetObject("path");
    if (sys_path == NULL) {
      Py_DECREF(path_add);
      goto cleanup;
    }
    if (PyList_Append(sys_path, path_add) < 0) {
      Py_DECREF(path_add);
      goto cleanup;
    }
    Py_DECREF(path_add);
    // Try again with path added
    out = import_python_class(module_name, class_name, error_prefix, ignore_error);
    // Remove added path
    // Removing added path makes the object un-picklable
    PyObject_CallMethod(sys_path, "pop", "n", Py_SIZE(sys_path) - 1);
  }
  PYTHON_ERROR_CLEANUP_(import_python_object);
}


inline
PyObject* pickle_python_object(PyObject* x,
			       const std::string error_prefix="",
			       const bool ignore_error=false) {
  PyObject *out = NULL, *args = NULL;
  PyObject* pickleMethod = import_python_object("pickle:dumps", "GetPythonInstance", true);
  if (pickleMethod == NULL)
    goto cleanup;
  args = Py_BuildValue("(O)", x);
  if (args == NULL) {
    Py_DECREF(pickleMethod);
    goto cleanup;
  }
  out = PyObject_Call(pickleMethod, args, NULL);
  Py_DECREF(pickleMethod);
  Py_DECREF(args);
  PYTHON_ERROR_CLEANUP_(pickle_python_object);
}

inline
PyObject* unpickle_python_object(const char* buffer,
				 size_t buffer_length,
				 const std::string error_prefix="",
				 const bool ignore_error=false) {
  PyObject *out = NULL, *py_str = NULL, *args = NULL;
  PyObject* pickle = import_python_object("pickle:loads", error_prefix, ignore_error);
  if (pickle == NULL)
    goto cleanup;
  py_str = PyBytes_FromStringAndSize(buffer, (Py_ssize_t)buffer_length);
  if (py_str == NULL) {
    Py_DECREF(pickle);
    goto cleanup;
  }
  args = Py_BuildValue("(O)", py_str);
  Py_DECREF(py_str);
  if (args == NULL) {
    Py_DECREF(pickle);
    goto cleanup;
  }
  out = PyObject_Call(pickle, args, NULL);
  Py_DECREF(pickle);
  Py_DECREF(args);
  PYTHON_ERROR_CLEANUP_(unpickle_python_object);
}


inline
bool PyObject_IsInstanceString(PyObject* x, std::string class_name) {
  if (!PyObject_HasAttrString(x, "__class__"))
    return false;
  PyObject* inst_class = PyObject_GetAttrString(x, "__class__");
  if (inst_class == NULL)
    return false;
  PyObject* inst_class_str = PyObject_Str(inst_class);
  Py_DECREF(inst_class);
  if (inst_class_str == NULL)
    return false;
  std::string result(PyUnicode_AsUTF8(inst_class_str));
  Py_DECREF(inst_class_str);
  std::string check = "<class '" + class_name + "'>";
  return (check == result);
}
inline
bool IsStructuredArray(PyObject* x) {
  RAPIDJSON_ASSERT(isPythonInitialized());
  if (!isPythonInitialized())
    return false;
#ifdef RAPIDJSON_DONT_IMPORT_NUMPY
  (void)x;
  return false;
#else // RAPIDJSON_DONT_IMPORT_NUMPY
  if (x == NULL || !PyList_Check(x)) return false;
  int nd = 0;
  npy_intp *dims = NULL;
  Py_ssize_t N = PyList_Size(x);
  if (N == 0) return false;
  for (Py_ssize_t i = 0; i < N; i++) {
    PyObject* item = PyList_GetItem(x, i);
    RAPIDJSON_ASSERT(item);
    if (item == NULL)
      return false;
    if (!PyArray_CheckExact(item))
      return false;
    PyArray_Descr* desc = PyArray_DESCR((PyArrayObject*)item);
    RAPIDJSON_ASSERT(desc);
    if (desc == NULL)
      return false;
    if (desc->names == NULL)
      return false;
    if (PyTuple_Size(desc->names) != 1)
      return false;
    if (dims == NULL) {
      if (i > 0)
	return false;
      nd = PyArray_NDIM((PyArrayObject*)item);
      dims = PyArray_DIMS((PyArrayObject*)item);
    } else {
      if (nd != PyArray_NDIM((PyArrayObject*)item))
	return false;
      for (int j = 0; j < nd; j++) {
	npy_intp* idims = PyArray_DIMS((PyArrayObject*)item);
	if (dims[j] != idims[j])
	  return false;
      }
    }
  }
  return true;
#endif // RAPIDJSON_DONT_IMPORT_NUMPY
}
inline
PyObject* GetStructuredArray(PyObject* x) {
  RAPIDJSON_ASSERT(isPythonInitialized());
  if (!isPythonInitialized())
    return NULL;
#ifdef RAPIDJSON_DONT_IMPORT_NUMPY
  return x;
#else // RAPIDJSON_DONT_IMPORT_NUMPY
  Py_ssize_t N = PyList_Size(x);
  int nd = 0;
  npy_intp *dims = NULL, *strides = NULL;
  PyObject *out = NULL, *names = NULL, *fields = NULL;
  PyObject *ikey = NULL, *idtype = NULL, *ioffset = NULL, *ifield = NULL;
  PyArray_Descr *idesc = NULL, *sub_desc = NULL, *desc = NULL, *isub_desc;
  PyArrayObject *ival = NULL, *array = NULL;
  Py_ssize_t i = 0, kw_pos = 0;
  npy_intp offset = 0;
  std::vector<npy_intp> offsets;
  names = PyTuple_New(N);
  if (names == NULL)
    goto cleanup;
  fields = PyDict_New();
  if (fields == NULL)
    goto cleanup;
  for (i = 0; i < N; i++) {
    ival = (PyArrayObject*)PyList_GetItem(x, i);
    if (ival == NULL)
      goto cleanup;
    if (i == 0) {
      nd = PyArray_NDIM(ival);
      dims = PyArray_DIMS(ival);
    }
    idesc = PyArray_DESCR(ival);
    if (idesc == NULL)
      goto cleanup;
    ikey = PyTuple_GetItem(idesc->names, 0);
    if (ikey == NULL)
      goto cleanup;
    Py_INCREF(ikey);
    if (PyTuple_SetItem(names, i, ikey) < 0)
      goto cleanup;
    ifield = PyDict_GetItem(idesc->fields, ikey);
    if (ifield == NULL)
      goto cleanup;
    isub_desc = (PyArray_Descr*)PyTuple_GetItem(ifield, 0);
    if (isub_desc == NULL)
      goto cleanup;
    sub_desc = PyArray_DescrNewFromType(isub_desc->type_num);
    // sub_desc = PyArray_DescrNewFromType(PyArray_TYPE(ival));
    if (sub_desc == NULL)
      goto cleanup;
    sub_desc->elsize = (int)PyArray_ITEMSIZE(ival);
    offsets.push_back(offset);
    ioffset = PyLong_FromSsize_t((Py_ssize_t)offset);
    if (ioffset == NULL) {
      Py_DECREF(sub_desc);
      goto cleanup;
    }
    idtype = PyTuple_Pack(2, sub_desc, ioffset);
    Py_DECREF(sub_desc);
    Py_DECREF(ioffset);
    if (idtype == NULL)
      goto cleanup;
    if (PyDict_SetItem(fields, ikey, idtype) < 0) {
      Py_DECREF(idtype);
      goto cleanup;
    }
    Py_DECREF(idtype);
    offset += PyArray_ITEMSIZE(ival);
  }
  desc = PyArray_DescrNewFromType(NPY_VOID);
  if (desc == NULL)
    goto cleanup;
  Py_INCREF(fields);
  desc->fields = fields;
  Py_INCREF(names);
  desc->names = names;
  desc->elsize = (int)offset;
  // TODO: fill in other fields? alignment?
  array = (PyArrayObject*)PyArray_NewFromDescr(&PyArray_Type, desc,
					       nd, dims, strides,
					       NULL, 0, NULL);
  desc = NULL;
  if (array == NULL)
    goto cleanup;
  i = 0;
  while (PyDict_Next(fields, &kw_pos, &ikey, &idtype)) {
    ival = (PyArrayObject*)PyList_GetItem(x, i);
    if (ival == NULL)
      goto cleanup;
    idesc = (PyArray_Descr*)PyTuple_GetItem(idtype, 0);
    if (idesc == NULL)
      goto cleanup;
    Py_INCREF(idesc); // Does PyArray_SetField steal descr ref?
    if (PyArray_SetField(array, idesc, (int)offsets[(size_t)i], (PyObject*)ival) < 0)
      goto cleanup;
    i++;
  }
  out = (PyObject*)array;
  array = NULL;
 cleanup:
  Py_XDECREF(fields);
  Py_XDECREF(names);
  Py_XDECREF(desc);
  Py_XDECREF(array);
  return out;
#endif // RAPIDJSON_DONT_IMPORT_NUMPY
}

RAPIDJSON_NAMESPACE_END

#endif // YGGDRASIL_DISABLE_PYTHON_C_API

#endif // RAPIDJSON_PYTHON_H_
