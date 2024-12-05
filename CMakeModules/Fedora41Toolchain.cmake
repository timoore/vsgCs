# Work around a bug in patchelf, which doesn't work with the ELF file arrangement in Fedora 41
set(VCPKG_LINKER_FLAGS "-Wl,-z,noseparate-code")

include("$ENV{VCPKG_ROOT}/scripts/toolchains/linux.cmake")
