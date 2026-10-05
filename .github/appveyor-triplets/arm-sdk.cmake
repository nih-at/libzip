set(VCPKG_CMAKE_SYSTEM_VERSION 10.0.19041.0)
set(VCPKG_LOAD_VCVARS_ENV OFF)
# Track the environment initialized by appveyor-arm.cmd in package ABI keys.
set(VCPKG_ENV_PASSTHROUGH
    PATH INCLUDE LIB LIBPATH
    WindowsSdkDir WindowsSDKVersion WindowsSDKLibVersion
    UniversalCRTSdkDir UCRTVersion
    VCINSTALLDIR VCToolsVersion VCToolsInstallDir
    VSCMD_ARG_HOST_ARCH VSCMD_ARG_TGT_ARCH VSCMD_ARG_APP_PLAT
)
list(APPEND VCPKG_HASH_ADDITIONAL_FILES "${CMAKE_CURRENT_LIST_FILE}")
