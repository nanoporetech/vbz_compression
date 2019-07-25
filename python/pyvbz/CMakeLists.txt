
set(PYVBZ_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/pyvbz_cffi.touch")
set(PYVBZ_BUILD_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build_pyvbz.cmake")

if (NOT PYTHON_EXECUTABLE)
    find_package(PythonInterp 3)
endif()

add_custom_command(
    OUTPUT "${PYVBZ_OUTPUT_PATH}"
    COMMAND ${CMAKE_COMMAND}
        ARGS -E env
            PYTHON_EXE="${PYTHON_EXECUTABLE}"
            PYVBZ_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
            VBZ_BUILD_DIR="${CMAKE_BINARY_DIR}"
            VBZ_INCLUDE_PATHS="${CMAKE_SOURCE_DIR}/vbz\;${CMAKE_BINARY_DIR}/vbz/"
            VBZ_LINK_LIBS="$<TARGET_FILE:vbz>\;${STREAMVBYTE_STATIC_LIB}\;${ZSTD_LIBRARY}"
            OUTPUT_FILE="${PYVBZ_OUTPUT_PATH}"
            ${CMAKE_COMMAND}
                -P "${PYVBZ_BUILD_SCRIPT}"
    DEPENDS vbz "${PYVBZ_BUILD_SCRIPT}"
)

set(PYVBZ_ALL)
if (NOT WIN32)
    set(PYVBZ_ALL "ALL")
endif()

add_custom_target(pyvbz "${PYVBZ_ALL}"
    DEPENDS "${PYVBZ_OUTPUT_PATH}")

if (BUILD_TESTING AND NOT WIN32)
    set(PYVBZ_VENV_PATH "${CMAKE_CURRENT_BINARY_DIR}/venv")
    set(PYVBZ_VENV_PYTHON "${PYVBZ_VENV_PATH}/bin/python")
    add_custom_command(
        OUTPUT "${PYVBZ_VENV_PYTHON}"
        COMMAND ${PYTHON_EXECUTABLE} -m venv
            --system-site-packages "${CMAKE_CURRENT_BINARY_DIR}/venv"
        COMMAND "${PYVBZ_VENV_PYTHON}" -m pip install -r
            "${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt"
        COMMAND ${CMAKE_COMMAND} -E env
            VBZ_BUILD_DIR="${CMAKE_BINARY_DIR}"
            VBZ_INCLUDE_PATHS="${CMAKE_SOURCE_DIR}/vbz\;${CMAKE_BINARY_DIR}/vbz/"
            VBZ_LINK_LIBS="$<TARGET_FILE:vbz>\;${STREAMVBYTE_STATIC_LIB}\;${ZSTD_LIBRARY}"
            "${PYVBZ_VENV_PYTHON}" setup.py develop
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"
    )
    add_custom_target(pyvbz_venv ALL DEPENDS "${PYVBZ_VENV_PYTHON}")

    add_test(
        NAME pyvbz_test
        COMMAND "${PYVBZ_VENV_PYTHON}" ./tests/unit
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
