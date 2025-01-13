
option(SANITIZE_FUZZER "Enable Fuzzer for sanitized targets." Off)

include(sanitize-helpers)

if (SANITIZE_FUZZER)
    set(CMAKE_REQUIRED_FLAGS "-g -fsanitize=fuzzer")
    check_cxx_source_compiles("#include <cstdint>\n#include <cstddef>\nextern \"C\" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) { return 0; }" Fuzz_FLAG_DETECTED)

    if (Fuzz_FLAG_DETECTED)
        get_property(ENABLED_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
        foreach (LANG ${ENABLED_LANGUAGES})
            # Sanitizer flags are not dependend on language, but the used compiler.
            # So instead of searching flags foreach language, search flags foreach
            # compiler used.
            set(COMPILER ${CMAKE_${LANG}_COMPILER_ID})
            set(FuzzExe_${COMPILER}_FLAGS "${CMAKE_REQUIRED_FLAGS}" CACHE STRING "Fuzzer arguments")
            set(Fuzz_${COMPILER}_FLAGS "-g -fsanitize=fuzzer-no-link" CACHE STRING "FuzzerExe arguments")
        endforeach()
    else()
        message(FATAL_ERROR "Compiler doesn't support fuzzing")
    endif()
endif()

function(add_sanitize_fuzzer TARGET)
    if (NOT SANITIZE_FUZZER)
        return()
    endif ()

    sanitizer_add_flags(${TARGET} "FuzzSanitizer" "Fuzz")
    target_compile_options(${TARGET} PUBLIC -D SANITIZE_FUZZER -U NDEBUG)
endfunction()

function(add_sanitize_fuzzer_exe TARGET)
    if (NOT SANITIZE_FUZZER)
        return()
    endif ()

    sanitizer_add_flags(${TARGET} "FuzzSanitizer" "FuzzExe")
    target_compile_options(${TARGET} PUBLIC -D SANITIZE_FUZZER -U NDEBUG)
endfunction()
