cmake_minimum_required(VERSION 3.11.0)

execute_process(
  COMMAND git describe --tags
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "v?([0-9.]+)-?.*" _dummy "${GIT_VERSION}")
set(GIT_VERSION "${CMAKE_MATCH_1}")

message(STATUS "Building version ${GIT_VERSION}")
project(hdf_plugins
    VERSION ${GIT_VERSION}
    LANGUAGES CXX C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(ExternalProject)
include(GenerateExportHeader)
# If we're including hdf_plugins as a subdir then we might not be able
# to override BUILD_TESTING from CTest's default ON value.
if (NOT DEFINED BUILD_TESTING OR BUILD_TESTING)
    include(CTest)
endif()

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    message(STATUS "No CMAKE_BUILD_TYPE set - defaulting to Debug")
    # Default to Debug build
    get_property(_bt_docstring
        CACHE CMAKE_BUILD_TYPE
        PROPERTY HELPSTRING)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "${_bt_docstring}" FORCE)
endif()

set(HDF_PLUGIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if (NOT DEFINED VBZ_BUILD_ARCHIVE)
    option(VBZ_BUILD_ARCHIVE "Build vbz release as an single archive" OFF)
endif()

if (NOT DEFINED ENABLE_PYTHON)
    option(ENABLE_PYTHON "Build python wheel" ON)
endif()

if (NOT DEFINED ENABLE_PACKAGING)
    option(ENABLE_PACKAGING "Enable packaging support" ON)
endif()

if (NOT DEFINED ENABLE_PERF_TESTING)
    option(ENABLE_PERF_TESTING "Enable performance tests" ON)
endif()

if (NOT DEFINED ENABLE_CONAN)
    option(ENABLE_CONAN "Enable conan for dependency installation" ON)
endif()

if (NOT DEFINED STANDARD_LIB_INSTALL)
    set(STD_LIB_INSTALL_DEFAULT OFF)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_DIR)
        set(STD_LIB_INSTALL_DEFAULT ON)
    endif()
    option(STANDARD_LIB_INSTALL
        "Install library to standard lib / bin paths"
        ${STD_LIB_INSTALL_DEFAULT}
        )
endif()

if (NOT VBZ_BUILD_ARCHIVE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if (ENABLE_CONAN)
    #
    # Dependencies: Conan
    #

    if (CONAN_EXPORTED)
        # we're being run by conan
        include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    else()
        include(conan)

        set(_requirements
            "zstd/1.3.4@nanopore/stable")
        set(_generators
            cmake
            json)
        set(_imports
            "bin, *.dll -> ../bin"
            "lib, *.dylib* -> ../bin")
        set(_extra_args)

        if (ENABLE_PERF_TESTING)
            list(APPEND _requirements
                "google-benchmark/1.5@nanopore/stable"
                )
        endif()
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
        if (CONAN_EXTRA_SETTINGS)
            list(APPEND _extra_args
                SETTINGS
                ${CONAN_EXTRA_SETTINGS}
                )
        endif()

        conan_cmake_run(
            GENERATORS ${_generators}
            REQUIRES ${_requirements}
            IMPORTS ${_imports}
            ${_extra_args})
    endif()
    conan_check_compiler()
    conan_define_targets()
    conan_set_find_paths()
endif()

if (ENABLE_PACKAGING)
    include(packaging/hdf_plugin_packaging.cmake)
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party")

find_package(HDF5 1.8.16)
find_package(zstd 1.3.1 REQUIRED)
find_package(Sanitizers)

get_filename_component(STREAMVBYTE_SOURCE_DIR
    third_party/streamvbyte
    ABSOLUTE
)
set(STREAMVBYTE_INSTALL_DIR "${CMAKE_BINARY_DIR}/streamvbyte_lib/")
set(STREAMVBYTE_PREFIX ${CMAKE_BINARY_DIR}/streamvbyte)
set(STREAMVBYTE_STATIC_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}streamvbyte_static${CMAKE_STATIC_LIBRARY_SUFFIX})
set(STREAMVBYTE_STATIC_LIB_SUBDIR ".")
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (is_multi_config)
    set(STREAMVBYTE_STATIC_LIB_SUBDIR "$<CONFIG>")
endif()
ExternalProject_Add(
    streamvbyte
    SOURCE_DIR ${STREAMVBYTE_SOURCE_DIR}
    UPDATE_DISCONNECTED TRUE
    PREFIX ${STREAMVBYTE_PREFIX}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${STREAMVBYTE_INSTALL_DIR} -DSTREAMVBYTE_DISABLE_NATIVE=ON -DSTD_FLAGS="-std=c99"
    # We need to tell CMake about libraries we will link to, otherwise some generators (eg: Ninja)
    # complain about how knowing how to build libstreamvbyte_static.a, for example.
    # Ideally, we'd set a hypothetical INSTALL_BYPRODUCTS, and link to the installed file, but that
    # doesn't seem to be an option.
    BUILD_BYPRODUCTS "<BINARY_DIR>/${STREAMVBYTE_STATIC_LIB_NAME}"
)
ExternalProject_Get_Property(streamvbyte BINARY_DIR)
set(STREAMVBYTE_STATIC_LIB "${BINARY_DIR}/${STREAMVBYTE_STATIC_LIB_SUBDIR}/${STREAMVBYTE_STATIC_LIB_NAME}")


########################################################################
#
# Warnings
#
########################################################################

if (MSVC)
    add_compile_options(
        # Level 3 warnings, as errors
        /W3 /WX

        ##
        ## Disable warnings:
        ##

        # C4251: A base class or structure must be declared with the __declspec(dllexport) keyword
        #        for a function in a derived class to be exported.
        #
        # Happens when using STL types as members of a class tagged with WHATEVER_EXPORT. Since we
        # don't care about our DLLs maintaining compatibility with other versions of the CRT
        # (Microsoft's C Runtime Library), this is just noise.
        /wd4251

        # C4275: An exported class was derived from a class that was not exported.
        #
        # Very similar to C4251, but for inheritance rather than members. Again, just noise for us.
        /wd4275

        # C4373: '%$S': virtual function overrides '%$pS', previous versions of the compiler did not
        #        override when parameters only differed by const/volatile qualifiers
        #
        # The current behaviour (virtuals are overridden when their arguments differ only in
        # constness) is what you'd expect - the warning only exists because VS 2008 and earlier had
        # *different* behaviour. Since we never used VS 2008, this warning isn't useful to us.
        /wd4373

        ##
        ## Enable additional warnings:
        ##

        # C5038: data member 'member1' will be initialized after data member 'member2'
        #
        # Enable warning about incorrect order of initialisation in classes.
        # Matches clang warnings which are enabled on the build server.
        /w35038
        )
    set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
        # MSVC's standard library warns about a lot of "insecure" code by default (eg: unchecked
        # iterators). However, the recommended replacements are not cross-platform, so aren't really
        # an option for us.
        _SCL_SECURE_NO_WARNINGS
        )
elseif (CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(
        # "All" warnings as errors
        -Wall -Wextra -Werror

        # Unused parameters are too common (and unimportant) to warn about
        -Wno-unused-parameter
        # Missing field init warning is too general to be useful in C++ code
        -Wno-missing-field-initializers
        # boost::optional triggers this on release builds
        -Wno-maybe-uninitialized
        )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang") # AppleClang and Clang
    add_compile_options(
        # "All" warnings as errors
        -Wall -Werror
        )
endif()

include(SigningUtils)

if (ENABLE_PYTHON)
    add_subdirectory(python)
endif()
add_subdirectory(vbz)
add_subdirectory(vbz_plugin)
