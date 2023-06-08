include(CMakeFindDependencyMacro)

find_dependency(CURL)

include("${CMAKE_CURRENT_LIST_DIR}/vsgCsTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/vsgMacros.cmake")
