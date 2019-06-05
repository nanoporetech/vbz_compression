add_library(hdf_test_utils STATIC
    hdf_id_helper.cpp
    hdf_id_helper.h
)

set_property(TARGET hdf_test_utils PROPERTY CXX_STANDARD 11)

target_include_directories(hdf_test_utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(hdf_test_utils
    PRIVATE
        HDF5::C
)