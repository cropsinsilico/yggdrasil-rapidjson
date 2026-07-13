set -e
REBUILD=""
BUILD_DIR=""
INSTALL_DIR=""
while [[ $# -gt 0 ]]; do
    case $1 in
        --rebuild )
            REBUILD="TRUE"
	    shift
	    ;;
        --build-dir )
            BUILD_DIR="$2"
	    shift
	    shift # past argument with value
	    ;;
        --install-dir )
            INSTALL_DIR="$2"
	    shift
	    shift # past argument with value
	    ;;
    esac
done
if [ ! -n "${BUILD_DIR}" ]; then
    BUILD_DIR="$(pwd)/build"
fi
if [ ! -n "${INSTALL_DIR}" ]; then
    INSTALL_DIR="$(pwd)/_install"
fi
if [ -n "$REBUILD" ]; then
    if [ -d ${BUILD_DIR} ]; then
        rm -rf ${BUILD_DIR}
    fi
fi
if [ ! -d ${BUILD_DIR} ]; then
    mkdir ${BUILD_DIR}
fi
if [ ! -d ${INSTALL_DIR} ]; then
    mkdir ${INSTALL_DIR}
fi

cmake -G Ninja -B ${BUILD_DIR} -S $(pwd) \
      -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
      -DYGGDRASIL_RAPIDJSON_SKIP_VALGRIND_TESTS:BOOL=ON \
      -DYGGDRASIL_RAPIDJSON_CREATE_METASCHEMA_FULL:BOOL=ON \
      -DYGGDRASIL_RAPIDJSON_ENABLE_COVERAGE:BOOL=OFF \
      -DYGGDRASIL_RAPIDJSON_BUILD_TESTS:BOOL=ON \
      -DYGGDRASIL_RAPIDJSON_BUILD_UNITTESTS:BOOL=ON \
      -DYGGDRASIL_RAPIDJSON_BUILD_PERFTESTS:BOOL=OFF \
      -DYGGDRASIL_RAPIDJSON_SCHEMA_TESTS:BOOL=ON \
      -DYGGDRASIL_RAPIDJSON_BUILD_UBSAN:BOOL=ON \
      -DYGGDRASIL_RAPIDJSON_BUILD_ASAN:BOOL=ON \
      -DYGGDRASIL_RAPIDJSON_BUILD_EXAMPLES:BOOL=ON \
      -DYGGDRASIL_RAPIDJSON_BUILD_DOC:BOOL=OFF
# -DCMAKE_BUILD_TYPE=Debug
# -DCMAKE_INSTALL_PREFIX:FILEPATH=${INSTALL_DIR}

# Tests
cmake --build ${BUILD_DIR} --target=tests -- -j 8
cd ${BUILD_DIR}
ctest -R unittest --stop-on-failure
# ctest -R perftest --stop-on-failure

# Examples
# cmake --build ${BUILD_DIR} --target=examples -- -j 8
# ${BUILD_DIR}/bin/yggdrasil
# ${BUILD_DIR}/bin/${example_name}

# Install
cmake --install ${BUILD_DIR} --prefix "${INSTALL_DIR}"


# ctest -R coverage
# # cmake .. -DYGGDRASIL_RAPIDJSON_SKIP_VALGRIND_TESTS=ON -DYGGDRASIL_RAPIDJSON_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
# # ctest -T Coverage
# # ./bin/unittest
# # export DATADIR=/Users/langmm/rapidjson/test
# # export YGG_PYTHON_EXEC=/Users/langmm/miniconda3/envs/conda37/bin/python
# # valgrind --leak-check=full   --show-leak-kinds=all --dsymutil=no --track-origins=yes -v --suppressions=/Users/langmm/valgrind-macos/darwin13.supp ./bin/unittest &> log.txt
# # --suppressions=/Users/langmm/valgrind-macos/default.supp ./bin/unittest &> log.txt
