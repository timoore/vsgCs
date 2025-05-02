set(VCPKG_CXX_FLAGS_RELEASE "-g -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer")
set(VCPKG_C_FLAGS_RELEASE "-g -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer")

include("$ENV{VCPKG_ROOT}/triplets/x64-linux.cmake")
