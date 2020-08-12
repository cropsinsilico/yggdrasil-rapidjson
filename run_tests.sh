set -e
cmake --build . --target tests
ctest -R unittest
# ./bin/unittest
# valgrind --leak-check=full   --show-leak-kinds=all --dsymutil=no --track-origins=yes -v --suppressions=/Users/langmm/valgrind-macos/darwin13.supp ./bin/unittest &> log.txt
# --suppressions=/Users/langmm/valgrind-macos/default.supp ./bin/unittest &> log.txt
