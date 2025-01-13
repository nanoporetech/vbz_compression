
add_library(vbz_hdf_plugin
    vbz_plugin_user_utils.h
    vbz_plugin.cpp
    vbz_plugin.h
)
add_sanitizers(vbz_hdf_plugin)

generate_export_header(vbz_hdf_plugin EXPORT_FILE_NAME vbz_plugin/vbz_hdf_plugin_export.h)

target_compile_features(vbz_hdf_plugin PRIVATE cxx_std_17)

if(WIN32 AND HDF5_USE_STATIC_LIBRARIES)
    # See vbz_plugin.cpp -- if also building with Python on Windows then the runtime
    # used is determined by that Python version, so we only support dynamic builds.
    if(ENABLE_PYTHON)
        message(FATAL_ERROR "Cannot enable HDF5_USE_STATIC_LIBRARIES as well as ENABLE_PYTHON")
    else()
        add_definitions(-DHDF5_USE_STATIC_LIBRARIES)
    endif()
endif()

target_include_directories(
    vbz_hdf_plugin
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/
)

target_link_libraries(vbz_hdf_plugin
    PRIVATE
        vbz
)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel" AND NOT WIN32)
    target_link_libraries(vbz_hdf_plugin
        PRIVATE
        $<$<CXX_COMPILER_ID:Intel>:-static-intel>
        $<$<CXX_COMPILER_ID:Intel>:-mkl=sequential>
        $<$<CXX_COMPILER_ID:Intel>:-wd10237>
        )
endif()

if (ENABLE_CODE_SIGNING)
    hdf_add_signing_command(vbz_hdf_plugin)
endif()

if (WIN32)
    target_compile_definitions(vbz_hdf_plugin
        PRIVATE
            -D_CRT_SECURE_NO_WARNINGS
    )
endif()

if (BUILD_TESTING)
    if (HDF5_FOUND)
        add_subdirectory(hdf_test_utils)

        add_subdirectory(test)

        if (ENABLE_PERF_TESTING)
            add_subdirectory(perf)
        endif()
    else()
        message(STATUS "Not building hdf5 testing as hdf5 not found")
    endif()
endif()

if (NOT VBZ_BUILD_ARCHIVE)
    set(VBZ_PLUGIN_COMPONENT COMPONENT hdf_plugin)
endif()

set(INSTALL_DESTINATION "hdf5/lib/plugin")
if (STANDARD_LIB_INSTALL)
    set(VBZ_PLUGIN_COMPONENT COMPONENT vbz_hdf_plugin)
    set_target_properties(vbz_hdf_plugin
        PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        )
    if(WIN32)
        set(INSTALL_DESTINATION "bin")
    else()
        set(INSTALL_DESTINATION "lib")
    endif()
elseif (VBZ_BUILD_ARCHIVE)
    set(INSTALL_DESTINATION "bin")
endif()

install(
    TARGETS vbz_hdf_plugin
    DESTINATION ${INSTALL_DESTINATION}
    ${VBZ_PLUGIN_COMPONENT}
)

if (APPLE AND VBZ_CODESIGN_IDENTITY AND VBZ_CODESIGN_KEYCHAIN)
    add_custom_command(
        TARGET vbz_hdf_plugin
        POST_BUILD
            COMMAND codesign -s "${VBZ_CODESIGN_IDENTITY}" "${VBZ_CODESIGN_KEYCHAIN}" --force --deep -vvvv $<TARGET_FILE:vbz_hdf_plugin>
            COMMENT "Signing vbz_hdf_plugin with ${VBZ_CODESIGN_IDENTITY}"
    )
endif()
