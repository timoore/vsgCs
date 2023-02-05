find_package(CURL REQUIRED)

set(LIB_NAME vsgCs)

set(LIB_PUBLIC_HEADERS
  ${GENERATED_HEADERS_DIR}/vsgCs/Config.h
  CSDebugColorizeTilesOverlay.h
  CSOverlay.h
  CsView.h
  CesiumGltfBuilder.h
  Export.h
  ImageComponent.h
  jsonUtils.h
  LoadGltfResult.h
  RuntimeEnvironment.h
  TilesetNode.h
  ViewDependentStateExt.h
  vsgResourcePreparer.h
  runtimeSupport.h
  WorldNode.h
)

set(SOURCES
  ${LIB_PUBLIC_HEADERS}
  CSDebugColorizeTilesOverlay.cpp
  CSOverlay.cpp
  CsView.cpp
  CesiumGltfBuilder.cpp
  CompilableImage.cpp
  DescriptorSetConfigurator.cpp
  ImageComponent.cpp
  jsonUtils.cpp
  MultisetPipelineConfigurator.cpp
  OpThreadTaskProcessor.cpp
  RuntimeEnvironment.cpp
  TilesetNode.cpp
  UrlAssetAccessor.cpp
  runtimeSupport.cpp
  ViewDependentStateExt.cpp
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

target_link_libraries(${LIB_NAME} vsg::vsg vsgImGui::vsgImGui cesium::3DTilesSelection cesium::GltfReader cesium::Gltf ${CURL_LIBRARIES} spdlog)

install(TARGETS vsgCs EXPORT vsgCsTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME}"
  )
