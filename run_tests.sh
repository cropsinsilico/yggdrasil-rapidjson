set -e
INSTALL_DIR="$(pwd)/_install"
# rm -rf build
if [ ! -d build ]; then
    mkdir build
fi
if [ ! -d ${INSTALL_DIR} ]; then
    mkdir ${INSTALL_DIR}
fi
cd build
# cmake .. -DRAPIDJSON_HAS_STDSTRING:BOOL=ON -DRAPIDJSON_BUILD_TESTS:BOOL=OFF -DRAPIDJSON_BUILD_EXAMPLES:BOOL=OFF -DRAPIDJSON_BUILD_DOC:BOOL=OFF -DCMAKE_INSTALL_PREFIX:FILEPATH=${INSTALL_DIR}
# cmake --install . --prefix "${INSTALL_DIR}"
cmake .. -DRAPIDJSON_SKIP_VALGRIND_TESTS=ON -DRAPIDJSON_ENABLE_COVERAGE=OFF -DCMAKE_BUILD_TYPE=Debug -DRAPIDJSON_CREATE_METASCHEMA_FULL=ON -DRAPIDJSON_YGGDRASIL_TESTS=ON -DRAPIDJSON_BUILD_UBSAN=ON -DRAPIDJSON_BUILD_ASAN=ON
# cmake --build . -- -j 8
cmake --build . --target=tests -- -j 8
# ctest -C Debug --output-on-failure --verbose --stop-on-failure
ctest -R unittest --stop-on-failure
# ctest -R coverage
# ctest -R coverage
# cmake .. -DRAPIDJSON_SKIP_VALGRIND_TESTS=ON -DRAPIDJSON_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
# ctest -T Coverage
# ./bin/unittest
# export DATADIR=/Users/langmm/rapidjson/test
# export YGG_PYTHON_EXEC=/Users/langmm/miniconda3/envs/conda37/bin/python
# valgrind --leak-check=full   --show-leak-kinds=all --dsymutil=no --track-origins=yes -v --suppressions=/Users/langmm/valgrind-macos/darwin13.supp ./bin/unittest &> log.txt
# --suppressions=/Users/langmm/valgrind-macos/default.supp ./bin/unittest &> log.txt
