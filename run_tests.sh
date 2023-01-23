set -e
# mkdir build
# cd build
cmake .. -DRAPIDJSON_SKIP_VALGRIND_TESTS=ON -DRAPIDJSON_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug -DRAPIDJSON_CREATE_METASCHEMA_FULL=ON -DRAPIDJSON_YGGDRASIL_TESTS=ON -DRAPIDJSON_BUILD_UBSAN=ON -DRAPIDJSON_BUILD_ASAN=ON
# cmake --build .
cmake --build . --target=tests
# ctest -C Debug --output-on-failure --verbose
ctest -R unittest
# ctest -R coverage
# cmake .. -DRAPIDJSON_SKIP_VALGRIND_TESTS=ON -DRAPIDJSON_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
# ctest -T Coverage
# ./bin/unittest
# export DATADIR=/Users/langmm/rapidjson/test
# export YGG_PYTHON_EXEC=/Users/langmm/miniconda3/envs/conda37/bin/python
# valgrind --leak-check=full   --show-leak-kinds=all --dsymutil=no --track-origins=yes -v --suppressions=/Users/langmm/valgrind-macos/darwin13.supp ./bin/unittest &> log.txt
# --suppressions=/Users/langmm/valgrind-macos/default.supp ./bin/unittest &> log.txt
