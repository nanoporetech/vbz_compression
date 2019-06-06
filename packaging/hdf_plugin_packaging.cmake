include(CPackComponent)

set(CPACK_PACKAGE_NAME ont-vbz-hdf-plugin)
set(CPACK_PACKAGE_VENDOR "nanoporetech")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/packaging/readme.txt")
#set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/packaging/welcome.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/packaging/readme.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")

cpack_add_component(
        hdf_plugin
        DISPLAYNAME "Hdf5 Plugin"
        GROUP HDF_PLUGIN)

set(CPACK_COMPONENTS_ALL hdf_plugin)

if (NOT HDF5_PLUGIN_PATH)
    IF (WIN32)
        set(HDF5_PLUGIN_PATH "C:/ProgramData/hdf5/lib/plugin")
    else()
        set(HDF5_PLUGIN_PATH "/usr/local/hdf5/lib/plugin")
    endif()
endif()

option (VBZ_BUILD_ARCHIVE "Build vbz release as an archive" OFF)
if (VBZ_BUILD_ARCHIVE)
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
elseif (APPLE)
    set(CPACK_GENERATOR "productbuild")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/Applications/OxfordNanopore/${CPACK_PACKAGE_NAME}/")

    set(ONT_VBZ_PLUGIN_SOURCE_PATH "${CPACK_PACKAGING_INSTALL_PREFIX}/libvbz_hdf_plugin.dylib")
    set(ONT_VBZ_PLUGIN_DEST_PATH "${HDF5_PLUGIN_PATH}/libvbz_hdf_plugin.dylib")

    set(POSTFLIGHT_SCRIPT "${CMAKE_BINARY_DIR}/postinstall.sh")
    configure_file(
        "${CMAKE_SOURCE_DIR}/packaging/postinstall.sh.in"
        "${POSTFLIGHT_SCRIPT}"
        @ONLY
    )

    set(CPACK_POSTFLIGHT_HDF_PLUGIN_SCRIPT "${POSTFLIGHT_SCRIPT}")
elseif (WIN32)
    set(CPACK_GENERATOR "WIX")
    set(CPACK_PACKAGE_VENDOR "Oxford Nanopore Technologies, Limited")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/packaging/wix_nanopore_icon.ico")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/packaging/wix_nanopore_background.bmp")
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/packaging/wix_nanopore_banner.bmp")

    if(VBZ_MSI_UPGRADE_CODE)
        set(CPACK_WIX_UPGRADE_GUID "${VBZ_MSI_UPGRADE_CODE}")
    endif()

    set(WIX_SOURCE_FILE "${CMAKE_SOURCE_DIR}/packaging/wix_extra.wxs.in")
    set(CPACK_WIX_EXTRA_SOURCES "${CMAKE_BINARY_DIR}/wix_extra.wxs")
    set(CPACK_WIX_PATCH_FILE "${CMAKE_SOURCE_DIR}/packaging/wix_patch.wxs.xml")
    
    install(CODE "
        set(ONT_VBZ_PLUGIN_SOURCE_PATH \"${CMAKE_BINARY_DIR}/bin/\$\{CMAKE_INSTALL_CONFIG_NAME\}/vbz_hdf_plugin.dll\")
        configure_file(
            \"${WIX_SOURCE_FILE}\"
            \"${CPACK_WIX_EXTRA_SOURCES}\"
            @ONLY
        )"
        COMPONENT hdf_plugin
    )

    set(VBZ_INSTALLATION_PREFIX "OxfordNanopore/${CPACK_PACKAGE_NAME}")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "${VBZ_INSTALLATION_PREFIX}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${VBZ_INSTALLATION_PREFIX}")
    set(CPACK_WIX_PROPERTY_ALLUSERS 1)  # Install to everyone by default

else()
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGING_INSTALL_PREFIX "${HDF5_PLUGIN_PATH}")

    set(CPACK_DEB_PACKAGE_COMPONENT ON)
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    find_program(DPKG dpkg)
    execute_process(
        COMMAND ${DPKG} --print-architecture  # e.g. "amd64" or "arm64"
        OUTPUT_VARIABLE DPKG_ARCHITECTURE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    find_program(LSB_RELEASE lsb_release)
    execute_process(
        COMMAND ${LSB_RELEASE} -cs  # e.g. "trusty" or "xenial"
        OUTPUT_VARIABLE LSB_PLATFORM_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(CPACK_DEBIAN_PACKAGE_VERSION "${PROJECT_VERSION}-1~${LSB_PLATFORM_NAME}")

    set(CPACK_DEBIAN_HDF_PLUGIN_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
    set(CPACK_DEBIAN_HDF_PLUGIN_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}_${DPKG_ARCHITECTURE}.deb")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER nanoporetech.com)
endif()

include(CPack)