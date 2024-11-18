vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO timoore/cesium-native
        REF 606672401a8e184866c8ec3151974144fb6896fd
        SHA512 db8de61f1851dfd82a1729fe707064e3a5fd79f461e1e7fb9dff9cea2075f1ef842e3b93f1881bd22b11bfb847fb949405d7e66e6624125fc14720dfea4fe404
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

