vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO timoore/cesium-native
        REF 197eecc1e620666333202f0a67ea6f1c1a80c9cd
        SHA512 d89c587ae09cc01ba6eccf34bb07c1a1f7a115b90f8c7824d79e7033d6ee33533a6db4724493a09149b31feb9aede05efb9e6ec338c4efc2a33d0d3c8266fec1
        HEAD_REF vcpkg-pkg
        )

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        OPTIONS
                -DVCPKG_MANIFEST_MODE=ON
        )

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/cesium-native PACKAGE_NAME cesium-native)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")

