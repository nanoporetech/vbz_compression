include(ONTFindModuleHelpers)

set(HDF5_known_components
    Z
    C
    CXX
    HL
    HL_CXX
    Tools
)
foreach(_comp ${HDF5_known_components})
    set(HDF5_${_comp}_compile_opts "-DH5_BUILT_AS_DYNAMIC_LIB=1")
endforeach()

set(HDF5_Z_compile_opts "")
set(HDF5_Z_lib_names_debug "zlibd;zd")
set(HDF5_Z_lib_names_release "zlib;z")
set(HDF5_Z_component_deps)

set(HDF5_C_lib_names_release "hdf5")
set(HDF5_C_header_names "hdf5.h")
set(HDF5_C_component_deps Z)

set(HDF5_CXX_lib_names_release "hdf5_cpp")
set(HDF5_CXX_header_names "H5Cpp.h")
set(HDF5_CXX_component_deps C)

set(HDF5_HL_lib_names_release "hdf5_hl")
set(HDF5_HL_header_names "hdf5_hl.h")
set(HDF5_HL_component_deps C)

set(HDF5_HL_CXX_lib_names_release "hdf5_hl_cpp")
set(HDF5_HL_CXX_header_names "H5PacketTable.h")
set(HDF5_HL_CXX_component_deps HL CXX)

set(HDF5_Tools_lib_names_release "hdf5_tools")
set(HDF5_Tools_header_names "h5tools.h")
set(HDF5_Tools_component_deps C)

foreach(_comp ${HDF5_known_components})
    if(NOT _comp STREQUAL "Z")
        set(HDF5_${_comp}_lib_names_debug)
        foreach(_name ${HDF5_${_comp}_lib_names_release})
            list(APPEND HDF5_${_comp}_lib_names_debug "${_name}_D")
        endforeach()
    endif()
endforeach()

ont_find_package_parse_components(HDF5
    RESULT_VAR HDF5_components
    KNOWN_COMPONENTS ${HDF5_known_components}
)
ont_find_package_handle_library_components(HDF5
    COMPONENTS ${HDF5_components}
    DEBUG_AND_RELEASE
    SKIP_PKG_CONFIG
)

find_path(HDF5_INCLUDE_DIR
    NAMES H5public.h
    HINTS ${PKG_HDF5_C_INCLUDE_DIRS} 
)
mark_as_advanced(HDF5_INCLUDE_DIR)
if(HDF5_INCLUDE_DIR AND EXISTS "${HDF5_INCLUDE_DIR}/H5public.h")
    set(HDF5_PUBLIC_HEADER "${HDF5_INCLUDE_DIR}/H5public.h")
    file(STRINGS ${HDF5_PUBLIC_HEADER} _hdf5_version_lines
        REGEX "^#define[ \t]+H5_VERS")
    string(REGEX MATCH "H5_VERS_MAJOR[\t ]+([0-9]+)" _dummy ${_hdf5_version_lines})
    set(_hdf5_version_major "${CMAKE_MATCH_1}")
    string(REGEX MATCH "H5_VERS_MINOR[\t ]+([0-9]+)" _dummy ${_hdf5_version_lines})
    set(_hdf5_version_minor "${CMAKE_MATCH_1}")
    string(REGEX MATCH "H5_VERS_RELEASE[\t ]+([0-9]+)" _dummy ${_hdf5_version_lines})
    set(_hdf5_version_release "${CMAKE_MATCH_1}")
    unset(_hdf5_version_lines)
    set(HDF5_VERSION "${_hdf5_version_major}.${_hdf5_version_minor}.${_hdf5_version_release}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HDF5
    FOUND_VAR
        HDF5_FOUND
    REQUIRED_VARS
        HDF5_LIBRARIES
        HDF5_INCLUDE_DIR
    VERSION_VAR
        HDF5_VERSION
    HANDLE_COMPONENTS
)
