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

macro(yggdrasil_rapidjson_config_vars PREFIX)
  list(
    APPEND ${PREFIX}_CONFIG_VARS
    ${PREFIX}_ASAN_COMPILE_FLAGS
  )
  foreach(tool GNU Clang AppleClang MSVC)
    list(
      APPEND ${PREFIX}_CONFIG_VARS
      ${PREFIX}_${tool}_ASAN_COMPILE_FLAGS
    )
  endforeach()
  foreach(suffix LIBRARIES INCLUDE_DIRS COMPILE_FLAGS LINK_FLAGS)
    list(
      APPEND ${PREFIX}_CONFIG_VARS
      ${PREFIX}_${suffix}
      ${PREFIX}_PUBLIC_${suffix}
      ${PREFIX}_PRIVATE_${suffix}
    )
    set(suffix_lang ${suffix})
    if(suffix STREQUAL "LIBRARIES")
      set(suffix_lang "LIBS")
    endif()
    foreach(lang C CXX Fortran)
      list(
        APPEND ${PREFIX}_CONFIG_VARS
        ${PREFIX}_PUBLIC_${lang}_${suffix_lang}
        ${PREFIX}_PRIVATE_${lang}_${suffix_lang}
      )
    endforeach()
    foreach(tool GNU Clang AppleClang MSVC)
      list(
        APPEND ${PREFIX}_CONFIG_VARS
        ${PREFIX}_PUBLIC_${tool}_${suffix_lang}
        ${PREFIX}_PRIVATE_${tool}_${suffix_lang}
      )
    endforeach()
  endforeach()
endmacro()

macro(yggdrasil_rapidjson_config_init PREFIX)
  yggdrasil_rapidjson_config_vars(${PREFIX})
  foreach(var IN LISTS ${PREFIX}_CONFIG_VARS)
    set(${var})
  endforeach()
endmacro()

macro(yggdrasil_rapidjson_config_show PREFIX LEVEL)
  foreach(var IN LISTS ${PREFIX}_CONFIG_VARS)
    message(${LEVEL} "${var} = ${${var}}")
  endforeach()
endmacro()

macro(yggdrasil_rapidjson_config_accum PREFIX)
  foreach(suffix LIBRARIES INCLUDE_DIRS COMPILE_FLAGS LINK_FLAGS)
    set(${PREFIX}_ALL_PUBLIC_${suffix} ${${PREFIX}_PUBLIC_${suffix}}
        ${${PREFIX}_${suffix}})
    set(${PREFIX}_ALL_PRIVATE_${suffix} ${${PREFIX}_PRIVATE_${suffix}})
    if(suffix STREQUAL "LIBRARIES" OR suffix STREQUAL "LINK_FLAGS")
      set(gentype "LINK")
    else()
      set(gentype "COMPILE")
    endif()
    set(suffix_lang ${suffix})
    if(suffix STREQUAL "LIBRARIES")
      set(suffix_lang "LIBS")
    endif()
    foreach(lang C CXX Fortran)
      set(kpublic ${PREFIX}_PUBLIC_${lang}_${suffix_lang})
      set(kprivate ${PREFIX}_PRIVATE_${lang}_${suffix_lang})
      if(${kpublic})
        list(
          APPEND ${PREFIX}_ALL_PUBLIC_${suffix}
          "$<$<${gentype}_LANGUAGE:${lang}>:${${kpublic}}>"
        )
      endif()
      if(${kprivate})
        list(
          APPEND ${PREFIX}_ALL_PRIVATE_${suffix}
          "$<$<${gentype}_LANGUAGE:${lang}>:${${kprivate}}>"
        )
      endif()
    endforeach()
    foreach(tool GNU Clang AppleClang MSVC)
      set(k ${PREFIX}_PUBLIC_${tool}_${suffix_lang})
      if(NOT ${k})
        continue()
      endif()
      set(langlist C CXX)
      set(toollist ${tool})
      if(tool STREQUAL "GNU")
        list(APPEND langlist Fortran)
      endif()
      foreach(lang IN LISTS langlist)
        list(
          APPEND ${PREFIX}_ALL_PUBLIC_${suffix}
          $<$<${gentype}_LANG_AND_ID:${lang},${tool}>:${${k}}>
        )
      endforeach()
    endforeach()
    message(DEBUG "${PREFIX}_ALL_PUBLIC_${suffix} = ${${PREFIX}_ALL_PUBLIC_${suffix}}")
    message(DEBUG "${PREFIX}_ALL_PRIVATE_${suffix} = ${${PREFIX}_ALL_PRIVATE_${suffix}}")
  endforeach()
endmacro()

macro(yggdrasil_rapidjson_global_compiler_flags LANGUAGE)
  # TODO: Some of these should be interface flags

  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    if (CMAKE_${LANGUAGE}_COMPILER_ID MATCHES "Clang")
      set(CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS} -Qunused-arguments -fcolor-diagnostics")
    endif()
  endif(CCACHE_FOUND)

  find_program(VALGRIND_FOUND valgrind)

  if(CMAKE_${LANGUAGE}_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS} -Wall -Wextra -Werror")
    set(EXTRA_${LANGUAGE}_FLAGS -Weffc++ -Wswitch-default -Wfloat-equal -Wconversion -Wsign-conversion)
  elseif (CMAKE_${LANGUAGE}_COMPILER_ID MATCHES "Clang")
    if(NOT CMAKE_CROSSCOMPILING)
      if(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc64le")
        set(CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS} -mcpu=native")
      else()
        if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64" OR
           CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
	  set(CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS} -mcpu=apple-m1")
	else()
          #FIXME: x86 is -march=native, but doesn't mean every arch is this option. To keep original project's compatibility, I leave this except POWER.
          set(CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS} -march=native")
        endif()
      endif()
    endif()
    set(CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS} -Wall -Wextra -Werror -Wno-missing-field-initializers")
    set(EXTRA_${LANGUAGE}_FLAGS -Weffc++ -Wswitch-default -Wfloat-equal -Wconversion -Wimplicit-fallthrough)
  elseif (CMAKE_${LANGUAGE}_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS} /EHsc /bigobj /Zm10")
    # CMake >= 3.10 should handle the above CMAKE_CXX_STANDARD fine, otherwise use /std:c++XX with MSVC >= 19.10
    # Always compile with /WX
    if(CMAKE_${LANGUAGE}_FLAGS MATCHES "/WX-")
      string(REGEX REPLACE "/WX-" "/WX" CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS}")
    else()
      set(CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS} /WX")
    endif()
  elseif (CMAKE_${LANGUAGE}_COMPILER_ID MATCHES "XL")
    set(CMAKE_${LANGUAGE}_FLAGS "${CMAKE_${LANGUAGE}_FLAGS} -qarch=auto")
  endif()
endmacro()

macro(yggdrasil_rapidjson_global_config LANGUAGE PREFIX)
  yggdrasil_rapidjson_config_show(${PREFIX} DEBUG)
  set(global_compiler ${CMAKE_${LANGUAGE}_COMPILER_ID})
  set(global_linker ${global_compiler})
  list(
    APPEND EXTRA_${LANGUAGE}_FLAGS
    ${${PREFIX}_PUBLIC_COMPILE_FLAGS}
    ${${PREFIX}_PUBLIC_${LANGUAGE}_COMPILE_FLAGS}
    ${${PREFIX}_PUBLIC_${global_compiler}_COMPILE_FLAGS}
  )
  add_compile_options(
    ${${PREFIX}_COMPILE_FLAGS}
    ${${PREFIX}_PUBLIC_COMPILE_FLAGS}
    ${${PREFIX}_PUBLIC_${LANGUAGE}_COMPILE_FLAGS}
    ${${PREFIX}_PUBLIC_${global_compiler}_COMPILE_FLAGS}
  )
  add_link_options(
    ${${PREFIX}_LINK_FLAGS}
    ${${PREFIX}_PUBLIC_LINK_FLAGS}
    ${${PREFIX}_PUBLIC_${LANGUAGE}_LINK_FLAGS}
    ${${PREFIX}_PUBLIC_${global_linker}_LINK_FLAGS}
  )
  include_directories(
    ${${PREFIX}_INCLUDE_DIRS}
    ${${PREFIX}_PUBLIC_INCLUDE_DIRS}
    ${${PREFIX}_PUBLIC_${LANGUAGE}_INCLUDE_DIRS}
  )
  link_libraries(
    ${${PREFIX}_LIBRARIES}
    ${${PREFIX}_PUBLIC_LIBRARIES}
    ${${PREFIX}_PUBLIC_${LANGUAGE}_LIBRARIES}
  )
endmacro()

macro(yggdrasil_rapidjson_target_config TARGET TYPE PREFIX)
  yggdrasil_rapidjson_config_show(${PREFIX} DEBUG)
  yggdrasil_rapidjson_config_accum(${PREFIX})
  if(${PREFIX}_ALL_PUBLIC_LIBRARIES)
    target_link_libraries(
      ${TARGET} ${TYPE} ${${PREFIX}_ALL_PUBLIC_LIBRARIES}
    )
  endif()
  if(${PREFIX}_ALL_PUBLIC_INCLUDE_DIRS)
    target_include_directories(
      ${TARGET} ${TYPE} ${${PREFIX}_ALL_PUBLIC_INCLUDE_DIRS}
    )
  endif()
  if(${PREFIX}_ALL_PUBLIC_COMPILE_FLAGS)
    target_compile_options(
      ${TARGET} ${TYPE} ${${PREFIX}_ALL_PUBLIC_COMPILE_FLAGS}
    )
  endif()
  if(${PREFIX}_ALL_PUBLIC_LINK_FLAGS)
    target_link_options(
      ${TARGET} ${TYPE} ${${PREFIX}_ALL_PUBLIC_LINK_FLAGS}
    )
  endif()
  if(NOT ("${TYPE}" STREQUAL "INTERFACE"))
    if(${PREFIX}_ALL_PRIVATE_LIBRARIES)
      target_link_libraries(
        ${TARGET} PRIVATE ${${PREFIX}_ALL_PRIVATE_LIBRARIES}
      )
    endif()
    if(${PREFIX}_ALL_PRIVATE_INCLUDE_DIRS)
      target_include_directories(
        ${TARGET} PRIVATE ${${PREFIX}_ALL_PRIVATE_INCLUDE_DIRS}
      )
    endif()
    if(${PREFIX}_ALL_PRIVATE_COMPILE_FLAGS)
      target_compile_options(
        ${TARGET} PRIVATE ${${PREFIX}_ALL_PRIVATE_COMPILE_FLAGS}
      )
    endif()
    if(${PREFIX}_ALL_PRIVATE_LINK_FLAGS)
      target_link_options(
        ${TARGET} PRIVATE ${${PREFIX}_ALL_PRIVATE_LINK_FLAGS}
      )
    endif()
  endif()
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
        list(APPEND ${OUTPUT_PREFIX}_PUBLIC_LIBRARIES Python3::Python)
      endif()
      list(APPEND ${OUTPUT_PREFIX}_PUBLIC_LIBRARIES Python3::NumPy)
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
  if(YGGDRASIL_RAPIDJSON_BUILD_ASAN)
    foreach(tool GNU Clang AppleClang)
      list(
        APPEND ${OUTPUT_PREFIX}_${tool}_ASAN_COMPILE_FLAGS
        -fsanitize=address
      )
    endforeach()
  endif()
  if(YGGDRASIL_RAPIDJSON_BUILD_UBSAN)
    foreach(tool GNU Clang)
      list(
        APPEND ${OUTPUT_PREFIX}_${tool}_ASAN_COMPILE_FLAGS
        -fsanitize=undefined
      )
    endforeach()
    list(APPEND ${OUTPUT_PREFIX}_AppleClang_ASAN_COMPILE_FLAGS
         -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error)
  endif()
  if(YGGDRASIL_RAPIDJSON_BUILD_ASAN OR YGGDRASIL_RAPIDJSON_BUILD_UBSAN
     AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libclang_rt.asan_osx_dynamic.dylib
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE ${OUTPUT_PREFIX}_ASAN_LIB
      RESULT_VARIABLE ${OUTPUT_PREFIX}_ASAN_RESULT
    )
    if(${OUTPUT_PREFIX}_ASAN_RESULT)
      set(${OUTPUT_PREFIX}_ASAN_LIB)
    endif()
  endif()
  set(
    ${OUTPUT_PREFIX}_ASAN_COMPILE_FLAGS
    ${${OUTPUT_PREFIX}_${CMAKE_CXX_COMPILER_ID}_ASAN_COMPILE_FLAGS}
  )
  foreach(tool GNU Clang AppleClang MSVC)
    set(k ${OUTPUT_PREFIX}_${tool}_ASAN_COMPILE_FLAGS)
    if(${k})
      foreach(suffix COMPILE_FLAGS LINK_FLAGS)
        list(APPEND ${OUTPUT_PREFIX}_PUBLIC_${tool}_${suffix} ${${k}})
      endforeach()
    endif()
  endforeach()

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

macro(yggdrasil_rapidjson_gitversion OUTPUT_VARIABLE DEFAULT)
  find_package(Git)
  if(NOT Git_FOUND)
    message(STATUS "Failed to find Git cmake package, falling back to version ${DEFAULT}")
    set(${OUTPUT_VARIABLE} ${DEFAULT})
  else()
    # Generate a git-describe version string from Git repository tags
    execute_process(
      COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --match "v*"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
      RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(GIT_DESCRIBE_ERROR_CODE)
      message(STATUS "Could not locate a git tag, falling back to version ${DEFAULT}")
      set(${OUTPUT_VARIABLE} ${DEFAULT})
    else()
      string(SUBSTRING "${GIT_DESCRIBE_VERSION}" 1 -1 GIT_DESCRIBE_VERSION)
      string(FIND "${GIT_DESCRIBE_VERSION}" "-" idx)
      if(NOT ${idx} EQUAL "-1")
        string(SUBSTRING "${GIT_DESCRIBE_VERSION}" 0 ${idx} GIT_DESCRIBE_VERSION)
        string(FIND "${GIT_DESCRIBE_VERSION}" "." idx REVERSE)
        math(EXPR idxp1 "${idx}+1")
        string(SUBSTRING "${GIT_DESCRIBE_VERSION}" ${idxp1} -1 EXTEN_VERSION)
        math(EXPR EXTEN_VERSION "${EXTEN_VERSION}+1")
        string(SUBSTRING "${GIT_DESCRIBE_VERSION}" 0 ${idx} GIT_DESCRIBE_VERSION)
        set(GIT_DESCRIBE_VERSION "${GIT_DESCRIBE_VERSION}.${EXTEN_VERSION}")
      endif()
      set(${OUTPUT_VARIABLE} ${GIT_DESCRIBE_VERSION})
    endif()
  endif()
endmacro()

function(yggdrasil_rapidjson_version_header MACRO_PREFIX VERSION_STRING SRC DST)
  set(rem ${VERSION_STRING})
  set(idx 0)
  set(parts)
  while(NOT ${idx} EQUAL "-1")
    string(FIND "${rem}" "." idx)
    if(${idx} EQUAL "-1")
      list(APPEND parts "${rem}")
    else()
      string(SUBSTRING "${rem}" 0 ${idx} part)
      list(APPEND parts "${part}")
      math(EXPR idxp1 "${idx}+1")
      string(SUBSTRING "${rem}" ${idxp1} -1 rem)
    endif()
  endwhile()
  list(GET parts 0 MAJOR_VERSION)
  list(GET parts 1 MINOR_VERSION)
  list(GET parts 2 PATCH_VERSION)
  list(GET parts 3 EXTEN_VERSION)
  configure_file(${SRC} ${DST} @ONLY)
endfunction()