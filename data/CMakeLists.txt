set(IMAGE_FILES
  images/LDR_LLL1_0.png)
set(SHADER_FILES
  shaders/csstandard.vert
  shaders/cspoint.vert
  shaders/csstandard_pbr.frag
  shaders/descriptor_defs.glsl)

install(FILES ${IMAGE_FILES}
  DESTINATION ${VSGCS_DATA_DIR}/images)

install(FILES ${SHADER_FILES}
  DESTINATION ${VSGCS_DATA_DIR}/shaders)
