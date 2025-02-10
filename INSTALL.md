# Build and Installation

## Prerequisites

vsgCs manages its dependencies using the
[vcpkg](https://github.com/microsoft/vcpkg) package manager. You
should clone that repository and follow the steps described in [vcpkg
with CMake](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash).

On Linux, vcpkg will complain if some development packages aren't
installed in the system, although it's unclear if this makes any
practical difference. On Fedora, to take an example, you can install
them with:
```
sudo dnf install xorg-macros xproto xcb-proto libXdmcp-devel libXau-devel
```

## Command line build instructions:

Once you have set the `VCPKG_ROOT` environment variable, you can build
vsgCS using a CMake preset defined in `CMakePresets.json`, for example:

```
export VCPKG_ROOT=<location of vcpkg repository>
cmake --preset vcpkg-debug
cmake --build build-debug --target install -j 10
```

The `cmake` files for vsgCs don't explicitly depend on vcpkg, so you
can install vsgCs' dependencies yourself e.g., using your system's
package manager. Look in vsgCs' `CMakeLists.txt` files for calls to
`find_package` for prerequisites, as well as in the `vcpkg.json` files
in the `extern/vcpkg-overlays` subdirectories.

## Cesium Native

For this release candidate, vsgCs is using a private fork of the
Cesium Native library which is better integrated with `cmake`. This
branch is quite close to a [pullrequest](https://github.com/CesiumGS/cesium-native/pull/1026) in
the offical Cesium Native repository, and we intend to use Cesium
Native releases once that PR is merged.

vsgCs is often used as a vehicle for developing Cesium Native, and it
would be quite painful to create a vcpkg port and package for each
incremental change. You can include Cesium Native as a CMake
subdirectory from its source subdirectory using the
`VSGCS_CESIUM_NATIVE_DIRECTORY` cache variable. The CMake
`local-cesium-native` preset in `CMakePresets.json` uses vcpkg to get
Cesium Native's dependencies without Cesium Native itself, and so is
ideal for this usage. You can create a preset in a local
`CMakeUserPresets.json` file that configures this case:
```
{
  "version": 3,
  "configurePresets": [
    {
      "name": "my-local-native",
        "inherits": ["vcpkg-debug", "local-cesium-native"],
      "binaryDir": "${sourceDir}/build-debug",
      "cacheVariables": {
        "VSGCS_CESIUM_NATIVE_DIRECTORY": "/home/foo/CesiumGS/cesium-native"
      }
    }
  ]
}
```
