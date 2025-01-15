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

target_compile_features(vbz PRIVATE cxx_std_17)

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

option(VBZ_DISABLE_SSE3 "Disable SSE3 optimisations" OFF)
if ((WIN32 OR CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64") AND NOT VBZ_DISABLE_SSE3)
    message(STATUS "SSE3 optimisations enabled")
    if(${CMAKE_CXX_COMPILER_ID} MATCHES "IntelLLVM" OR NOT MSVC)
        target_compile_options(vbz PRIVATE -mssse3)
    endif()
endif()

target_link_libraries(vbz
    PUBLIC
        ${STREAMVBYTE_STATIC_LIB}
        ${zstd_target}
)

if (BUILD_TESTING)
    add_subdirectory(fuzzing)
    add_subdirectory(test)

    if (ENABLE_PERF_TESTING)
        add_subdirectory(perf)
    endif()
endif()
