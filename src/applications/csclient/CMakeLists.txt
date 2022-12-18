set(SOURCES
    csclient.cpp
)

SET(TARGET_SRC ${SOURCES})

INCLUDE_DIRECTORIES(${Vulkan_INCLUDE_DIR})

add_executable(csclient ${SOURCES})

target_link_libraries(csclient vsgCs vsg::vsg)

target_link_libraries(csclient vsgXchange::vsgXchange)

install(TARGETS csclient
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
