set(IMAGE_FILES
  images/Cesium_ion_256.png)
set(SHADER_FILES
  shaders/csstandard.vert
  shaders/cspoint.vert
  shaders/csstandard_pbr.frag)

install(FILES ${IMAGE_FILES}
  DESTINATION ${VSGCS_DATA_DIR}/images)

install(FILES ${SHADER_FILES}
  DESTINATION ${VSGCS_DATA_DIR}/shaders)
