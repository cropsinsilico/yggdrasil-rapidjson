macro(yggdrasil_rapidjson_options OUTPUT_VARIABLE)
  set(
    ${OUTPUT_VARIABLE}
    DISABLE_YGGDRASIL_RAPIDJSON
    YGGDRASIL_PYGIL_NO_MANAGEMENT
    YGGDRASIL_DISABLE_PYTHON_C_API
    YGGDRASIL_RAPIDJSON_DISABLE_STDSTRING
    YGGDRASIL_RAPIDJSON_USE_MEMBERSMAP
    YGGDRASIL_RAPIDJSON_CHECK_PYREFS
    YGGDRASIL_RAPIDJSON_PYTHON_WRAPPER
    YGGDRASIL_RAPIDJSON_BUILD_ASAN
    YGGDRASIL_RAPIDJSON_BUILD_UBSAN
    YGGDRASIL_RAPIDJSON_ENABLE_INSTRUMENTATION_OPT
  )
endmacro()

macro(yggdrasil_rapidjson_options_create)
  option(DISABLE_YGGDRASIL_RAPIDJSON "Build without yggdrasil RapidJSON extensions" OFF)
  option(YGGDRASIL_PYGIL_NO_MANAGEMENT "Disable any handling of the Python GIL in YggdrasilRapidJSON" OFF)
  option(YGGDRASIL_DISABLE_PYTHON_C_API "Disable the Python C API in YggdrasilRapidJSON" OFF)
  option(YGGDRASIL_RAPIDJSON_DISABLE_STDSTRING "Disable the use of the C++ std::string library in YggdrasilRapidJSON" OFF)
  option(YGGDRASIL_RAPIDJSON_USE_MEMBERSMAP "Enable the use of the C++ std::map library in YggdrasilRapidJSON" OFF)
  option(YGGDRASIL_RAPIDJSON_CHECK_PYREFS "Turn on tracking of Python references in YggdrasilRapidJSON" OFF)
  option(YGGDRASIL_RAPIDJSON_PYTHON_WRAPPER "Set flags to use YggdrasilRapidJSON as part of a Python C extension" OFF)
  option(YGGDRASIL_RAPIDJSON_BUILD_ASAN "Build yggdrasil_rapidjson with address sanitizer (gcc/clang)" OFF)
  option(YGGDRASIL_RAPIDJSON_BUILD_UBSAN "Build yggdrasil_rapidjson with undefined behavior sanitizer (gcc/clang)" OFF)
  option(YGGDRASIL_RAPIDJSON_ENABLE_INSTRUMENTATION_OPT "Build yggdrasil_rapidjson with -march or -mcpu options" ON)
endmacro()

macro(yggdrasil_rapidjson_options_config OUTPUT_PREFIX)
  if(DISABLE_YGGDRASIL_RAPIDJSON)
    list(
      APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS
      -DDISABLE_YGGDRASIL_RAPIDJSON
      -DYGGDRASIL_DISABLE_PYTHON_C_API
    )
    set(YGGDRASIL_PYGIL_NO_MANAGEMENT OFF)
    set(YGGDRASIL_DISABLE_PYTHON_C_API ON)
    set(YGGDRASIL_RAPIDJSON_CHECK_PYREFS OFF)
  else()
    if (YGGDRASIL_PYGIL_NO_MANAGEMENT)
      list(
        APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS
        -DYGGDRASIL_PYGIL_NO_MANAGEMENT
      )
    endif()
    if(YGGDRASIL_DISABLE_PYTHON_C_API)
      list(
        APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS
        -DYGGDRASIL_DISABLE_PYTHON_C_API
      )
    else()
      if(NOT YGGDRASIL_RAPIDJSON_PYTHON_WRAPPER)
        list(APPEND ${OUTPUT_PREFIX}_LIBRARIES Python3::Python)
      endif()
      list(APPEND ${OUTPUT_PREFIX}_LIBRARIES Python3::NumPy)
      list(
          APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS
          -DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION
      )
      if(YGGDRASIL_RAPIDJSON_CHECK_PYREFS)
        list(
          APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS
          -DYGGDRASIL_RAPIDJSON_CHECK_PYREFS -DPy_DEBUG
        )
      endif()
    endif()
  endif()
  if(NOT YGGDRASIL_RAPIDJSON_DISABLE_STDSTRING)
    list(APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS -DYGGDRASIL_RAPIDJSON_HAS_STDSTRING)
  endif()
  if(YGGDRASIL_RAPIDJSON_USE_MEMBERSMAP)
    list(
      APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS
      -DYGGDRASIL_RAPIDJSON_USE_MEMBERSMAP=1
    )
  endif()
  if(YGGDRASIL_RAPIDJSON_PYTHON_WRAPPER)
    list(
      APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS
      -DYGGDRASIL_RAPIDJSON_PYTHON_WRAPPER
    )
  endif()
  if(YGGDRASIL_RAPIDJSON_BUILD_UBSAN)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.0")
        message(FATAL_ERROR "GCC < 4.8 doesn't support the address sanitizer")
      else()
	list(APPEND ${OUTPUT_PREFIX}_ASAN_COMPILE_FLAGS -fsanitize=address)
	list(APPEND ${OUTPUT_PREFIX}_ASAN_LINK_FLAGS -fsanitize=address)
      endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      list(APPEND ${OUTPUT_PREFIX}_ASAN_COMPILE_FLAGS -fsanitize=address)
      list(APPEND ${OUTPUT_PREFIX}_ASAN_LINK_FLAGS -fsanitize=address)
    else()
      message(FATAL_ERROR "ASAN unsupported by compiler ${CMAKE_CXX_COMPILER_ID}")
    endif()
  endif()
  if(YGGDRASIL_RAPIDJSON_BUILD_ASAN)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.0")
        message(FATAL_ERROR "GCC < 4.9 doesn't support the undefined behavior sanitizer")
      else()
        list(APPEND ${OUTPUT_PREFIX}_ASAN_COMPILE_FLAGS -fsanitize=undefined)
        list(APPEND ${OUTPUT_PREFIX}_ASAN_LINK_FLAGS -fsanitize=undefined)
      endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        list(APPEND ${OUTPUT_PREFIX}_ASAN_COMPILE_FLAGS -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error)
        list(APPEND ${OUTPUT_PREFIX}_ASAN_LINK_FLAGS -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error)
      else()
        list(APPEND ${OUTPUT_PREFIX}_ASAN_COMPILE_FLAGS -fsanitize=undefined)
        list(APPEND ${OUTPUT_PREFIX}_ASAN_LINK_FLAGS -fsanitize=undefined)
      endif()
    else()
      message(FATAL_ERROR "UBSAN unsupported by compiler ${CMAKE_CXX_COMPILER_ID}")
    endif()
  endif()
  if(${OUTPUT_PREFIX}_ASAN_COMPILE_FLAGS)
    list(
      APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS
      ${${OUTPUT_PREFIX}_ASAN_COMPILE_FLAGS}
    )
  endif()
  if(${OUTPUT_PREFIX}_ASAN_LINK_FLAGS)
    list(
      APPEND ${OUTPUT_PREFIX}_PUBLIC_LINK_FLAGS
      ${${OUTPUT_PREFIX}_ASAN_COMPILE_FLAGS}
    )
  endif()

  if(YGGDRASIL_RAPIDJSON_ENABLE_INSTRUMENTATION_OPT
     AND (NOT CMAKE_CROSSCOMPILING)
     AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64le")
      list(
        APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS
        -mcpu=native
      )
    else()
      #FIXME: x86 is -march=native, but doesn't mean every arch is this option. To keep original project's compatibility, I leave this except POWER.
      list(
        APPEND ${OUTPUT_PREFIX}_PUBLIC_COMPILE_FLAGS
        -march=native
      )
    endif()
  endif()

endmacro()
