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
  REF "v${VERSION}"
  SHA512 cad6164bf81d83174be0a97ea5f626267d3ebe59f058957fd375215159903a3128a390471bdb4ee2fee6aa09ca3996904b409d30021d441b7416ffd8f16e1593
  HEAD_REF main
        )

# cesium-native doesn't provide a convenient way to turn off
# warning-as-error, but compiling with clang can produce a spew of
# errors

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        OPTIONS
                -DCESIUM_USE_EZVCPKG=OFF
                --compile-no-warning-as-error
        )

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH share/cesium-native/cmake PACKAGE_NAME cesium-native)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
