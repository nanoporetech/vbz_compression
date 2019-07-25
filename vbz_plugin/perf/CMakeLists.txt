find_package(GoogleBenchmark)

add_executable(vbz_hdf_perf_test
    vbz_hdf_perf.cpp
)
add_sanitizers(vbz_hdf_perf_test)

target_link_libraries(vbz_hdf_perf_test
    PRIVATE
        google::benchmark
        HDF5::C
        hdf_test_utils
        vbz_hdf_plugin
)

set_property(TARGET vbz_hdf_perf_test PROPERTY CXX_STANDARD 11)

add_test(
    NAME vbz_hdf_perf_test
    COMMAND vbz_hdf_perf_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)