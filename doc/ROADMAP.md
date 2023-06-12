vsgCs Roadmap and Future Directions
=====
vsgCs fills a niche for those who want to use 3D Tiles and Cesium ion
assets in a native application and who don't want or need all the
features of a commercial game engine. Developers who are coming from
the Open Scene Graph world will feel at home with the vsgCs code base
and, indeed, VSG. We cannot reimplement all the features of a game
engine, such as advanced rendering algorithms and graphical
application editors, but we can rely on the VSG ecosystem to evolve
and provide much functionality.

In the near term, we plan to implement these features:

- [X] Credits system based on ImGui
- [ ] A GeoReference node to to fix the world in a local coordinate system
- [ ] Finish glTF primitive support
  - [ ] handle multiple texture coordinate sets
  - [ ] Make better use of supported Vulkan vertex and texture data
    formats
  - [ ] Standalone glTF reader. This isn't directly supported by Cesium
    Native, but it would be helpful for testing glTF features that
    don't often show up in 3D Tiles.
- [ ] Overlay features
  - [ ] Support for non Cesium ion overlays. This is probably close to
    working using Cesium Native; we just need to create the right
    objects. Work has started on this, with a pull request to Cesium Native.
  - [ ] color to alpha
- [ ] Improved environment
  - [ ] A real time-based lighting model with ephemeris
  - [ ] sky box
- [ ] Caching asset accessor. vsgCs aleady provides an experimental
  option of using Cesium Native's SqliteCache, but we have seen
  problems of it "running out of memory."
- [ ] User interface for setting viewpoint, loading and reloading
  assets, etc.

In the longer term, and depending on the uses to which vsgCs is put,
we propose these features:

- [ ] Intersections with terrain, height above terrain, etc. Otherwise
     known as "physics." Intersection testing is probably close to
     working already.
- [ ] Create missing tangent vectors in glTF source.
- [ ] Tile water mask
- [ ] Do "something" with metadata. Read it into VSG, query it during
  intersection testing, create alternative renders with it.
  

     

  
