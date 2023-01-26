set(SOURCES
  csclient.cpp
  UI.cpp
)

SET(TARGET_SRC ${SOURCES})

INCLUDE_DIRECTORIES(${Vulkan_INCLUDE_DIR})

add_executable(csclient ${SOURCES})

target_link_libraries(csclient vsgCs CsApp vsg::vsg)

target_link_libraries(csclient vsgImGui::vsgImGui)

if (vsgXchange_FOUND)
  target_compile_definitions(csclient PRIVATE vsgXchange_FOUND)
  target_link_libraries(csclient vsgXchange::vsgXchange)
endif()

install(TARGETS csclient
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
