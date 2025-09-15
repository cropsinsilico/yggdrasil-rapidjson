
SET(GTEST_SEARCH_PATH
    "${GTEST_SOURCE_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/gtest/googletest")

IF(UNIX)
    IF(RAPIDJSON_BUILD_THIRDPARTY_GTEST)
        LIST(APPEND GTEST_SEARCH_PATH "/usr/src/gtest")
    ELSE()
        LIST(INSERT GTEST_SEARCH_PATH 1 "/usr/src/gtest")
    ENDIF()
ENDIF()

FIND_PATH(GTEST_SOURCE_DIR
    NAMES src/gtest_main.cc
    PATHS ${GTEST_SEARCH_PATH})

string(FIND ${GTEST_SOURCE_DIR} "thirdparty" IDX_THIRDPARTY)
set(GTEST_THIRDPARTY OFF)
if(NOT IDX_THIRDPARTY EQUAL -1)
  set(GTEST_THIRDPARTY ON)
endif()
if(GTEST_THIRDPARTY)
  execute_process(
    COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/gtest.patch
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/gtest"
    RESULT_VARIABLE GTEST_THIRDPARTY_PATCH_RESULT
  )
endif()


# Debian installs gtest include directory in /usr/include, thus need to look
# for include directory separately from source directory.
FIND_PATH(GTEST_INCLUDE_DIR
    NAMES gtest/gtest.h
    PATH_SUFFIXES include
    HINTS ${GTEST_SOURCE_DIR}
    PATHS ${GTEST_SEARCH_PATH})

INCLUDE(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GTestSrc DEFAULT_MSG
    GTEST_SOURCE_DIR
    GTEST_INCLUDE_DIR)
message(STATUS "GTEST_SOURCE_DIR = ${GTEST_SOURCE_DIR}")
message(STATUS "GTEST_INCLUDE_DIR = ${GTEST_INCLUDE_DIR}")