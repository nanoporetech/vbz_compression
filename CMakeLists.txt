cmake_minimum_required(VERSION 3.11.0)

execute_process(
  COMMAND git describe --tags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "v?([0-9.]+)-?.*" _dummy "${GIT_VERSION}")
set(GIT_VERSION "${CMAKE_MATCH_1}")

message(STATUS "Building version ${GIT_VERSION}")
project(hdf_plugins
    VERSION ${GIT_VERSION}
    LANGUAGES CXX
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(ExternalProject)
include(GenerateExportHeader)
include(CTest)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    message(STATUS "No CMAKE_BUILD_TYPE set - defaulting to Debug")
    # Default to Debug build
    get_property(_bt_docstring
        CACHE CMAKE_BUILD_TYPE
        PROPERTY HELPSTRING)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "${_bt_docstring}" FORCE)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(ENABLE_CONAN "Enable conan for dependency installation" ON)
if (ENABLE_CONAN)
    #
    # Dependencies: Conan
    #

    if (CONAN_EXPORTED)
        # we're being run by conan
        include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    else()
        include(conan)

        set(_extra_args)

        if (CMAKE_BUILD_TYPE)
            set(_build_type_DEBUG Debug)
            set(_build_type_RELEASE Release)
            set(_build_type_RELWITHDEBINFO Release)
            set(_build_type_MINSIZEREL Release)
            string(TOUPPER "${CMAKE_BUILD_TYPE}" _build_type)
            if(DEFINED _build_type_${_build_type})
                list(APPEND _extra_args BUILD_TYPE "${_build_type_${_build_type}}")
            endif()
        endif()

        conan_cmake_run(CONANFILE conanfile.txt ${_extra_args})
    endif()
    # Don't use conan_basic_setup() - half of what it does conflicts with or duplicates ONTSettings.
    # TODO: revisit this decision: it could be good to force the flags to match how depenencies were
    # built. Might also be useful for anything using MinKNOW-Core code (eg: VolTRAX).
    conan_check_compiler()
    #conan_set_find_library_paths()
    conan_define_targets()
    conan_set_find_paths()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Linking conan debug libraries")
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CONAN_CMAKE_MODULE_PATH_DEBUG})
    else()
        message(STATUS "Linking conan release libraries")
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CONAN_CMAKE_MODULE_PATH_RELEASE})
    endif()
endif()

include(packaging/hdf_plugin_packaging.cmake)

include_directories("${CMAKE_SOURCE_DIR}/third_party")

find_package(HDF5 1.8.16)
find_package(zstd 1.3.4 REQUIRED)

get_filename_component(STREAMVBYTE_SOURCE_DIR
    third_party/streamvbyte
    ABSOLUTE
)
set(STREAMVBYTE_INSTALL_DIR "${CMAKE_BINARY_DIR}/streamvbyte_lib/")
ExternalProject_Add(
    streamvbyte
    SOURCE_DIR ${STREAMVBYTE_SOURCE_DIR}
    UPDATE_DISCONNECTED TRUE
    PREFIX ${CMAKE_BINARY_DIR}/streamvbyte
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${STREAMVBYTE_INSTALL_DIR}
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(vbz)
add_subdirectory(vbz_plugin)