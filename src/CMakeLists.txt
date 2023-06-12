add_subdirectory(vsgCs)
add_subdirectory(CsApp)
add_subdirectory(applications)

install(EXPORT vsgCsTargets
  FILE vsgCsTargets.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vsgCs)

install(FILES vsgCsConfig.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vsgCs)
