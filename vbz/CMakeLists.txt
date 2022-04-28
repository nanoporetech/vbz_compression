
add_library(vbz STATIC
    v0/vbz_streamvbyte.h
    v0/vbz_streamvbyte.cpp
    v0/vbz_streamvbyte_impl.h
    v0/vbz_streamvbyte_impl_sse3.h

    v1/vbz_streamvbyte.h
    v1/vbz_streamvbyte.cpp
    v0/vbz_streamvbyte_impl.h

    vbz.h
    vbz.cpp
)
add_sanitizers(vbz)

generate_export_header(vbz EXPORT_FILE_NAME vbz/vbz_export.h)

set_property(TARGET vbz PROPERTY CXX_STANDARD 11)

target_include_directories(vbz
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${STREAMVBYTE_INSTALL_DIR}/include
)

# This explicit dependency is required to make sure the header files are installed into the right
# location before we try building against them.
add_dependencies(vbz
    streamvbyte
)

if (WIN32 OR 
    (APPLE AND NOT "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm64") OR
    "${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64")
    if(NOT MSVC)
        target_compile_options(vbz PRIVATE -mssse3)
    endif()
endif()

target_link_libraries(vbz
    PUBLIC
        ${STREAMVBYTE_STATIC_LIB}
        zstd::zstd
)

if (BUILD_TESTING)
    add_subdirectory(fuzzing)
    add_subdirectory(test)

    if (ENABLE_PERF_TESTING)
        add_subdirectory(perf)
    endif()
endif()
