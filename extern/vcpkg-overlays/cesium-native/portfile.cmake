vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
  FEATURES
  dependencies-only CESIUM_NATIVE_DEPS_ONLY
)

if(CESIUM_NATIVE_DEPS_ONLY)
  message(STATUS "skipping installation of cesium-native")
  set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
  return()
endif()

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO CesiumGS/cesium-native
        REF e221a72631a184864acfa4863820a8a7d8bb629b
        SHA512 65a37611728a1db9a81c94b0a5c0a91071eb96cf37c3e9ecbd4bcfacb4da16cfff7a8af6fea5aeb28f31dd371f79226142bc029409c1c6f68ecc3c9f5078e020
        HEAD_REF vcpkg-pkg
        )

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        OPTIONS
                -DCESIUM_USE_EZVCPKG=OFF
        )

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH share/cesium-native/cmake PACKAGE_NAME cesium-native)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
