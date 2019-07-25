find_package(GoogleBenchmark)

add_executable(vbz_perf_test
    vbz_perf.cpp
)
add_sanitizers(vbz_perf_test)

target_link_libraries(vbz_perf_test
    PRIVATE
        vbz
        google::benchmark
)

set_property(TARGET vbz_perf_test PROPERTY CXX_STANDARD 11)

add_test(
    NAME vbz_perf_test
    COMMAND vbz_perf_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)