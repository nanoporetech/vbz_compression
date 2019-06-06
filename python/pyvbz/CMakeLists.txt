
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
            VBZ_INCLUDE_PATHS="${CMAKE_SOURCE_DIR}/vbz\;${CMAKE_BINARY_DIR}/"
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

add_custom_target(pyvbz_cffi "${PYVBZ_ALL}"
    DEPENDS "${PYVBZ_OUTPUT_PATH}")

if (NOT WIN32)
  add_test(
      NAME pyvbz_test
      COMMAND ./tests/unit
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()
