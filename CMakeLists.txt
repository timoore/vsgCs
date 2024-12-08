cmake_minimum_required(VERSION 3.22)

project(vsgCs
    VERSION 0.9.0
    DESCRIPTION "VSG library for rendering 3D Tiles and Cesium ion assets"
    LANGUAGES CXX C
)
set(VSGCS_SOVERSION 0)
SET(VSGCS_RELEASE_CANDIDATE 0)

# We have some custom .cmake scripts not in the official distribution.
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${vsgCs_SOURCE_DIR}/CMakeModules")

include("vsgCsMacros")

# set the use of C++20 as Cesium Native requires it
set(CMAKE_CXX_STANDARD 20)

if (MSVC)
  # template madness
  add_compile_options(/bigobj)
  # windows.h madness
  add_compile_definitions(NOGDI)
endif()

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

option(RUN_CLANG_TIDY "run the clang-tidy checker" OFF)

# Find Vulkan and the VSG
if (VULKAN_SDK)
    set(ENV{VULKAN_SDK} ${VULKAN_SDK})
endif()

set(VSG_MIN_VERSION 1.1.0)

find_package(vsg ${VSG_MIN_VERSION} REQUIRED)

find_package(vsgXchange)

set(VSGIMGUI_MIN_VERSION 0.1.0)

find_package(vsgImGui ${VSGIMGUI_MIN_VERSION} REQUIRED)

find_package(cesium-native REQUIRED)

find_package(PROJ REQUIRED)
find_package(imgui REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(spdlog REQUIRED)

if (RUN_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
  if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=google-*,performance-*,portability-*,readability-*,modernize-*,-google-build-using-namespace,-readability-identifier-length,-readability-uppercase-literal-suffix,-google-readability-namespace-comments,-readability-named-parameter,-readability-implicit-bool-conversion,-readability-magic-numbers,-modernize-use-trailing-return-type)
  endif()
endif()

vsgCs_setup_build_vars()

include(GNUInstallDirs)

set(VSGCS_DATA_DIR "${CMAKE_INSTALL_DATADIR}/vsgCs")
set(VSGCS_FULL_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/vsgCs")

if(WIN32)
  set(CMAKE_INSTALL_UCRT_LIBRARIES YES)

  include(InstallRequiredSystemLibraries)
endif()

# source directory for library and applications

add_subdirectory(src)
add_subdirectory(data)

include(FeatureSummary)
feature_summary(WHAT ALL)

option(BUILD_TRACY "build with tracy profiling and server" OFF)

if (BUILD_TRACY)
  option ( TRACY_ENABLE "" ON)
  option ( TRACY_ON_DEMAND "" ON)
  add_subdirectory(3rdparty/tracy)
  set_target_properties(TracyClient PROPERTIES COMPILE_WARNING_AS_ERROR OFF)
endif()

