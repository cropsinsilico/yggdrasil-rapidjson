#!/bin/sh
set -euo pipefail

PYTHON=${PYTHON:-python}
Python_INCLUDE_DIR="$(${PYTHON} -c 'import sysconfig; print(sysconfig.get_path("include"))')"
Python_NumPy_INCLUDE_DIR="$(${PYTHON} -c 'import numpy; print(numpy.get_include())')"

CMAKE_ARGS+=" -DPython3_EXECUTABLE:PATH=${PYTHON}"
CMAKE_ARGS+=" -DPython3_INCLUDE_DIR:PATH=${Python_INCLUDE_DIR}"
CMAKE_ARGS+=" -DPython3_NumPy_INCLUDE_DIR=${Python_NumPy_INCLUDE_DIR}"

cmake -B build -S ${SRC_DIR} \
      -G "Ninja" \
      -D YGGDRASIL_RAPIDJSON_BUILD_TESTS:BOOL=OFF \
      -D YGGDRASIL_RAPIDJSON_BUILD_EXAMPLES:BOOL=OFF \
      -D YGGDRASIL_RAPIDJSON_BUILD_DOC:BOOL=OFF \
      -D YGGDRASIL_RAPIDJSON_VERSION=${PKG_VERSION} \
      -D CMAKE_VERBOSE_MAKEFILE:BOOL=ON \
       ${CMAKE_ARGS}
cmake --build build -j${CPU_COUNT}
cmake --install build
