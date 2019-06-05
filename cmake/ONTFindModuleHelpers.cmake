#.rst:
# ONTFindModuleHelpers
# --------------------
#
# Based on ECMFindModuleHelpers from extra-cmake-modules.

include(CMakeParseArguments)

macro(ont_find_package_parse_components module_name)
    set(_options SKIP_DEPENDENCY_HANDLING)
    set(_oneValueArgs RESULT_VAR)
    set(_multiValueArgs KNOWN_COMPONENTS)
    cmake_parse_arguments(_ECM_FPPC "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})

    if(_ECM_FPPC_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments to ecm_find_package_parse_components: ${_ECM_FPPC_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT _ECM_FPPC_RESULT_VAR)
        message(FATAL_ERROR "Missing RESULT_VAR argument to ecm_find_package_parse_components")
    endif()
    if(NOT _ECM_FPPC_KNOWN_COMPONENTS)
        message(FATAL_ERROR "Missing KNOWN_COMPONENTS argument to ecm_find_package_parse_components")
    endif()

    if(${module_name}_FIND_COMPONENTS)
        set(_requestedComps ${${module_name}_FIND_COMPONENTS})

        if(NOT _ECM_FPPC_SKIP_DEPENDENCY_HANDLING)
            # Make sure deps are included
            set(_new_comps ${_requestedComps})
            while(_new_comps)
                set(_working_comps ${_new_comps})
                set(_new_comps)
                foreach(_comp ${_working_comps})
                    foreach(_dep_comp ${${module_name}_${_comp}_component_deps})
                        list(FIND _requestedComps "${_dep_comp}" _index)
                        if("${_index}" STREQUAL "-1")
                            if(NOT ${module_name}_FIND_QUIETLY)
                                message(STATUS "${module_name}: ${_comp} requires ${_dep_comp}")
                            endif()
                            list(APPEND _new_comps "${_dep_comp}")
                            list(APPEND _requestedComps "${_dep_comp}")
                        endif()
                    endforeach()
                endforeach()
            endwhile()
        else()
            message(STATUS "Skipping dependency handling for ${module_name}")
        endif()
        list(REMOVE_DUPLICATES _requestedComps)

        # This makes sure components are listed in the same order as
        # KNOWN_COMPONENTS (potentially important for inter-dependencies)
        set(${_ECM_FPPC_RESULT_VAR})
        foreach(_comp ${_ECM_FPPC_KNOWN_COMPONENTS})
            list(FIND _requestedComps "${_comp}" _index)
            if(NOT "${_index}" STREQUAL "-1")
                list(APPEND ${_ECM_FPPC_RESULT_VAR} "${_comp}")
                list(REMOVE_AT _requestedComps ${_index})
            endif()
        endforeach()
        # if there are any left, they are unknown components
        if(_requestedComps)
            set(_msgType STATUS)
            if(${module_name}_FIND_REQUIRED)
                set(_msgType FATAL_ERROR)
            endif()
            if(NOT ${module_name}_FIND_QUIETLY)
                message(${_msgType} "${module_name}: requested unknown components ${_requestedComps}")
            endif()
            return()
        endif()
    else()
        set(${_ECM_FPPC_RESULT_VAR} ${_ECM_FPPC_KNOWN_COMPONENTS})
    endif()
endmacro()

macro(ont_debug_lib_helper outvar debugvar releasevar)
    if (${releasevar} AND ${debugvar})
        if (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
            set(${outvar}
                optimized ${${releasevar}}
                debug ${${debugvar}}
            )
        else()
            set(${outvar} ${${releasevar}})
        endif()
    elseif (${releasevar} AND NOT ${debugvar})
        set(${outvar} ${${releasevar}})
    elseif (${debugvar} AND NOT ${releasevar})
        set(${outvar} ${${debugvar}})
    endif()
endmacro()

function(ont_create_library_target target)
    if(TARGET ${target})
        return()
    endif()

    set(options)
    set(oneValueArgs DEBUG_LIBRARY RELEASE_LIBRARY LIBRARY)
    set(multiValueArgs DEPENDENCIES DEBUG_DEPENDENCIES RELEASE_DEPENDENCIES COMPILE_DEFINITIONS COMPILE_OPTIONS INCLUDE_DIRS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments to ont_create_library_target: ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if (ARG_LIBRARY)
        if (ARG_DEBUG_LIBRARY OR ARG_RELEASE_LIBRARY)
            message(FATAL_ERROR "Cannot specify LIBRARY as well as DEBUG_LIBRARY or RELEASE_LIBRARY")
        endif()
    elseif (NOT ARG_DEBUG_LIBRARY AND NOT ARG_RELEASE_LIBRARY)
        message(FATAL_ERROR "Must specify LIBRARY, DEBUG_LIBRARY or RELEASE_LIBRARY")
    endif()

    add_library(${target} UNKNOWN IMPORTED)
    if (ARG_LIBRARY)
        set_property(TARGET ${target}
            PROPERTY
            IMPORTED_LOCATION "${ARG_LIBRARY}"
        )
    else()
        if (ARG_RELEASE_LIBRARY)
            set_property(TARGET ${target}
                APPEND PROPERTY
                IMPORTED_CONFIGURATIONS RELEASE
            )
            set_property(TARGET ${target}
                PROPERTY
                IMPORTED_LOCATION_RELEASE "${ARG_RELEASE_LIBRARY}"
            )
        endif()
        if (ARG_DEBUG_LIBRARY)
            set_property(TARGET ${target}
                APPEND PROPERTY
                IMPORTED_CONFIGURATIONS DEBUG
            )
            set_property(TARGET ${target}
                PROPERTY
                IMPORTED_LOCATION_DEBUG "${ARG_DEBUG_LIBRARY}"
            )
        endif()
        if (ARG_DEBUG_DEPENDENCIES)
            list(APPEND ARG_DEPENDENCIES "$<$<CONFIG:Debug>:${ARG_DEBUG_DEPENDENCIES}>")
        endif()
        if (ARG_RELEASE_DEPENDENCIES)
            list(APPEND ARG_DEPENDENCIES "$<$<NOT:$<CONFIG:Debug>>:${ARG_RELEASE_DEPENDENCIES}>")
        endif()
    endif()
    set_target_properties(${target} PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS "${ARG_COMPILE_DEFINITIONS}"
        INTERFACE_COMPILE_OPTIONS "${ARG_COMPILE_OPTIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${ARG_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${ARG_DEPENDENCIES}"
    )
endfunction()

macro(ont_find_package_handle_library_components module_name)
    set(_options SKIP_PKG_CONFIG SKIP_DEPENDENCY_HANDLING DEBUG_AND_RELEASE)
    set(_oneValueArgs)
    set(_multiValueArgs COMPONENTS)
    cmake_parse_arguments(_ONT_FPWC "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})

    if(_ONT_FPWC_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments to ecm_find_package_handle_components: ${_ONT_FPWC_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT _ONT_FPWC_COMPONENTS)
        message(FATAL_ERROR "Missing COMPONENTS argument to ecm_find_package_handle_components")
    endif()

    include(FindPackageHandleStandardArgs)
    if(NOT _ONT_FPWC_SKIP_PKG_CONFIG)
        find_package(PkgConfig QUIET)
    endif()
    foreach(_comp ${_ONT_FPWC_COMPONENTS})
        set(_dep_vars)
        set(_dep_targets)
        if(NOT SKIP_DEPENDENCY_HANDLING)
            foreach(_dep ${${module_name}_${_comp}_component_deps})
                list(APPEND _dep_vars "${module_name}_${_dep}_FOUND")
                list(APPEND _dep_targets "${module_name}::${_dep}")
            endforeach()
        endif()

        if(NOT _ONT_FPWC_SKIP_PKG_CONFIG AND ${module_name}_${_comp}_pkg_config)
            pkg_check_modules(PKG_${module_name}_${_comp} QUIET
                              ${${module_name}_${_comp}_pkg_config})
        endif()

        set(_include_var)
        if (${module_name}_${_comp}_header_names)
            set(_include_var ${module_name}_${_comp}_INCLUDE_DIR)
            find_path(${_include_var}
                NAMES ${${module_name}_${_comp}_header_names}
                HINTS ${PKG_${module_name}_${_comp}_INCLUDE_DIRS}
                PATH_SUFFIXES ${${module_name}_${_comp}_header_path_suffixes}
            )
        endif()
        if (_ONT_FPWC_DEBUG_AND_RELEASE)
            find_library(${module_name}_${_comp}_LIBRARY_RELEASE
                NAMES ${${module_name}_${_comp}_lib_names_release}
                HINTS ${PKG_${module_name}_${_comp}_LIBRARY_DIRS}
                DOC "The release version of the ${_comp} library from ${module_name}"
            )
            find_library(${module_name}_${_comp}_LIBRARY_DEBUG
                NAMES ${${module_name}_${_comp}_lib_names_debug}
                HINTS ${PKG_${module_name}_${_comp}_LIBRARY_DIRS}
                DOC "The debug version of the ${_comp} library from ${module_name}"
            )
            ont_debug_lib_helper(
                ${module_name}_${_comp}_LIBRARY
                ${module_name}_${_comp}_LIBRARY_DEBUG
                ${module_name}_${_comp}_LIBRARY_RELEASE
            )
            set(${module_name}_${_comp}_LIBRARY
                "${${module_name}_${_comp}_LIBRARY}"
                CACHE STRING "The ${_comp} library from ${module_name}" FORCE
            )
        else()
            find_library(${module_name}_${_comp}_LIBRARY
                NAMES ${${module_name}_${_comp}_lib_names}
                HINTS ${PKG_${module_name}_${_comp}_LIBRARY_DIRS}
                DOC "The ${_comp} library from ${module_name}"
            )
        endif()

        set(${module_name}_${_comp}_VERSION "${PKG_${module_name}_${_comp}_VERSION}")
        if(NOT ${module_name}_VERSION)
            set(${module_name}_VERSION ${${module_name}_${_comp}_VERSION})
        endif()

        find_package_handle_standard_args(${module_name}_${_comp}
            FOUND_VAR
                ${module_name}_${_comp}_FOUND
            REQUIRED_VARS
                ${module_name}_${_comp}_LIBRARY
                ${_include_var}
                ${_dep_vars}
            VERSION_VAR
                ${module_name}_${_comp}_VERSION
            )

        mark_as_advanced(
            ${module_name}_${_comp}_LIBRARY
            ${module_name}_${_comp}_INCLUDE_DIR
        )

        if(${module_name}_${_comp}_FOUND)
            set(${module_name}_${_comp}_DEFINITIONS
                ${PKG_${module_name}_${_comp}_DEFINITIONS}
                ${${module_name}_${_comp}_compile_opts})
            ont_create_library_target(${module_name}::${_comp}
                DEBUG_LIBRARY "${${module_name}_${_comp}_LIBRARY_DEBUG}"
                RELEASE_LIBRARY "${${module_name}_${_comp}_LIBRARY_RELEASE}"
                DEPENDENCIES "${_dep_targets}"
                COMPILE_OPTIONS "${${module_name}_${_comp}_DEFINITIONS}"
                INCLUDE_DIRS "${${module_name}_${_comp}_INCLUDE_DIR}"
            )
            if (NOT ${module_name}_${_comp}_LIBRARY_DEBUG)
                set(${module_name}_${_comp}_LIBRARY_DEBUG
                    ${module_name}_${_comp}_LIBRARY_RELEASE)
            elseif (NOT ${module_name}_${_comp}_LIBRARY_RELEASE)
                set(${module_name}_${_comp}_LIBRARY_RELEASE
                    ${module_name}_${_comp}_LIBRARY_DEBUG)
            endif()
            list(APPEND
                ${module_name}_LIBRARIES
                "${${module_name}_${_comp}_LIBRARY}")
            if (_ONT_FPWC_DEBUG_AND_RELEASE)
                if (${module_name}_${_comp}_LIBRARY_RELEASE)
                    list(APPEND
                        ${module_name}_LIBRARIES_RELEASE
                        "${${module_name}_${_comp}_LIBRARY_RELEASE}")
                endif()
                if (${module_name}_${_comp}_LIBRARY_DEBUG)
                    list(APPEND
                        ${module_name}_LIBRARIES_DEBUG
                        "${${module_name}_${_comp}_LIBRARY_DEBUG}")
                endif()
            endif()
            if (${_include_var})
                list(APPEND
                    ${module_name}_INCLUDE_DIRS
                    "${${_include_var}}")
            endif()
            set(${module_name}_DEFINITIONS
                    ${${module_name}_DEFINITIONS}
                    ${${module_name}_${_comp}_DEFINITIONS})
            list(APPEND ${module_name}_TARGETS
                        "${module_name}::${_comp}")
        endif()
    endforeach()
    if(${module_name}_DEBUG_LIBRARIES)
        list(REMOVE_DUPLICATES ${module_name}_DEBUG_LIBRARIES)
    endif()
    if(${module_name}_INCLUDE_DIRS)
        list(REMOVE_DUPLICATES ${module_name}_INCLUDE_DIRS)
    endif()
    if(${module_name}_DEFINITIONS)
        list(REMOVE_DUPLICATES ${module_name}_DEFINITIONS)
    endif()
    if(${module_name}_TARGETS)
        list(REMOVE_DUPLICATES ${module_name}_TARGETS)
    endif()
endmacro()


