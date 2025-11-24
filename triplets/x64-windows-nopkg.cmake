# Custom triplet that disables pkg-config to avoid MSYS2 download issues
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

# Disable pkg-config acquisition
set(VCPKG_POLICY_SKIP_PKGCONFIG_CHECK enabled)
set(VCPKG_CMAKE_SYSTEM_NAME Windows)
