# Copied from vsgMacros.cmake, but we want different warning options
# and will set them up differently

macro(vsgCs_setup_build_vars)
    # Change the default build type to Release
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif(NOT CMAKE_BUILD_TYPE)

    if(CMAKE_COMPILER_IS_GNUCXX)
      set(VSGCS_WARNING_FLAGS -Wall -Wextra -Wpedantic  CACHE STRING "Compile flags to use.")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      set(VSGCS_WARNING_FLAGS -Wall -Wextra -Wparenthese -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wshadow -Wunused CACHE STRING "Compile flags to use.")
    else()
      set(VSGCS_WARNING_FLAGS CACHE STRING "Compile flags to use.")
    endif()

    add_compile_options(${VSGCS_WARNING_FLAGS})
endmacro()
