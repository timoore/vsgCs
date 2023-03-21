set(SOURCES
  gltfviewer.cpp
)

SET(TARGET_SRC ${SOURCES})

INCLUDE_DIRECTORIES(${Vulkan_INCLUDE_DIR})

add_executable(gltfviewer ${SOURCES})

target_link_libraries(gltfviewer vsgCs CsApp vsg::vsg)

target_link_libraries(gltfviewer vsgImGui::vsgImGui)

install(TARGETS gltfviewer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
