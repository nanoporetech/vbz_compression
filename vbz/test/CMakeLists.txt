add_executable(vbz_test
    streamvbyte_test.cpp
    test_data.h
    test_utils.h
    vbz_test.cpp
    main.cpp
)
add_sanitizers(vbz_test)

set_property(TARGET vbz_test PROPERTY CXX_STANDARD 11)

find_package( Threads )

target_link_libraries(vbz_test
    PUBLIC
        vbz
    ${CMAKE_THREAD_LIBS_INIT}
)

add_test(
    NAME vbz_test
    COMMAND vbz_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
