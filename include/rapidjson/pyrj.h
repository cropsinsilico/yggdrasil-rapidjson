#ifndef RAPIDJSON_PYTHON_H_
#define RAPIDJSON_PYTHON_H_

#ifdef _OPENMP
#include <omp.h>
#endif

#include "pyrj_c.h"

#ifdef YGGDRASIL_DISABLE_PYTHON_C_API

#include "rapidjson.h"
#include <iostream>

RAPIDJSON_NAMESPACE_BEGIN

inline
bool isPythonInitialized() {
  std::cerr << "The Python C API was disabled" << std::endl;
  return false;
}

RAPIDJSON_NAMESPACE_END

#else // YGGDRASIL_DISABLE_PYTHON_C_API

#include <string>
#include <stdexcept>
#include <iostream>
#include <vector>
#ifdef WIN32
#include <stdlib.h>
#endif // WIN32
#include "rapidjson.h"
#include "encodings.h"

#ifdef YGG_CHECK_REF_COUNTS
#include <map>
#endif // YGG_CHECK_REF_COUNTS

RAPIDJSON_NAMESPACE_BEGIN

#ifdef YGG_ENSURE_PY_GIL
#define BEGIN_PY_GIL				\
  PyGILState_STATE gstate;			\
  gstate = PyGILState_Ensure()
#define END_PY_GIL				\
  PyGILState_Release(gstate)
#else // YGG_ENSURE_PY_GIL
#define BEGIN_PY_GIL
#define END_PY_GIL
#endif // YGG_ENSURE_PY_GIL

#ifdef YGG_CHECK_REF_COUNTS

inline
void __add_refs(const std::string& name,
		std::map<std::string, Py_ssize_t>& reg, PyObject* x) {
  Py_ssize_t N = 0;
  if (x != NULL || PyErr_Occurred())
    N = Py_REFCNT(x);
  if (x != NULL && N > 0) {
    PyObject* str = PyObject_Str(x);
    std::string key = "unknown";
    if (str != NULL && PyUnicode_Check(str)) {
      key.assign(PyUnicode_AsUTF8(str));
    }
    Py_XDECREF(str);
    long tot = -1;
    PyObject* sys = PyImport_ImportModule("sys");
    if (sys != NULL && PyObject_HasAttrString(sys, "gettotalrefcount")) {
      PyObject* refc = PyObject_GetAttrString(sys, "gettotalrefcount");
      if (refc != NULL) {
	PyObject* res = PyObject_CallFunction(refc, NULL);
	Py_DECREF(refc);
	if (res != NULL && PyLong_Check(res)) {
	  tot = PyLong_AsLong(res);
	}
	Py_XDECREF(res);
      }
    }
    Py_XDECREF(sys);
    reg[key] = N;
    std::cerr << name << ": " << N << " REFS: " << key <<
      " (total = " << tot << ")" << std::endl;
  }
}

inline
void _add_refs(const std::string& name,
	       std::map<std::string, Py_ssize_t>& reg, PyObject* x) {
  __add_refs(name, reg, x);
}
template<typename... Args>
inline
void _add_refs(const std::string& name,
	       std::map<std::string, Py_ssize_t>& reg, PyObject* x,
	       Args... args) {
  __add_refs(name, reg, x);
  _add_refs(name, reg, args...);
}
template<typename... Args>
inline
std::map<std::string, Py_ssize_t> _get_refs(const std::string& name,
					    PyObject* x, Args... args) {
  std::map<std::string, Py_ssize_t> out;
  _add_refs(name, out, x, args...);
  return out;
}
// std::map<std::string, Py_ssize_t> _get_refs_diff(
//   std::map<std::string, Py_ssize_t> a,
//   std::map<std::string, Py_ssize_t> b) {
//   std::map<std::string, Py_ssize_t> out;
  
// }

#define CHECK_REFS(...)						\
  _get_refs(__PRETTY_FUNCTION__, __VA_ARGS__)
#else // YGG_CHECK_REF_COUNTS
#define CHECK_REFS(...)
#endif // YGG_CHECK_REF_COUNTS

/*!
  @brief Initialize Numpy arrays if it is not initalized.
 */
static inline
void init_numpy_API() {
#ifndef RAPIDJSON_DONT_IMPORT_NUMPY
#ifdef RAPIDJSON_FORCE_IMPORT_ARRAY
  BEGIN_PY_GIL;
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
  END_PY_GIL;
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
#if defined(RAPIDJSON_FORCE_IMPORT_ARRAY) && !defined(RAPIDJSON_DONT_IMPORT_NUMPY)
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
#ifdef RAPIDJSON_YGGDRASIL_PYTHON
inline
void finalize_python(const std::string="") {}
#else
inline
void finalize_python(const std::string error_prefix="") {
  try {
    if (Py_IsInitialized())
      Py_Finalize();
  } catch (std::exception& e) {
    throw std::runtime_error(error_prefix + "finalize_python: " + e.what()); // GCOVR_EXCL_LINE
  }
}
#endif // RAPIDJSON_YGGDRASIL_PYTHON

#define PYTHON_ERROR_SETUP_			\
  std::string err;				\
  BEGIN_PY_GIL
#define PYTHON_ERROR_EXIT_(val)			\
  {						\
    out = val;					\
    goto cleanup;				\
  }
// TODO: Version that doesn't throw error
// TODO: Version that allows other actions in cleanup
#ifdef RAPIDJSON_YGGDRASIL_PYTHON
#define PYTHON_ERROR_CLEANUP_BASE_(method)			\
  if (ignore_error) {						\
    PyErr_Clear();						\
  } else {							\
    if (PyErr_Occurred() != NULL) {				\
      if (err.empty()) {					\
	err = error_prefix + #method + ": Python error occured";	\
      }								\
    }								\
  }								\
  END_PY_GIL
#else // RAPIDJSON_YGGDRASIL_PYTHON
#define PYTHON_ERROR_CLEANUP_BASE_(method)			\
  if (ignore_error) {						\
    PyErr_Clear();						\
  } else {							\
    if (PyErr_Occurred() != NULL) {				\
      PyErr_Print();						\
      if (err.empty()) {					\
	err = error_prefix + #method + ": Python error occured";	\
      }								\
    }								\
  }								\
  END_PY_GIL
#endif // RAPIDJSON_YGGDRASIL_PYTHON
#define PYTHON_ERROR_CLEANUP_(method, before)	\
  cleanup:					\
  before;					\
  PYTHON_ERROR_CLEANUP_BASE_(method);		\
  if (!err.empty()) {				\
    throw std::runtime_error(err);		\
  }						\
  return out
#define PYTHON_ERROR_CLEANUP_NOTHROW_BASE_(method, before)	\
  UNUSED(err);							\
  before;							\
  END_PY_GIL
#define PYTHON_ERROR_CLEANUP_NOTHROW_(method, before)	\
  cleanup:						\
  UNUSED(err);						\
  before;						\
  END_PY_GIL;				\
  return out


template<typename Ch>
inline bool assign_wchar_PyUnicode(PyObject*, Py_ssize_t&, Ch*&)
{ return false; }
template<>
inline bool assign_wchar_PyUnicode<wchar_t>(PyObject* x, Py_ssize_t& py_len,
					    wchar_t*& free_orig) {
  free_orig = PyUnicode_AsWideCharString(x, &py_len);
  return true;
}

template<typename Encoding>
inline bool assign_char_PyUnicode(PyObject*, Py_ssize_t&,
				  const typename Encoding::Ch*&,
				  PyObject*&,
				  RAPIDJSON_DISABLEIF((internal::IsSame<typename Encoding::Ch,char>)))
{ return false; }
template<typename Encoding>
inline bool assign_char_PyUnicode(PyObject* x, Py_ssize_t& py_len,
				  const typename Encoding::Ch*& orig,
				  PyObject*& x2,
				  RAPIDJSON_ENABLEIF((internal::IsSame<typename Encoding::Ch,char>))) {
  bool out = false;
  BEGIN_PY_GIL;
#define ENCODING_BRANCH(enc)						\
  else if (internal::IsSame<Encoding, enc<typename Encoding::Ch> >::Value) { \
    x2 = PyUnicode_As ## enc ## String(x);				\
    if (x2 != NULL) {							\
      py_len = PyBytes_Size(x2);					\
      orig = PyBytes_AsString(x2);					\
      out = true;							\
      goto release;							\
    }									\
  }
  if (internal::IsSame<Encoding, UTF8<typename Encoding::Ch> >::Value) {
    orig = PyUnicode_AsUTF8AndSize(x, &py_len);
    out = true;
    goto release;
  }
  ENCODING_BRANCH(UTF16)
  ENCODING_BRANCH(UTF32)
  ENCODING_BRANCH(ASCII)
#undef ENCODING_BRANCH
 release:
  END_PY_GIL;
  return out;
}

template<typename Encoding, typename Allocator>
typename Encoding::Ch* PyUnicode_AsEncoding(PyObject* x, SizeType& length,
					    Allocator& allocator) {
  PyObject* x2 = NULL;
  typename Encoding::Ch* free_orig = NULL;
  const typename Encoding::Ch* orig = NULL;
  Py_ssize_t py_len = 0;
  BEGIN_PY_GIL;
  if (assign_wchar_PyUnicode(x, py_len, free_orig)) {
    orig = free_orig;
  } else {
    assign_char_PyUnicode<Encoding>(x, py_len, orig, x2);
  }
  length = (SizeType)py_len;
  typename Encoding::Ch* out = NULL;
  if (orig != NULL) {
    out = (typename Encoding::Ch*)allocator.Malloc(length * sizeof(typename Encoding::Ch));
    memcpy(out, orig, length * sizeof(typename Encoding::Ch));
    if (free_orig)
      PyMem_Free(free_orig);
  }
  Py_XDECREF(x2);
  END_PY_GIL;
  return out;
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
  PYTHON_ERROR_SETUP_;
  PyObject* out = NULL;
  RAPIDJSON_ASSERT(isPythonInitialized());
  if (!isPythonInitialized())
    goto cleanup;
  out = PyImport_ImportModule(module_name);
  PYTHON_ERROR_CLEANUP_(import_python_module, CHECK_REFS(out));
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
  PYTHON_ERROR_SETUP_;
  PyObject *py_module = import_python_module(module_name,
					     error_prefix,
					     ignore_error);
  if (py_module == NULL)
    goto cleanup;
  out = PyObject_GetAttrString(py_module, class_name);
  Py_DECREF(py_module);
  PYTHON_ERROR_CLEANUP_(import_python_class, CHECK_REFS(out, py_module));
}

template<typename Encoding, typename Allocator>
bool export_python_object(PyObject* x, typename Encoding::Ch*&mod_cls,
			  SizeType& mod_cls_siz,
			  Allocator& allocator) {
  typedef typename Encoding::Ch Ch;
  bool out = true;
  Ch *mod = NULL, *cls = NULL, *file = NULL, *curr = NULL;
  SizeType mod_len = 0, cls_len = 0, file_len = 0;
  PyObject *mod_py = NULL, *cls_py = NULL, *x_repr = NULL,
    *local_str = NULL, *file_py = NULL, *inspect = NULL,
    *inspect_getfile = NULL, *globals = NULL;
  int is_local = 0;
  BEGIN_PY_GIL;
  RAPIDJSON_ASSERT(PyObject_HasAttrString(x, "__module__"));
  RAPIDJSON_ASSERT(PyObject_HasAttrString(x, "__name__"));
  if (!(PyObject_HasAttrString(x, "__module__") && PyObject_HasAttrString(x, "__name__"))) {
    out = false;
    goto release;
  }
  // Get the module
  mod_py = PyObject_GetAttrString(x, "__module__");
  RAPIDJSON_ASSERT(mod_py != NULL);
  if (mod_py == NULL) {
    out = false;
    goto release;
  }
  // Get the class
  cls_py = PyObject_GetAttrString(x, "__name__");
  RAPIDJSON_ASSERT(cls_py != NULL);
  if (cls_py == NULL) {
    Py_DECREF(mod_py);
    out = false;
    goto release;
  }
  // Check for local function
  x_repr = PyObject_Repr(x);
  if (x_repr == NULL) {
    Py_DECREF(mod_py);
    Py_DECREF(cls_py);
    out = false;
    goto release;
  }
  local_str = PyUnicode_FromString("<locals>");
  if (local_str == NULL) {
    Py_DECREF(x_repr);
    Py_DECREF(mod_py);
    Py_DECREF(cls_py);
    out = false;
    goto release;
  }
  is_local = PySequence_Contains(x_repr, local_str);
  Py_DECREF(x_repr);
  Py_DECREF(local_str);
  if (is_local < 0) {
    Py_DECREF(mod_py);
    Py_DECREF(cls_py);
    out = false;
    goto release;
  }
  // Get file containing the module
  // Using the file alone is not portable
  file_py = NULL;
  if (is_local) {
    file_py = PyUnicode_FromString("local");
  } else {
    inspect = PyImport_ImportModule("inspect");
    if (inspect == NULL) {
      Py_DECREF(mod_py);
      Py_DECREF(cls_py);
      out = false;
      goto release;
    }
    inspect_getfile = PyObject_GetAttrString(inspect, "getfile");
    Py_DECREF(inspect);
    if (inspect_getfile == NULL) {
      Py_DECREF(mod_py);
      Py_DECREF(cls_py);
      out = false;
      goto release;
    }
    file_py = PyObject_CallFunction(inspect_getfile, "(O)", x);
    Py_DECREF(inspect_getfile);
  }
  if (file_py == NULL) {
    Py_DECREF(mod_py);
    Py_DECREF(cls_py);
    out = false;
    goto release;
  }
  mod = PyUnicode_AsEncoding<Encoding,Allocator>(mod_py, mod_len, allocator);
  cls = PyUnicode_AsEncoding<Encoding,Allocator>(cls_py, cls_len, allocator);
  curr = mod_cls;
  Py_DECREF(mod_py);
  Py_DECREF(cls_py);
  RAPIDJSON_ASSERT((mod != NULL) && (cls != NULL));
  if ((mod == NULL) || (cls == NULL)) {
    out = false;
    Py_XDECREF(file_py);
    goto release;
  }
  mod_cls_siz = mod_len + cls_len + 1;
  if (file_py != NULL) {
    file = PyUnicode_AsEncoding<Encoding,Allocator>(file_py, file_len, allocator);
    Py_DECREF(file_py);
    RAPIDJSON_ASSERT(file != NULL);
    if (file == NULL) {
      out = false;
      goto release;
    }
    if (file_len == 0) {
      allocator.Free(file);
      file = NULL;
    } else {
      mod_cls_siz += (file_len + 1);
    }
  }
  mod_cls = static_cast<Ch*>(allocator.Malloc((mod_cls_siz + 1) * sizeof(Ch)));
  RAPIDJSON_ASSERT(mod_cls);
  if (!mod_cls) {
    out = false;
    goto release;
  }
  curr = mod_cls;
  if (file_len > 0) {
    memcpy(curr, file, file_len * sizeof(Ch));
    curr[file_len] = (Ch)(':');
    curr += (file_len + 1);
    allocator.Free(file);
    file = NULL;
  }
  memcpy(curr, mod, mod_len * sizeof(Ch));
  curr[mod_len] = (Ch)(':');
  curr += (mod_len + 1);
  memcpy(curr, cls, cls_len);
  curr[cls_len] = '\0';
  allocator.Free(mod);
  allocator.Free(cls);
  mod = NULL;
  cls = NULL;
  if (is_local) {
    // Add local function to globals so that there is a chance of
    // deserializing if called in the same python session
    globals = PyEval_GetGlobals();
    // TODO: Handle encoding?
    if (PyDict_GetItemString(globals, (char*)mod_cls) != NULL) {
      out = false;
      goto release;
    }
    if (PyDict_SetItemString(globals, (char*)mod_cls, x) < 0) {
      out = false;
      goto release;
    }
  }
 release:
  CHECK_REFS(mod_py, cls_py, x_repr, local_str, file_py, inspect,
	     inspect_getfile, globals);
  END_PY_GIL;
  return out;
}

inline
PyObject* import_python_object(const char* mod_class,
			       const std::string error_prefix="",
			       const bool ignore_error=false) {
  PyObject *out = NULL, *globals = NULL, *tmpPy = NULL, *path_add = NULL,
    *path = NULL, *os_path = NULL, *path_abspath = NULL,
    *path_abs = NULL, *path_split = NULL, *path_parts = NULL,
    *path_dir = NULL, *path_base = NULL, *sys_path = NULL;
  char file_name[256] = "";
  char module_name[256] = "";
  char class_name[100] = "";
  size_t mod_class_len = strlen(mod_class);
  size_t iColon1 = mod_class_len, iColon2 = mod_class_len;
  size_t file_size = 0, module_size = 0, class_size = 0;
  bool ignore_error_1st = ignore_error, ends_with_py = false,
    is_local = false;
  Py_ssize_t tmp_size = 0;
  const char* tmp = NULL;
  PYTHON_ERROR_SETUP_;
  RAPIDJSON_ASSERT(mod_class_len <= 256);
  if (mod_class_len > 256) {
    if (!(ignore_error)) {
      err = error_prefix + "import_python_object: Name of module is greater that 256 characters"; // GCOVR_EXCL_LINE
    }
    out = NULL;
    goto cleanup;
  }
  for (size_t i = 0; i < mod_class_len; i++) {
    if (i > 1 && mod_class[i] == ':') {
      if (iColon1 == mod_class_len) {
	iColon1 = i;
      } else if (iColon2 == mod_class_len) {
	iColon2 = i;
      } else {
	if (!(ignore_error)) {
	  err = error_prefix + "import_python_object: Name of module has more than 3 colons in it: " + mod_class; // GCOVR_EXCL_LINE
	}
	out = NULL;
	goto cleanup;
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
    if (!(ignore_error)) {
      err = error_prefix + "import_python_object: Failed to import Python object '" + mod_class + "'"; // GCOVR_EXCL_LINE
    }
    out = NULL;
    goto cleanup;
  }
  ends_with_py = ((module_size > 3) && (strncmp(module_name + (module_size - 3), ".py", 3) == 0));
  if (ends_with_py) {
    if (file_size > 0) {
      if (!(ignore_error)) {
	err = error_prefix + "import_python_object: File specified and module is file: " + mod_class; // GCOVR_EXCL_LINE
      }
      out = NULL;
      goto cleanup;
    }
    strcpy(file_name, module_name);
    file_size = module_size;
    module_size = 0;
  } else {
    ends_with_py = ((file_size > 3) &&
		    (strncmp(file_name + (file_size - 3), ".py", 3) == 0));
  }
  is_local = (strcmp(file_name, "local") == 0);
  if (is_local) {
    // Look for local function added to the globals dictionary during
    // serialization.
    file_size = 0;
    globals = PyEval_GetGlobals();
    tmpPy = PyDict_GetItemString(globals, mod_class);
    if (tmpPy != NULL) {
      Py_INCREF(tmpPy);
    }
    out = tmpPy;
    goto cleanup;
  }
  path_add = NULL;
  if (file_size > 0) {
    if (ends_with_py)
      file_size -= 3;
    path = PyUnicode_FromStringAndSize(file_name, (Py_ssize_t)file_size);
    if (path == NULL)
      goto cleanup;
    os_path = PyImport_ImportModule("os.path");
    if (os_path == NULL) {
      Py_DECREF(path);
      goto cleanup;
    }
    path_abspath = PyObject_GetAttrString(os_path, "abspath");
    if (path_abspath == NULL) {
      Py_DECREF(path);
      Py_DECREF(os_path);
      goto cleanup;
    }
    path_abs = PyObject_CallFunction(path_abspath, "(O)", path);
    Py_DECREF(path_abspath);
    Py_DECREF(path);
    path = path_abs;
    path_abs = NULL;
    path_split = PyObject_GetAttrString(os_path, "split");
    Py_DECREF(os_path);
    if (path_split == NULL) {
      Py_DECREF(path);
      goto cleanup;
    }
    path_parts = PyObject_CallFunction(path_split, "(O)", path);
    Py_DECREF(path_split);
    if (path_parts == NULL) {
      Py_DECREF(path);
      goto cleanup;
    }
    path_dir = PyTuple_GetItem(path_parts, 0);
    if (path_dir == NULL) {
      Py_DECREF(path);
      Py_DECREF(path_parts);
      path_dir = NULL;
      goto cleanup;
    }
    if (ends_with_py || module_size == 0) {
      Py_INCREF(path_dir);
      path_add = path_dir;
    } else {
      Py_INCREF(path);
      path_add = path;
    }
    Py_DECREF(path);
    path_base = PyTuple_GetItem(path_parts, 1);
    if (path_base == NULL) {
      Py_DECREF(path_parts);
      path_dir = NULL;
      goto cleanup;
    }
    tmp = PyUnicode_AsUTF8AndSize(path_base, &tmp_size);
    if ((tmp == NULL) || (tmp_size >= 100)) {
      Py_DECREF(path_parts);
      path_base = NULL;
      path_dir = NULL;
      goto cleanup;
    }
    if (module_size == 0) {
      module_size = static_cast<size_t>(tmp_size);
      memcpy(module_name, tmp, module_size);
      module_name[module_size] = '\0';
    }
    Py_DECREF(path_parts);
    path_base = NULL;
    path_dir = NULL;
    ignore_error_1st = true;
  }
  out = import_python_class(module_name, class_name, error_prefix, ignore_error_1st);
  // Removing added path makes the object un-picklable
  if (out == NULL && file_size > 0) {
    // Add file path
    RAPIDJSON_ASSERT(path_add != NULL);
    if (path_add == NULL) {
      out = NULL;
      goto cleanup;
    }
    sys_path = PySys_GetObject("path");
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
    // PyObject_CallMethod(sys_path, "pop", "n", Py_SIZE(sys_path) - 1);
  }
  PYTHON_ERROR_CLEANUP_(import_python_object,
			CHECK_REFS(out, globals, tmpPy, path_add, path,
				   os_path, path_abspath,
				   path_abs, path_split, path_parts,
				   path_dir, path_base, sys_path));
}


inline
PyObject* pickle_python_object(PyObject* x,
			       const std::string error_prefix="",
			       const bool ignore_error=false) {
  PyObject *out = NULL, *args = NULL;
  PYTHON_ERROR_SETUP_;
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
  PYTHON_ERROR_CLEANUP_(pickle_python_object,
			CHECK_REFS(out, args, pickleMethod));
}

inline
PyObject* unpickle_python_object(const char* buffer,
				 size_t buffer_length,
				 const std::string error_prefix="",
				 const bool ignore_error=false) {
  PyObject *out = NULL, *py_str = NULL, *args = NULL;
  PYTHON_ERROR_SETUP_;
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
  PYTHON_ERROR_CLEANUP_(unpickle_python_object,
			CHECK_REFS(out, py_str, args, pickle));
}


inline
bool PyObject_IsInstanceString(PyObject* x, std::string class_name) {
  bool out = true;
  PyObject *inst_class = NULL, *inst_class_str = NULL;
  std::string result, check;
  PYTHON_ERROR_SETUP_;
  if (!PyObject_HasAttrString(x, "__class__")) {
    out = false;
    goto cleanup;
  }
  inst_class = PyObject_GetAttrString(x, "__class__");
  if (inst_class == NULL) {
    out = false;
    goto cleanup;
  }
  inst_class_str = PyObject_Str(inst_class);
  Py_DECREF(inst_class);
  if (inst_class_str == NULL) {
    out = false;
    goto cleanup;
  }
  result.assign(PyUnicode_AsUTF8(inst_class_str));
  Py_DECREF(inst_class_str);
  check = "<class '" + class_name + "'>";
  out = (check == result);
  PYTHON_ERROR_CLEANUP_NOTHROW_(PyObject_IsInstanceString,
				CHECK_REFS(inst_class, inst_class_str));
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
  bool out = true;
  int nd = 0, j = 0;
  npy_intp *dims = NULL, *idims = NULL;
  Py_ssize_t N = 0, i = 0;
  PyObject *item = NULL;
  PyArray_Descr *desc = NULL;
  PYTHON_ERROR_SETUP_;
  if (x == NULL || !PyList_Check(x))
    PYTHON_ERROR_EXIT_(false);
  N = PyList_Size(x);
  if (N == 0)
    PYTHON_ERROR_EXIT_(false);
  for (i = 0; i < N; i++) {
    item = PyList_GetItem(x, i);
    RAPIDJSON_ASSERT(item);
    if (item == NULL)
      PYTHON_ERROR_EXIT_(false);
    if (!PyArray_CheckExact(item))
      PYTHON_ERROR_EXIT_(false);
    desc = PyArray_DESCR((PyArrayObject*)item);
    RAPIDJSON_ASSERT(desc);
    if (desc == NULL)
      PYTHON_ERROR_EXIT_(false);
    if (desc->names == NULL)
      PYTHON_ERROR_EXIT_(false);
    if (PyTuple_Size(desc->names) != 1)
      PYTHON_ERROR_EXIT_(false);
    if (dims == NULL) {
      if (i > 0)
	PYTHON_ERROR_EXIT_(false);
      nd = PyArray_NDIM((PyArrayObject*)item);
      dims = PyArray_DIMS((PyArrayObject*)item);
    } else {
      if (nd != PyArray_NDIM((PyArrayObject*)item))
	PYTHON_ERROR_EXIT_(false);
      for (j = 0; j < nd; j++) {
	idims = PyArray_DIMS((PyArrayObject*)item);
	if (dims[j] != idims[j])
	  PYTHON_ERROR_EXIT_(false);
      }
    }
  }
  PYTHON_ERROR_CLEANUP_NOTHROW_(IsStructuredArray,
				CHECK_REFS(x, item));
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
  PYTHON_ERROR_SETUP_;
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
  PYTHON_ERROR_CLEANUP_NOTHROW_BASE_(GetStructuredArray,
				     CHECK_REFS(out, names, fields,
						ikey, idtype, ioffset, ifield));
  return out;
#endif // RAPIDJSON_DONT_IMPORT_NUMPY
}

template<typename Ch>
class CleanupLocals {
public:
#define METHOD_NOARGS(name)			\
  bool name() { return true; }
#define METHOD_ARGS(name, ...)			\
  bool name(__VA_ARGS__) { return true; }
  CleanupLocals() {}
  METHOD_NOARGS(Null)
  METHOD_ARGS(Bool, bool)
  METHOD_ARGS(Int, int)
  METHOD_ARGS(Uint, unsigned)
  METHOD_ARGS(Int64, int64_t)
  METHOD_ARGS(Uint64, uint64_t)
  METHOD_ARGS(Double, double)
  METHOD_ARGS(String, const Ch*, SizeType, bool)
  METHOD_NOARGS(StartObject)
  METHOD_ARGS(Key, const Ch*, SizeType, bool)
  METHOD_ARGS(EndObject, SizeType)
  METHOD_NOARGS(StartArray)
  METHOD_ARGS(EndArray, SizeType)
  template <typename YggSchemaValueType>
  bool YggdrasilString(const Ch* str, SizeType length, bool, YggSchemaValueType& valueSchema) {
    RAPIDJSON_ASSERT(valueSchema.IsObject());
    if (!valueSchema.IsObject())
      return false;
    const Ch local_str[] = {'l', 'o', 'c', 'a', 'l', ':', '\0'};
    if (length > 6 && (memcmp(str, local_str, 6 * sizeof(Ch)) == 0)) {
      typename YggSchemaValueType::ConstMemberIterator typeV = valueSchema.FindMember(YggSchemaValueType::GetTypeString());
      if (typeV != valueSchema.MemberEnd() &&
	  (typeV->value == YggSchemaValueType::GetPythonClassString() ||
	   typeV->value == YggSchemaValueType::GetPythonFunctionString())) {
	BEGIN_PY_GIL;
	PyObject* globals = PyEval_GetGlobals();
	if (PyDict_GetItemString(globals, (char*)str) != NULL) {
	  PyDict_DelItemString(globals, (char*)str);
	}
	END_PY_GIL;
      }
    }
    return true;
  }
  template <typename YggSchemaValueType>
  METHOD_ARGS(YggdrasilStartObject, YggSchemaValueType&)
  METHOD_ARGS(YggdrasilEndObject, SizeType)
};

RAPIDJSON_NAMESPACE_END

#endif // YGGDRASIL_DISABLE_PYTHON_C_API

#endif // RAPIDJSON_PYTHON_H_
