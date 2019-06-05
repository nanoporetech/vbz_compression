add_executable(vbz_hdf_plugin_test
    vbz_hdf_plugin_test.cpp
    main.cpp
)

set_property(TARGET vbz_hdf_plugin_test PROPERTY CXX_STANDARD 11)

target_link_libraries(vbz_hdf_plugin_test
    PUBLIC
        vbz_hdf_plugin
        HDF5::C
        hdf_test_utils
)

add_test(
    NAME vbz_hdf_plugin_test
    COMMAND vbz_hdf_plugin_test "${CMAKE_SOURCE_DIR}/test_data"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)