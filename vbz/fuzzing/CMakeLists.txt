
if (SANITIZE_FUZZER)
    message(STATUS "Building vbz fuzz support")

    add_executable(vbz_fuzz_test
        vbz_fuzz.cpp
    )
    target_link_libraries(vbz_fuzz_test
        PUBLIC
            vbz)
    target_compile_features(vbz_fuzz_test PRIVATE cxx_std_17)
    add_sanitizers(vbz_fuzz_test)
    add_sanitize_fuzzer_exe(vbz_fuzz_test)
endif()

include(CheckIncludeFileCXX)
check_include_file_cxx("filesystem" FILESYSTEM_AVAILABLE)
if (FILESYSTEM_AVAILABLE)
    # A runner that'll run through the corpus
    add_executable(vbz_fuzz_runner
        vbz_fuzz_runner.cpp
    )
    target_link_libraries(vbz_fuzz_runner
        PUBLIC
            vbz
    )
    target_compile_features(vbz_fuzz_runner PRIVATE cxx_std_17)
    add_sanitizers(vbz_fuzz_runner)
    add_test(
        NAME vbz_fuzz_runner
        COMMAND vbz_fuzz_runner ${CMAKE_SOURCE_DIR}/vbz/fuzzing/fuzz_corpus
    )
else()
    message(WARNING "Not building fuzz runner due to lack of <filesystem>")
endif()
