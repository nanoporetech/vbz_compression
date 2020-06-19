set(file_to_sign ${CMAKE_ARGV3})

# Pull from command line by default - otherwise require an environment variable.
if (NOT MINKNOW_CODE_SIGN_IDENTITY)
    if ("$ENV{MINKNOW_CODE_SIGN_IDENTITY}" STREQUAL "")
        message(FATAL_ERROR "Caller must specify code sign identiyy in environment variable 'MINKNOW_CODE_SIGN_IDENTITY'")
    endif()

    set(MINKNOW_CODE_SIGN_IDENTITY "$ENV{MINKNOW_CODE_SIGN_IDENTITY}")
endif()

if (APPLE)
    message("Signing file... ${file_to_sign}${keychain_comment}")
    set(SIGN_COMMAND 
        codesign -s ${MINKNOW_CODE_SIGN_IDENTITY} ${keychain_arg} --force --deep -vvvv ${file_to_sign})
elseif(WIN32)
    find_program(
        SIGNTOOL_EXE "Signtool.exe"
        PATHS "C:\\Program Files (x86)\\Windows Kits\\10\\App Certification Kit")

    if (NOT SIGNTOOL_EXE)
        message(FATAL_ERROR "Failed to find signtool executable")
    endif()

    message("Signing file... ${file_to_sign}${comment}")
    set(SIGN_COMMAND ${SIGNTOOL_EXE} sign "/v" "/sha1" "${MINKNOW_CODE_SIGN_IDENTITY}"
        "/t" "http://timestamp.globalsign.com/scripts/"
        ${file_to_sign}
    )

else()
    message(FATAL_ERROR "Cannot sign code on this platform.")
endif()

message("Running sign command: ${SIGN_COMMAND}")
execute_process(
    COMMAND ${SIGN_COMMAND}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
)

if (NOT result EQUAL 0)
    message(FATAL_ERROR "Could not sign file: ${result}: ${output}")
else()
    message("Signed file: ${output}")
endif()