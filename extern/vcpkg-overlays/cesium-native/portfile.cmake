vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO timoore/cesium-native
        REF 67336f4c3183bd1e27298a44b88b9bb90badb5f8
        SHA512 5e2967b5341593d17b8062f8f2e80265797d55e57f1f7983b497b5c32faf4dad33ab3c1637b18ae1a1e54502b2c4e9ae13bd6de2ff855687080771529344f737
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

