cmake_minimum_required(VERSION 3.22)

option(VSGCS_USE_PROJ "Use the Proj library for coordinate system transformations" ON)

if(VSGCS_USE_PROJ)
  list(APPEND VCPKG_MANIFEST_FEATURES "proj")
endif()

# SSL works better on some systems with the system openssl library, in particular Fedora.
if(VSGCS_USE_SYSTEM_OPENSSL)
  set(VCPKG_OVERLAY_PORTS "extern/optional-overlays/openssl")
endif()

project(vsgCs
    VERSION 1.0.0
    DESCRIPTION "VSG library for rendering 3D Tiles and Cesium ion assets"
    LANGUAGES CXX C
)
set(VSGCS_SOVERSION 0)

# We have some custom .cmake scripts not in the official distribution.
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${vsgCs_SOURCE_DIR}/CMakeModules")

include("vsgCsMacros")

# set the use of C++20 as Cesium Native requires it
set(CMAKE_CXX_STANDARD 20)

if (MSVC)
  # template madness
  add_compile_options(/bigobj)
  # windows.h madness
  add_compile_definitions(NOGDI NOMINMAX)
endif()

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

option(RUN_CLANG_TIDY "run the clang-tidy checker" OFF)
set(VSGCS_CESIUM_NATIVE_DIRECTORY "" CACHE PATH "directory for Cesium Native source if not installed")

# Find Vulkan and the VSG
if (VULKAN_SDK)
    set(ENV{VULKAN_SDK} ${VULKAN_SDK})
endif()

set(VSG_MIN_VERSION 1.1.0)

find_package(vsg ${VSG_MIN_VERSION} REQUIRED)

find_package(vsgXchange)

set(VSGIMGUI_MIN_VERSION 0.1.0)

find_package(vsgImGui ${VSGIMGUI_MIN_VERSION} REQUIRED)

if(VSGCS_CESIUM_NATIVE_DIRECTORY)
  add_subdirectory(${VSGCS_CESIUM_NATIVE_DIRECTORY} cesium-native)
else()
  find_package(cesium-native REQUIRED)
endif()

if(VSGCS_USE_PROJ)
  find_package(PROJ REQUIRED)
endif()
find_package(imgui REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Microsoft.GSL REQUIRED)

if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

  find_program(CLANG_TIDY_EXECUTABLE clang-tidy)
  find_program(RUN_CLANG_TIDY_EXECUTABLE run-clang-tidy)

  if(CLANG_TIDY_EXECUTABLE AND RUN_CLANG_TIDY_EXECUTABLE)
    add_custom_target(run-clang-tidy
      COMMAND ${RUN_CLANG_TIDY_EXECUTABLE}
      -clang-tidy-binary ${CLANG_TIDY_EXECUTABLE}
      -p ${CMAKE_BINARY_DIR}
    )
  endif()
endif()

find_path(VALGRIND_H_PATH
  valgrind.h
  PATHS ${CMAKE_INCLUDE_PATH}/valgrind ${CMAKE_SYSTEM_INCLUDE_PATH}/valgrind)

message("valgrind path: ${VALGRIND_H_PATH}")

vsgCs_setup_build_vars()

include(GNUInstallDirs)

set(VSGCS_DATA_DIR "${CMAKE_INSTALL_DATADIR}/vsgCs")
set(VSGCS_FULL_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/vsgCs")

if(VSGCS_USE_PROJ)
  if(VCPKG_INSTALLED_DIR)
    option(VSGCS_INSTALL_PROJ_DATA "Install Proj data from vcpkg" ON)
    if(VSGCS_INSTALL_PROJ_DATA)
      set(VSGCS_FULL_PROJ_DATA_DIR "${VSGCS_FULL_DATA_DIR}/proj" CACHE PATH "Directory for Proj data")
      install(
        DIRECTORY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/proj"
        DESTINATION "${VSGCS_DATA_DIR}"
        PATTERN "*.cmake" EXCLUDE)
    endif()
  else()
    set(VSGCS_FULL_PROJ_DATA_DIR "" CACHE PATH "Directory for Proj data")
  endif()
endif()

if(WIN32)
  set(CMAKE_INSTALL_UCRT_LIBRARIES YES)

  include(InstallRequiredSystemLibraries)
endif()

# source directory for library and applications

add_subdirectory(src)
add_subdirectory(data)

option(BUILD_TRACY "build with tracy profiling and server" OFF)

if (BUILD_TRACY)
  option ( TRACY_ENABLE "" ON)
  option ( TRACY_ON_DEMAND "" ON)
  add_subdirectory(3rdparty/tracy)
  set_target_properties(TracyClient PROPERTIES COMPILE_WARNING_AS_ERROR OFF)
endif()

if(NOT VSGCS_USE_SYSTEM_OPENSSL AND VCPKG_INSTALLED_DIR)
  set(_SSLConfigDir "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/etc/ssl")
  if(IS_DIRECTORY "${_SSLConfigDir}")
    install(
      DIRECTORY "${_SSLConfigDir}"
      DESTINATION "${VSGCS_DATA_DIR}/etc")
  endif()
endif()

include(FeatureSummary)
feature_summary(WHAT ALL)

