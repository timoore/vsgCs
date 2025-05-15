set(SOURCES
  worldviewer.cpp
  UI.cpp
)

SET(TARGET_SRC ${SOURCES})

INCLUDE_DIRECTORIES(${Vulkan_INCLUDE_DIR})

add_executable(worldviewer ${SOURCES})

target_link_libraries(worldviewer PUBLIC vsgCs CsApp vsg::vsg Microsoft.GSL::GSL)

target_link_libraries(worldviewer PUBLIC vsgImGui::vsgImGui)

if (vsgXchange_FOUND)
  target_compile_definitions(worldviewer PRIVATE vsgXchange_FOUND)
  target_link_libraries(worldviewer PRIVATE vsgXchange::vsgXchange)
endif()

if (BUILD_TRACY)
  target_link_libraries(worldviewer PUBLIC Tracy::TracyClient)
endif()

if(VALGRIND_H_PATH)
  target_include_directories(worldviewer PRIVATE "${VALGRIND_H_PATH}")
  target_compile_definitions(worldviewer PRIVATE HAVE_VALGRIND)
endif()

install(TARGETS worldviewer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
