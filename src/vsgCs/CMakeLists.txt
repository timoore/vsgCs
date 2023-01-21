IF(VSGCS_BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DVSGCS_LIBRARY)
ELSE()
  ADD_DEFINITIONS(-DVSGCS_LIBRARY_STATIC)
ENDIF()

find_package(CURL REQUIRED)

set(LIB_NAME vsgCs)

set(LIB_PUBLIC_HEADERS
  ${GENERATED_HEADERS_DIR}/vsgCs/Config.h
  CSOverlay.h
  CesiumGltfBuilder.h
  Export.h
  ImageComponent.h
  LoadGltfResult.h
  RuntimeEnvironment.h
  TilesetNode.h
  vsgResourcePreparer.h
  runtimeSupport.h
  WorldNode.h
)

set(SOURCES
  ${LIB_PUBLIC_HEADERS}
  CSOverlay.cpp
  CesiumGltfBuilder.cpp
  CompilableImage.cpp
  DescriptorSetConfigurator.cpp
  ImageComponent.cpp
  MultisetPipelineConfigurator.cpp
  OpThreadTaskProcessor.cpp
  RuntimeEnvironment.cpp
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

target_include_directories(${LIB_NAME} PRIVATE ${CURL_INCLUDE_DIR})

target_link_libraries(${LIB_NAME} vsg::vsg vsgImGui::vsgImGui cesium::3DTilesSelection cesium::GltfReader cesium::Gltf ${CURL_LIBRARIES})

target_compile_definitions(${LIB_NAME} PRIVATE vsgXchange_FOUND)

  

install(TARGETS vsgCs EXPORT vsgCsTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME}"
  )
