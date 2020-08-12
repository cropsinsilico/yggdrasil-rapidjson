find_package(GTestSrc)

add_definitions(-DRAPIDJSON_YGGDRASIL)
add_definitions(-DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION)

IF(GTESTSRC_FOUND)
    enable_testing()

    if (WIN32 AND (NOT CYGWIN) AND (NOT MINGW))
        set(gtest_disable_pthreads ON)
        set(gtest_force_shared_crt ON)
    endif()

    IF(GTEST_SOURCE_DIR)
        add_subdirectory(${GTEST_SOURCE_DIR} ${CMAKE_BINARY_DIR}/googletest)
    endif()
    message("GTEST_INCLUDE_DIR = ${GTEST_INCLUDE_DIR}, GTEST_INCLUDE_DIRS = ${GTEST_INCLUDE_DIRS}, GTEST_LIBRARIES = ${GTEST_LIBRARIES}")
    include_directories(SYSTEM ${GTEST_INCLUDE_DIR})
    include_directories(SYSTEM ${PYTHON_INCLUDE_DIRS} ${NUMPY_INCLUDE_DIR})

    set(TEST_LIBRARIES gtest gtest_main ${PYTHON_LIBRARIES})

    add_custom_target(tests ALL)
    # add_subdirectory(perftest)
    add_subdirectory(unittest)

ENDIF(GTESTSRC_FOUND)
