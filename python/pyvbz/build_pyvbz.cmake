set(PYTHON "$ENV{PYTHON_EXE}")
set(SOURCE_DIR "$ENV{PYVBZ_DIR}")

message("Building pyvbz with ${PYTHON}")
message("Include paths for pyvbz $ENV{VBZ_INCLUDE_PATHS}")
message("Linking pyvbz against $ENV{VBZ_LINK_LIBS}")

execute_process(
    COMMAND "${PYTHON}" setup.py develop bdist_wheel
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
)

if (NOT result EQUAL 0)
    message(FATAL_ERROR "Could not build pyvbz: ${result}: ${output}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E touch "$ENV{OUTPUT_FILE}")
