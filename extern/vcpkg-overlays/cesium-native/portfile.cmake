vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO timoore/cesium-native
        REF 6c1b1edd4ee388f6372dcfe1c33de6839696e5b4
        SHA512 1dd74feefe20dc0363c42f5b1a03d271de5d37cc86078ecc1090fa1c3517284c04ee0ae9f6c622f2b6391a48bccbf59a9b0f342e4e21f33a78c1cb85845b8b98
        HEAD_REF vcpkg-pkg
        )

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        OPTIONS
                -DCESIUM_USE_EZVCPKG=OFF
        )

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/cesium-native PACKAGE_NAME cesium-native)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")

