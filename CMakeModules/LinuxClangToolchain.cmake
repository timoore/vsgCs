SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR x86_64)

SET(CMAKE_C_COMPILER /usr/bin/clang)
SET(CMAKE_CXX_COMPILER /usr/bin/clang++)
SET(CMAKE_AR /usr/bin/llvm-ar)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH on)

# This was borrowed from cesium-unreal, which has this comment:
#    These were deduced by scouring Unreal's LinuxToolChain.cs.
# It's not clear if the librarie arguments are necessary or useful.
SET(CMAKE_CXX_FLAGS "-target x86_64-unknown-linux-gnu")
SET(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld -target x86_64-unknown-linux-gnu -lm -lc -lpthread -lgcc_s -lgcc")
