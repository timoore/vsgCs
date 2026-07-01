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
  SHA512 08dc383c40172c0393a5e06864968d9fc8648197288febe0e32b181c3ba5456901a18d29b674fc36746fc9c9c04efee5c7fa0de9db401243361fc8916bf7cbee
  HEAD_REF main
        )

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
        OPTIONS
                -DCESIUM_USE_EZVCPKG=OFF
                -DCMAKE_COMPILE_WARNING_AS_ERROR=OFF
        )

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH share/cesium-native/cmake PACKAGE_NAME cesium-native)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
