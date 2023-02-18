# Copied from vsgMacros.cmake, but we want different warning options
# and will set them up differently

macro(vsgCs_setup_build_vars)
    set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
    set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
    set(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
    set(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")

    # Change the default build type to Release
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif(NOT CMAKE_BUILD_TYPE)

    if(CMAKE_COMPILER_IS_GNUCXX)
      set(VSGCS_WARNING_FLAGS -Wall -Wextra -Wpedantic  CACHE STRING "Compile flags to use.")
      set(CMAKE_COMPILE_WARNING_AS_ERROR YES CACHE BOOL "Treat warnings as errors")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      set(VSGCS_WARNING_FLAGS -Wall -Wextra -Wparenthese -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wshadow -Wunused CACHE STRING "Compile flags to use.")
      set(CMAKE_COMPILE_WARNING_AS_ERROR YES CACHE BOOL "Treat warnings as errors")
    else()
      set(VSGCS_WARNING_FLAGS CACHE STRING "Compile flags to use.")
      set(CMAKE_COMPILE_WARNING_AS_ERROR NO CACHE BOOL "Treat warnings as errors")
    endif()

    add_compile_options(${VSGCS_WARNING_FLAGS})

    # set upper case <PROJECT>_VERSION_... variables
    string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)
    set(${UPPER_PROJECT_NAME}_VERSION ${PROJECT_VERSION})
    set(${UPPER_PROJECT_NAME}_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(${UPPER_PROJECT_NAME}_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(${UPPER_PROJECT_NAME}_VERSION_PATCH ${PROJECT_VERSION_PATCH})
endmacro()
