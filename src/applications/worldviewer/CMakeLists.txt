set(SOURCES
  worldviewer.cpp
  UI.cpp
)

SET(TARGET_SRC ${SOURCES})

INCLUDE_DIRECTORIES(${Vulkan_INCLUDE_DIR})

add_executable(worldviewer ${SOURCES})

target_link_libraries(worldviewer vsgCs CsApp vsg::vsg)

target_link_libraries(worldviewer vsgImGui::vsgImGui)

if (vsgXchange_FOUND)
  target_compile_definitions(worldviewer PRIVATE vsgXchange_FOUND)
  target_link_libraries(worldviewer vsgXchange::vsgXchange)
endif()

install(TARGETS worldviewer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
