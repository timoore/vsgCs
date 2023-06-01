add_library(cesium::Utility ALIAS CesiumUtility)
add_library(cesium::JsonReader ALIAS CesiumJsonReader)
add_library(cesium::GltfReader ALIAS CesiumGltfReader)
add_library(cesium::Gltf ALIAS CesiumGltf)
add_library(cesium::Geometry ALIAS CesiumGeometry)
add_library(cesium::Geospatial ALIAS CesiumGeospatial)
add_library(cesium::Async ALIAS CesiumAsync)
add_library(cesium::3DTilesSelection ALIAS Cesium3DTilesSelection)

install(TARGETS
  spdlog
  tinyxml2
  draco_static
  CesiumUtility
  CesiumJsonReader
  CesiumGltfReader
  CesiumGltf
  CesiumGeometry
  CesiumGeospatial
  CesiumAsync
  Cesium3DTilesSelection
  EXPORT CesiumExports)

install(EXPORT CesiumExports
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vsgCs
  NAMESPACE cesium::)

