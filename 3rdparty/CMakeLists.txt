include(FetchContent)

FetchContent_Declare(
        tinyxml2
        GIT_REPOSITORY https://github.com/andiwand/tinyxml2.git
        GIT_TAG f4e1f46
)
FetchContent_Declare(
        miniz
        GIT_REPOSITORY https://github.com/richgel999/miniz.git
        GIT_TAG cd99aff
)
FetchContent_Declare(
        glog
        GIT_REPOSITORY https://github.com/google/glog.git
        GIT_TAG 1b7d541
)
FetchContent_Declare(
        cryptopp
        GIT_REPOSITORY https://github.com/TomTasche/cryptopp.git
        GIT_TAG 6b1dd5f
)
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.7.3
)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.8.1
)

# cryptopp: fixes "phony targets" duplicate error during Android build
# also disables building unit tests for glog and tinyxml
option(BUILD_TESTING "" OFF)
set(BUILD_TESTING OFF)
# miniz
option(BUILD_EXAMPLES "" OFF)
set(BUILD_EXAMPLES OFF)
# tinyxml2
option(BUILD_SHARED_LIBS "" OFF)
set(BUILD_SHARED_LIBS OFF)
option(BUILD_TESTS "" OFF)
set(BUILD_TESTS OFF)

FetchContent_GetProperties(tinyxml2)
if (NOT tinyxml2_POPULATED)
    FetchContent_Populate(tinyxml2)
    add_subdirectory(${tinyxml2_SOURCE_DIR} ${tinyxml2_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()
FetchContent_GetProperties(miniz)
if (NOT miniz_POPULATED)
    FetchContent_Populate(miniz)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${miniz_SOURCE_DIR} ${miniz_BINARY_DIR} EXCLUDE_FROM_ALL)
    # miniz defines a lot of macros without that
    target_compile_definitions(miniz PUBLIC MINIZ_NO_ZLIB_COMPATIBLE_NAMES)
endif ()
FetchContent_GetProperties(glog)
if (NOT glog_POPULATED)
    FetchContent_Populate(glog)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    add_subdirectory(${glog_SOURCE_DIR} ${glog_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()
FetchContent_GetProperties(cryptopp)
if (NOT cryptopp_POPULATED)
    FetchContent_Populate(cryptopp)
    add_subdirectory(${cryptopp_SOURCE_DIR} ${cryptopp_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()
FetchContent_GetProperties(json)
if (NOT json_POPULATED)
    FetchContent_Populate(json)
    set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()
FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(gtest_build_tests OFF CACHE BOOL "" FORCE)
    set(gtest_build_samples OFF CACHE BOOL "" FORCE)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()