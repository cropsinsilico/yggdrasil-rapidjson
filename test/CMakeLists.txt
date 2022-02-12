# include(CTest)
enable_testing()
find_package(GTestSrc)
# find_package(GTest)

add_definitions(-DRAPIDJSON_YGGDRASIL)
add_definitions(-DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION)
add_definitions(-D_USE_MATH_DEFINES)

# include_directories(SYSTEM ${PYTHON_INCLUDE_DIRS} ${NUMPY_INCLUDE_DIR})
include_directories(SYSTEM ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})

# if(GTEST_FOUND)

#     include_directories(SYSTEM ${GTEST_INCLUDE_DIR})
##     set(TEST_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} ${PYTHON_LIBRARIES})
#     set(TEST_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} ${Python3_LIBRARIES})
    
if(GTESTSRC_FOUND)

    if (WIN32 AND (NOT CYGWIN) AND (NOT MINGW))
        set(gtest_disable_pthreads ON)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    IF(GTEST_SOURCE_DIR)
        add_subdirectory(${GTEST_SOURCE_DIR} ${CMAKE_BINARY_DIR}/googletest)
    endif()
    include_directories(SYSTEM ${GTEST_INCLUDE_DIR})

    # set(TEST_LIBRARIES gtest gtest_main ${PYTHON_LIBRARIES})
    set(TEST_LIBRARIES gtest gtest_main ${Python3_LIBRARIES})

endif()

if(TEST_LIBRARIES)
    option(RAPIDJSON_ENABLE_COVERAGE "Enable coverage reporting" OFF)
    if(RAPIDJSON_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang|ARMClang")
        set(CMAKE_CXX_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage -fprofile-filter-files=\"${CMAKE_SOURCE_DIR}/include/*\" -fprofile-exclude-files=\"${CMAKE_SOURCE_DIR}/include/rapidjson/fwd.h\"")
        set(CMAKE_C_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage -fprofile-filter-files=\"${CMAKE_SOURCE_DIR}/include/*\" -fprofile-exclude-files=\"${CMAKE_SOURCE_DIR}/include/rapidjson/fwd.h\"")
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|ARMClang")
            set(CTEST_COVERAGE_COMMAND "llvm-cov")
            set(CTEST_COVERAGE_EXTRA_FLAGS "gcov")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
        endif()
    endif(RAPIDJSON_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang|ARMClang")

    add_custom_target(tests ALL)
    add_subdirectory(perftest)
    add_subdirectory(unittest)
    
endif()
