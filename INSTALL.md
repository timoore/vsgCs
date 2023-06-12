# Build and Installation

## Prerequisites
vsgCs uses the `FetchContent` package of `cmake` to download most
dependencies. This is convenient for casual users, but might be too
inflexible if you want to use your own version of those
dependencies. You can force `FetchContent` to use the source code in a
certain location by setting the corresponding
`FETCHCONTENT_SOURCE_DIR_` variable e.g., to use your own version of
Cesium Native sources, set `FETCHCONTENT_SOURCE_DIR_CESIUM`.

These dependencies are not downloaded via `FetchContent` and should be
installed manually:

- `libcurl` This is best installed via a package manager.
- the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) from LunarG. Follow installation instructions
  for your operating system.
- The `git-lfs` component of git. This comes with Git for Windows but is not installed by default on Linux.

The following are downloaded by `cmake`:

- [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph)
- [vsgImGui](https://github.com/vsg-dev/vsgImGui)

- Cesium Native vsgCs uses a fork of Cesium Native that is better
  integrated with `cmake`. See [CMakeLists.txt](./CMakeLists.txt) for
  details. `cmake` may take a long time to download the Cesium Native
  library and its dependencies, and occaisionally there are errors
  from which `FetchContent` cannot recover. A workaround is to clone
  Cesium Native yourself, using the repository and tag in
  `CmakeLists.txt`, and then set the `FETCHCONTENT_SOURCE_DIR_CESIUM`
  cache variable.

## Command line build instructions:

To build and install the static library (libvsgCs.a / libvsgCs.lib) and sample
application (worldviewer):
```
    git clone https://github.com/timoore/vsgCs.git
    mkdir build # or wherever you like
    cd build
    cmake ../vsgCs.git
    make -j 8
    sudo make install
```
