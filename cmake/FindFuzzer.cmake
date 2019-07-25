
option(SANITIZE_FUZZER "Enable Fuzzer for sanitized targets." Off)

include(sanitize-helpers)

if (SANITIZE_FUZZER)
    set(CMAKE_REQUIRED_FLAGS "-g -fsanitize=fuzzer")
    check_cxx_source_compiles("#include <cstdint>\n#include <cstddef>\nextern \"C\" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {}" Fuzz_FLAG_DETECTED)

    get_property(ENABLED_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
    foreach (LANG ${ENABLED_LANGUAGES})
        # Sanitizer flags are not dependend on language, but the used compiler.
        # So instead of searching flags foreach language, search flags foreach
        # compiler used.
        set(COMPILER ${CMAKE_${LANG}_COMPILER_ID})
        if (Fuzz_FLAG_DETECTED)
            set(Fuzz_${COMPILER}_FLAGS "${CMAKE_REQUIRED_FLAGS}" CACHE STRING "Fuzzer arguments")
        endif()
    endforeach()
endif()

function(add_sanitize_fuzzer TARGET)
    if (NOT SANITIZE_FUZZER)
        return()
    endif ()

    sanitizer_add_flags(${TARGET} "FuzzSanitizer" "Fuzz")
endfunction ()
