"""Failure-only ARM SDK evidence; does not change the build environment."""

import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import sys


ENVIRONMENT_KEYS = (
    "WindowsSdkDir", "WindowsSDKVersion", "WindowsSDKLibVersion",
    "UniversalCRTSdkDir", "UCRTVersion", "VCINSTALLDIR", "VCToolsVersion",
    "VCToolsInstallDir", "VSCMD_ARG_HOST_ARCH", "VSCMD_ARG_TGT_ARCH",
    "VSCMD_ARG_APP_PLAT",
    "LIB", "INCLUDE",
)
SDK_FILES = (
    "um/arm/kernel32.lib", "um/arm/WindowsApp.lib", "ucrt/arm/ucrt.lib",
    "um/x86/kernel32.lib", "um/x86/WindowsApp.lib", "ucrt/x86/ucrt.lib",
)


def environment_snapshot(environ, which=shutil.which):
    return {
        "variables": {name: environ.get(name) for name in ENVIRONMENT_KEYS},
        "tools": {name: which(name) for name in ("cl.exe", "link.exe", "rc.exe", "mt.exe")},
    }


def sdk_inventory(root):
    libraries = root / "Lib"
    try:
        versions = sorted(path for path in libraries.iterdir() if path.is_dir())
    except OSError as error:
        return {"root": str(root), "error": str(error), "versions": None}
    result = []
    for version in versions:
        files = {}
        for relative in SDK_FILES:
            path = version / relative
            try:
                metadata = path.stat()
                regular = stat.S_ISREG(metadata.st_mode)
                files[relative] = {"present": regular, "bytes": metadata.st_size if regular else None}
            except FileNotFoundError:
                files[relative] = {"present": False, "bytes": None}
            except OSError as error:
                files[relative] = {"present": None, "bytes": None, "error": str(error)}
        result.append({"version": version.name, "files": files})
    return {"root": str(root), "error": None, "versions": result}


def observe(label, invocation):
    print(label + ":", flush=True)
    try:
        result = subprocess.run(invocation, timeout=45, check=False)
    except (OSError, subprocess.TimeoutExpired) as error:
        print(label + " unavailable: " + str(error), flush=True)
        return 1
    print(label + " exit code: " + str(result.returncode), flush=True)
    return result.returncode


def main(arguments):
    if arguments == ["--environment"]:
        print(json.dumps(environment_snapshot(os.environ), sort_keys=True))
        return 0
    if arguments:
        raise SystemExit("Only --environment is supported")
    if os.environ.get("PLATFORM") != "ARM":
        return 0
    triplet = os.environ.get("TRIPLET")
    if triplet not in ("arm-windows", "arm-uwp"):
        raise SystemExit("Unexpected ARM triplet")
    print("ARM SDK diagnostics only; the preceding build failure remains authoritative.", flush=True)
    program_files = Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"))
    root = program_files / "Windows Kits" / "10"
    print(json.dumps(sdk_inventory(root), sort_keys=True), flush=True)
    print("Parent environment: " + json.dumps(environment_snapshot(os.environ), sort_keys=True), flush=True)
    # vcpkg env reconstructs its port-build environment in a child cmd process.
    command = 'call "{}" "{}" --environment'.format(sys.executable, Path(__file__).resolve())
    invocation = [r"C:\tools\vcpkg\vcpkg.exe", "env", "--triplet=" + triplet, command]
    current_status = observe("vcpkg environment for " + triplet, invocation)
    vcvars = program_files / "Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvarsall.bat"
    platform = " store" if triplet == "arm-uwp" else ""
    explicit = 'call "{}" amd64_arm{} 10.0.19041.0 && {}'.format(vcvars, platform, command)
    explicit_status = observe("Isolated explicit SDK19041 child (not a build fix)",
                              [os.environ.get("COMSPEC", "cmd.exe"), "/d", "/c", explicit])
    return current_status or explicit_status


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
