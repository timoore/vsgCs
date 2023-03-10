set(LIB_NAME CsApp)

set(LIB_PUBLIC_HEADERS
  IonIconComponent.h
)

set(SOURCES
  IonIconComponent.cpp
)

add_library(
    ${LIB_NAME}
    ${LIB_PUBLIC_HEADERS}
    ${SOURCES}
)

set_target_properties(${LIB_NAME} PROPERTIES
  PUBLIC_HEADER "${LIB_PUBLIC_HEADERS}"
)

# XXX will need its own Export.h and symbols if we ever make it a shared lib!
if(VSGCS_BUILD_SHARED_LIBS)
  target_compile_definitions(${LIB_NAME} PRIVATE VSGCS_LIBRARY)
else()
  target_compile_definitions(${LIB_NAME} PUBLIC VSGCS_LIBRARY_STATIC)
endif()

target_link_libraries(${LIB_NAME} vsgCs vsg::vsg vsgImGui::vsgImGui cesium::3DTilesSelection cesium::GltfReader cesium::Gltf)

install(TARGETS CsApp EXPORT CsAppTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME}"
  )
