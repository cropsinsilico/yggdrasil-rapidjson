set -e
# mkdir build
# cd build
cmake .. -DRAPIDJSON_SKIP_VALGRIND_TESTS=ON -DRAPIDJSON_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
# cmake --build .
cmake --build . --target=tests
# ctest -C Debug --output-on-failure --verbose
ctest -R unittest
ctest -R coverage
# cmake .. -DRAPIDJSON_SKIP_VALGRIND_TESTS=ON -DRAPIDJSON_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
# ctest -T Coverage
# ./bin/unittest
# valgrind --leak-check=full   --show-leak-kinds=all --dsymutil=no --track-origins=yes -v --suppressions=/Users/langmm/valgrind-macos/darwin13.supp ./bin/unittest &> log.txt
# --suppressions=/Users/langmm/valgrind-macos/default.supp ./bin/unittest &> log.txt
