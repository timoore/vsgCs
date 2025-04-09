find_package(CURL REQUIRED)

set(LIB_NAME vsgCs)

set(LIB_PUBLIC_HEADERS
  accessorUtils.h
  accessor_traits.h
  ${PROJECT_BINARY_DIR}/include/vsgCs/Config.h
  CRS.h
  CsDebugColorizeTilesOverlay.h
  CsOverlay.h
  CesiumGltfBuilder.h
  ${CMAKE_CURRENT_BINARY_DIR}/Export.h
  GeoNode.h
  GeospatialServices.h
  GltfLoader.h
  GraphicsEnvironment.h
  jsonUtils.h
  LoadGltfResult.h
  ModelBuilder.h
  RuntimeEnvironment.h
  ShaderFactory.h
  Styling.h
  TracingCommandGraph.h
  TilesetNode.h
  Version.h
  vsgResourcePreparer.h
  runtimeSupport.h
  WorldAnchor.h
  WorldNode.h
)

set(SOURCES
  CRS.cpp
  CsDebugColorizeTilesOverlay.cpp
  CsOverlay.cpp
  CesiumGltfBuilder.cpp
  CompilableImage.cpp
  GeoNode.cpp
  GeospatialServices.cpp
  GltfLoader.cpp
  GraphicsEnvironment.cpp
  jsonUtils.cpp
  ModelBuilder.cpp
  OpThreadTaskProcessor.cpp
  RuntimeEnvironment.cpp
  ShaderFactory.cpp
  Styling.cpp
  TracingCommandGraph.cpp
  TilesetNode.cpp
  UrlAssetAccessor.cpp
  runtimeSupport.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/Version.cpp
  vsgResourcePreparer.cpp
  pbr.cpp
  WorldAnchor.cpp
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

target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)
target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

if(VSGCS_BUILD_SHARED_LIBS)
  target_compile_definitions(${LIB_NAME} PRIVATE VSGCS_LIBRARY)
else()
  target_compile_definitions(${LIB_NAME} PUBLIC VSGCS_LIBRARY_STATIC)
endif()

include(CheckIncludeFileCXX)

check_include_file_cxx("sys/utsname.h" HAVE_SYS_UTSNAME)

if(HAVE_SYS_UTSNAME)
  target_compile_definitions(${LIB_NAME} PRIVATE HAVE_SYS_UTSNAME)
endif()

target_link_libraries(${LIB_NAME} PUBLIC
  vsg::vsg vsgImGui::vsgImGui Cesium3DTilesSelection CesiumRasterOverlays CesiumGltfContent CesiumGltfReader CesiumGltf)
if(CESIUM_SPDLOG_HEADER_ONLY)
  target_link_libraries(${LIB_NAME} PUBLIC spdlog::spdlog_header_only)
else()
  target_link_libraries(${LIB_NAME} PUBLIC spdlog::spdlog)
endif()
target_link_libraries(${LIB_NAME} PRIVATE CURL::libcurl PROJ::proj)

if(BUILD_TRACY)
  target_link_libraries(${LIB_NAME} PUBLIC Tracy::TracyClient)
endif()

install(TARGETS ${LIB_NAME} EXPORT vsgCsTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME}"
  )

configure_file("Config.h.in"
  "${PROJECT_BINARY_DIR}/include/vsgCs/Config.h")

configure_file("Version.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Version.cpp")
  
include(GenerateExportHeader)
generate_export_header(vsgCs EXPORT_FILE_NAME Export.h)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/Export.h"
  "${PROJECT_BINARY_DIR}/include/vsgCs/Config.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME}"
)
