find_package(CURL REQUIRED)

set(LIB_NAME vsgCs)

set(LIB_PUBLIC_HEADERS
  accessorUtils.h
  accessor_traits.h
  ${GENERATED_HEADERS_DIR}/vsgCs/Config.h
  CRS.h
  CsDebugColorizeTilesOverlay.h
  CsOverlay.h
  CesiumGltfBuilder.h
  Export.h
  GeoNode.h
  GeospatialServices.h
  GraphicsEnvironment.h
  jsonUtils.h
  LoadGltfResult.h
  ModelBuilder.h
  RuntimeEnvironment.h
  ShaderFactory.h
  Styling.h
  TilesetNode.h
  vsgResourcePreparer.h
  runtimeSupport.h
  WorldNode.h
)

set(SOURCES
  ${LIB_PUBLIC_HEADERS}
  CRS.cpp
  CsDebugColorizeTilesOverlay.cpp
  CsOverlay.cpp
  CesiumGltfBuilder.cpp
  CompilableImage.cpp
  GeoNode.cpp
  GeospatialServices.cpp
  GraphicsEnvironment.cpp
  jsonUtils.cpp
  ModelBuilder.cpp
  OpThreadTaskProcessor.cpp
  RuntimeEnvironment.cpp
  ShaderFactory.cpp
  Styling.cpp
  TilesetNode.cpp
  UrlAssetAccessor.cpp
  runtimeSupport.cpp
  vsgResourcePreparer.cpp
  pbr.cpp
  WorldNode.cpp
)

add_library(
    ${LIB_NAME}
    ${LIB_PUBLIC_HEADERS}
    ${SOURCES}
)

set_target_properties(${LIB_NAME} PROPERTIES
  PUBLIC_HEADER "${LIB_PUBLIC_HEADERS}"
)

target_include_directories(${LIB_NAME} PRIVATE ${CURL_INCLUDE_DIRS})

if(VSGCS_BUILD_SHARED_LIBS)
  target_compile_definitions(${LIB_NAME} PRIVATE VSGCS_LIBRARY)
else()
  target_compile_definitions(${LIB_NAME} PUBLIC VSGCS_LIBRARY_STATIC)
endif()

# This important define should probably be in the import definition
# for spdlog in FindCesium.cmake. Without it there all sorts of
# multiple definition errors.

target_compile_definitions(${LIB_NAME} PUBLIC SPDLOG_COMPILED_LIB)

target_link_libraries(${LIB_NAME} PUBLIC vsg::vsg vsgImGui::vsgImGui Cesium3DTilesSelection CesiumGltfReader CesiumGltf spdlog PRIVATE CURL::libcurl PROJ::proj)

if(TRACY_ENABLE)
  target_link_libraries(${LIB_NAME} PUBLIC Tracy::TracyClient)
endif()

install(TARGETS ${LIB_NAME} EXPORT vsgCsTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME}"
  )

if(WIN32)
  install(IMPORTED_RUNTIME_ARTIFACTS CURL::libcurl_shared)
endif()
