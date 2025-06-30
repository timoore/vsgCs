# Change Log

### v1.1.1 - 2025-06-30

##### Additions

- vsgCs now uses Cesium Native v0.48.0.
- Many more raster overlay sources are supported: TMS, WMS, WMTS, and generic URLs with templates. Contributed by @chengdz.

### v1.0.0 - 2025-05-11

##### Breaking Changes

- vsgCs now requires C++20. This is driven by Cesium Native's requirements, and we are leaning into it. The main visible change is that `gsl::span` has been replaced with `std::span`.
- The `csclient` program has been removed.

##### Additions

- The official release of Cesium Native, at this time v0.47.0, is used. Previously vsgCs used a fork of Cesium Native.
- vsgCs now sends identifying headers with http requests.
- The control key works as a modifier in the manipulator.
- vsgCs now uses vcpkg to manage its dependencies, including VSG and, via an overlay port, Cesium Native.
- Added Basic github CI actions.
- Arguments to the `worldviewer` program have been greatly simplified. WorldNode files no longer require flags. Local and remote 3D Tilesets can be specified directly on the command line.
- The glTF `EXT_mesh_gpu_instancing` extension is supported. As a result, 3D Tiles .id3m files are now supported too.
- A World Anchor node, which places the world in a local coordinate frame, is added.
- vsgCs can now load glTF i.e., .gltf and .glb files. The new `gltfviewer` program displays glTF files.
- Added an option to build using header-only `spdlog`.
- Added GPU profiling with Tracy.
- Place collections of models using an aribitrary CRS. Added the `proj` library to do transforms between CRSes. Models can be loaded using VSG's vsgXchange library.
- Enabled depthClamp and samplerAnisotropy features (1 year, 7 months ago) 
- Added smooth LOD transitions based on a blue noise texture

##### Fixes
- Error messages from Curl were improved. Problems with OpenSSL finding the correct configuration files were fixed.
- The location specified by the --poi argument is actually used to initialize the home viewpoint.
- Several serious memory leaks are fixed.
- Use vsg::ViewDependentStateBinding in pbr shader set, which avoids a black Earth in some implementations.
- Use the transpose inverse modelview matrix for normals in the pbr shader

### v0.4.0 - 2023-06-12

##### Additions
  - vsgCs can stream Google Maps Photorealistic 3D Tiles.
  - Some support for tile styling has been added. If present, the `cesium#color` feature attribute is
    used to color tiles.
  - The mouse manipulator from the [rocky](https://github.com/pelicanmapping/rocky) project has been
    integrated. This provides better interactive control, especially close to the Earth, than the
    default VSG trackball manipulator.
  - Support for the [Tracy](https://github.com/wolfpld/tracy) profiling package has been added.
  - The credits display has been rewritten, using images and attribution from the data sources.
  - A "time of day" argument has been added to the `worldviewer` application, which is used in
    positioning the default light i.e., the Sun.
  - Point clouds in 3D tiles can be rendered.

##### Fixes
  - The build process is more self-contained and will now download Cesiuam native libraries and VSG.
  - The management of web connections and data download has been greatly improved.
