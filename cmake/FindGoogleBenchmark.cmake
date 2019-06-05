
find_path(GOOGLE_BENCHMARK_INCLUDE_DIR
    NAMES benchmark/benchmark.h
)

set(GOOGLE_BENCHMARK benchmark)
set(GOOGLE_BENCHMARK_DEBUG benchmarkd)

find_library(GOOGLE_BENCHMARK_LIBRARY_RELEASE NAMES ${GOOGLE_BENCHMARK})
find_library(GOOGLE_BENCHMARK_LIBRARY_DEBUG NAMES ${GOOGLE_BENCHMARK_DEBUG})

include(SelectLibraryConfigurations)
select_library_configurations(GOOGLE_BENCHMARK)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GOOGLE_BENCHMARK REQUIRED_VARS GOOGLE_BENCHMARK_LIBRARY GOOGLE_BENCHMARK_INCLUDE_DIR)


if (GOOGLE_BENCHMARK_FOUND)
    set(GOOGLE_BENCHMARK_INCLUDE_DIRS ${GOOGLE_BENCHMARK_INCLUDE_DIR})


    if (NOT GOOGLE_BENCHMARK_LIBRARIES)
        set(GOOGLE_BENCHMARK_LIBRARIES ${GOOGLE_BENCHMARK_LIBRARY})
    endif()

    if (NOT TARGET google::benchmark)
        add_library(google::benchmark UNKNOWN IMPORTED)
        set_target_properties(google::benchmark PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${GOOGLE_BENCHMARK_INCLUDE_DIRS}")

        set(EXTRA_LIBRARIES)
        if (WIN32)
            set(EXTRA_LIBRARIES shlwapi.lib)
        endif()

        find_package (Threads)
        if (CMAKE_THREAD_LIBS_INIT)
            set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
        endif()

        if (EXTRA_LIBRARIES)
            set_target_properties(google::benchmark 
                PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES
                    ${EXTRA_LIBRARIES}
            )
        endif()

        if(GOOGLE_BENCHMARK_LIBRARY_RELEASE)
            set_property(TARGET google::benchmark APPEND PROPERTY
                IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(google::benchmark PROPERTIES
                IMPORTED_LOCATION_RELEASE "${GOOGLE_BENCHMARK_LIBRARY_RELEASE}")
        endif()

        if(GOOGLE_BENCHMARK_LIBRARY_DEBUG)
            set_property(TARGET google::benchmark APPEND PROPERTY
                IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(google::benchmark PROPERTIES
                IMPORTED_LOCATION_DEBUG "${GOOGLE_BENCHMARK_LIBRARY_DEBUG}")
        endif()

        if(NOT GOOGLE_BENCHMARK_LIBRARY_RELEASE AND NOT GOOGLE_BENCHMARK_LIBRARY_DEBUG)
            set_property(TARGET google::benchmark APPEND PROPERTY
                IMPORTED_LOCATION "${GOOGLE_BENCHMARK_LIBRARY}")
        endif()
    endif()
endif()