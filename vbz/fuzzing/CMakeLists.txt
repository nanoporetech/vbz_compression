
if (SANITIZE_FUZZER)
    message(STATUS "Building vbz fuzz support")

    add_executable(vbz_fuzz_test
        vbz_fuzz.cpp
    )

    target_link_libraries(vbz_fuzz_test
        PUBLIC
            vbz)

    add_sanitize_fuzzer(vbz_fuzz_test)
    add_sanitizers(vbz_fuzz_test)
endif()