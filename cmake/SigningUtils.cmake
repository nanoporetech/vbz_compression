function(hdf_add_signing_command target)
    set(options)
    set(oneValArgs FILE)
    set(multiValArgs)
    cmake_parse_arguments(ARG "${options}" "${oneValArgs}" "${multiValArgs}" ${ARGN})

    if (ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments to hdf_add_signing_command: ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    if (NOT APPLE AND NOT WIN32)
        message(FATAL_ERROR "hdf_add_signing_command called on unsupported platform - platform=${CMAKE_SYSTEM_NAME}, target=${target}")
    endif()

    if (NOT ARG_FILE)
        set(ARG_FILE $<TARGET_FILE:${target}>)
    endif()

    add_custom_command(
        TARGET ${target}
        POST_BUILD
            COMMAND ${CMAKE_COMMAND} -P ${HDF_PLUGIN_SOURCE_DIR}/cmake/SignFile.cmake ${ARG_FILE}
            COMMENT "Signing ${target}"
    )
endfunction()
