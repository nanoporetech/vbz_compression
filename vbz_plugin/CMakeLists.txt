
add_library(vbz_hdf_plugin SHARED
    vbz_plugin_user_utils.h
    vbz_plugin.cpp
    vbz_plugin.h
)
add_sanitizers(vbz_hdf_plugin)

generate_export_header(vbz_hdf_plugin EXPORT_FILE_NAME vbz_plugin/vbz_hdf_plugin_export.h)

set_property(TARGET vbz_hdf_plugin PROPERTY CXX_STANDARD 11)

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

install(
    TARGETS vbz_hdf_plugin
    DESTINATION "hdf5/lib/plugin"
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
