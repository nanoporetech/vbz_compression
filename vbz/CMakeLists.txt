
add_library(vbz STATIC
    vbz_streamvbyte.h
    vbz_streamvbyte.cpp
    vbz_streamvbyte_impl.h
    vbz_streamvbyte_impl_sse3.h
    vbz.h
    vbz.cpp)

generate_export_header(vbz)

set_property(TARGET vbz PROPERTY CXX_STANDARD 11)

target_include_directories(vbz
    PUBLIC
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${STREAMVBYTE_INSTALL_DIR}/include
)

add_dependencies(vbz
    streamvbyte
)

target_link_libraries(vbz
    PUBLIC
        ${STREAMVBYTE_STATIC_LIB}
        zstd::zstd
)

if (BUILD_TESTING)
    add_subdirectory(test)
    add_subdirectory(perf)
endif()