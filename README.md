vsgCs  —  3D Tiles and Cesium ion for VSG
=====
vsgCs is a library for using [3D
Tiles](https://github.com/CesiumGS/3d-tiles) and other geospatial
    content within a [Vulkan Scene Graph](https://github.com/vsg-dev/VulkanSceneGraph) (VSG)
    application. In particular, it can download assets from a 
    [Cesium ion](https://cesium.com/platform/cesium-ion/) server.

<img src="doc/img/collioure.png" alt="Collioure, France">

For streaming and loading content, vsgCs uses the [Cesium Native](https://github.com/CesiumGS/cesium-native)
package, which also "powers Cesium's runtime integrations for
[Cesium for Unreal](https://github.com/CesiumGS/cesium-unreal),
[Cesium for Unity](https://github.com/CesiumGS/cesium-unity), and
[Cesium for O3DE](https://github.com/CesiumGS/cesium-o3de)."

---

## Features

- Streaming of geospatial assets into a VSG scene graph
  - 3D Tiles tilesets with [glTF](https://www.khronos.org/gltf/) models
  - Cesium ion assets
  
- Multiple tilesets in a scene
- Image overlays on tilesets
  - Multiple layered overlays with individual alpha values

- Whole-Earth terrain paging
- Example viewing application

---

## Installation

### Prerequisites:
#### Vulkan Scene Graph libraries
Follow the installation instructions for:
- [VulkanSceneGraph](https://github.com/vsg-dev/VulkanSceneGraph). In
  particular, you will need the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
  from LunarG. The Vulkan SDK debugging libraries are required to make a Debug configuration of vsgCS. 
- [vsgImGui](https://github.com/vsg-dev/vsgImGui)

#### Cesium Native

Download  [Cesium Native](https://github.com/CesiumGS/cesium-native),
follow its compilation instructions, and install. 

Note: In order to check out Cesium Native from git, you will need to
have the git-lfs component installed. This comes with Git for Windows
but is not installed by default on Linux.

### Command line build instructions:

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
---


## Usage

### Cesium ion

Unless you have your own 3D Tiles tilesets, you will need to stream
content from Cesium ion, so [create and account there](https://ion.cesium.com/signup/)
if you need to. Do the tutorial for creating a 3D tile if you want. Your "My Assets" page will look
something like:

<img src="doc/img/my_assets.png" alt="My Assets">

vsgCs uses the asset IDs in the left column to stream data from Cesium
ion. The assets with type "Imagery" can't be used on their own; they
must be draped over a tileset. Asset 1, "Cesium World Terrain", is
a tileset with only elevation data, so must be used with an overlay to
be useful.

### Access token

Any interaction with a Cesium ion server requires an access token. On
the "Access Tokens" page of Cesium ion, copy your token. while it can
be passed on the command line to csclient, it is more convenient to
store it in a file e.g., `.cesiumiontoken` in your home directory.

### `worldviewer`

The `worldviewer` program demonstrates the basic features of vsgCs. To
view the world:

```
worldviewer --ion-token-file ~/.cesiumiontoken --world-file vsgCs/tests/csterrain.json
```

which should produce:

<img src="doc/img/world.png" alt="the world">

`worldviewer` also displays discrete 3D Tiles tilesets. In my Cesium ion
account the AGI building has asset id 1418857, so:

```
worldviewer --ion-asset 1418857 --ion-token-file ~/.cesiumiontoken
```

produces

<img src="doc/img/agi.png" alt="AGI">

---

## JSON input files

vsgCs and the `worldviwer` program can read a simple JSON description
of the tilesets and overlays to include in a scene. See the
[tests](tests) directory for examples. These include:

* [csterrain.json](tests/csterrain.json) Cesium World Terrain with
  Bing images
* [color-debug-overlay.json](tests/color-debug-overlay.json) Debugging
  layer shows tile extents
* [multilayer.json](tests/multilayer.json) Multiple layers
* [osm-buildings.json](tests/osm-buildings.json) The world with OSM buildings
* [agi.json](tests/agi.json) The ion tutorial tileset

---

## Status

vsgCs and `worldviewer` have been tested with the Cesium World Terrain
and other imagery and tilesets available in the Cesium ion Asset
Depot. The OSM Buildings tileset works. For future directions, check
out our [roadmap](doc/ROADMAP.md). Enjoy!

<img src="doc/img/london.png" alt="London, England">
