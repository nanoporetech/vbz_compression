set(file_to_sign ${CMAKE_ARGV3})

# Pull from command line by default - otherwise require an environment variable.
macro(sk_pull_var_from_env var)
    if (NOT ${var})
        if ("$ENV{${var}}" STREQUAL "")
            message(FATAL_ERROR "To use codesigning, set the ${var} environment variable")
        endif()

        set(${var} "$ENV{${var}}")
    endif()
endmacro()


if (APPLE)
    sk_pull_var_from_env(APPLE_APP_SIGN_KEY_THUMBPRINT)
    message("Signing file... ${file_to_sign}${keychain_comment}")
    set(SIGN_COMMAND 
        codesign -s ${APPLE_APP_SIGN_KEY_THUMBPRINT} ${keychain_arg} --force --deep -vvvv ${file_to_sign})
elseif(WIN32)
    sk_pull_var_from_env(WINDOWS_CODE_SIGN_THUMBPRINT)
    find_program(
        SIGNTOOL_EXE "Signtool.exe"
        PATHS "C:\\Program Files (x86)\\Windows Kits\\10\\App Certification Kit")

    if (NOT SIGNTOOL_EXE)
        message(FATAL_ERROR "Failed to find signtool executable")
    endif()

    message("Signing file... ${file_to_sign}${comment}")
    set(SIGN_COMMAND ${SIGNTOOL_EXE} sign "/v" "/sha1" "${WINDOWS_CODE_SIGN_THUMBPRINT}"
        "/tr" "http://rfc3161timestamp.globalsign.com/advanced"
        "/td" "SHA256"
        ${file_to_sign}
    )

else()
    message(FATAL_ERROR "Cannot sign code on this platform.")
endif()

set(retry_count 10)
foreach(retry_index RANGE ${retry_count})
    message("Running sign command: ${SIGN_COMMAND}")
    execute_process(
        COMMAND ${SIGN_COMMAND}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE output
    )

    if (result EQUAL 0)
        break()
    endif()

    message(WARN "Signing failed, waiting and retrying (${retry_index}/${retry_count})")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E sleep 5
    )

endforeach()

if (NOT result EQUAL 0)
    message(FATAL_ERROR "Could not sign file: ${result}: ${output}")
else()
    message("Signed file: ${output}")
endif()
