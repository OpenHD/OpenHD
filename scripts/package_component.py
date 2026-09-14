#!/usr/bin/env python3
"""
OpenHD Unified Component Packager
Builds and packages OpenHD for embedded target platforms (X21, Luckfox Pico, Luckfox Lyra).
Replaces ad-hoc component shell scripts with a single maintainable module.
"""

import argparse
import glob
import json
import os
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile
from datetime import datetime, timezone

PLATFORM_CONFIGS = {
    "x21": {
        "platform_id": "x21b",
        "arch_name": "aarch64",
        "expected_machine_regex": r"AArch64|ARM64",
        "defconfig": "configs/x21_defconfig",
        "package_prefix": "openhd-x21b",
        "manifest_extra": {
            "schema": 1,
            "component": "openhd",
            "platform": "x21b",
            "architecture": "aarch64",
        },
        "aliases": ["openhd-x21b-latest.tar.gz"],
        "libraries": [
            ("libPocoFoundation.so*", False),
            ("libPocoNet.so*", False),
            ("libPocoEncodings.so*", False),
            ("libpcap.so*", False),
            ("libsodium.so*", False),
            ("libusb-1.0.so*", False),
        ],
        "extra_cmake_args": [],
    },
    "luckfox": {
        "platform_id": "luckfox-pico",
        "arch_name": "armhf",
        "expected_machine_regex": r"ARM",
        "defconfig": "configs/luckfox_defconfig",
        "package_prefix": "openhd-luckfox-pico",
        "manifest_extra": {
            "schema": 1,
            "component": "openhd",
            "platform": "luckfox-pico",
            "targets": ["rv1103", "rv1106"],
            "architecture": "armhf",
        },
        "aliases": [
            "openhd-luckfox-pico-latest.tar.gz",
            "openhd-luckfox-rv1106-latest.tar.gz",
            "openhd-luckfox-rv1103-latest.tar.gz",
        ],
        "libraries": [
            ("libPocoFoundation.so*", False),
            ("libPocoNet.so*", False),
            ("libPocoEncodings.so*", True),
            ("libpcap.so*", False),
            ("libsodium.so*", False),
            ("libusb-1.0.so*", True),
            ("librockchip_mpp.so*", True),
            ("librga.so*", True),
            ("libatomic.so*", True),
        ],
        "extra_cmake_args": ["-DCMAKE_EXE_LINKER_FLAGS=-lstdc++fs"],
    },
    "lyra": {
        "platform_id": "luckfox-lyra",
        "arch_name": "armhf",
        "expected_machine_regex": r"ARM",
        "defconfig": "configs/lyra_defconfig",
        "package_prefix": "openhd-lyra-ground",
        "manifest_extra": {
            "schema": 1,
            "component": "openhd",
            "role": "ground",
            "air_enabled": False,
            "platform": "luckfox-lyra",
            "target": "rk3506",
            "architecture": "armhf",
        },
        "aliases": [
            "openhd-lyra-ground-latest.tar.gz",
            "openhd-rk3506-ground-latest.tar.gz",
        ],
        "libraries": [
            ("libPocoFoundation.so*", True),
            ("libPocoNet.so*", True),
            ("libPocoEncodings.so*", True),
            ("libpcap.so*", True),
            ("libsodium.so*", True),
            ("libusb-1.0.so*", True),
            ("libatomic.so*", True),
        ],
        "extra_cmake_args": [
            "-DCMAKE_CXX_FLAGS=-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard"
        ],
        "extra_files": "lyra_ground",
    },
}

LYRA_SERVICE_CONTENT = """[Unit]
Description=OpenHD Ground Station Module (Luckfox Lyra RK3506)
After=network.target

[Service]
Type=simple
ExecStart=/usr/bin/openhd -g
Restart=always
RestartSec=3
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
"""

LYRA_INSTALLER_CONTENT = """#!/bin/sh
set -eu
DEST_BIN="${DEST_BIN:-/usr/bin}"
DEST_LIB="${DEST_LIB:-/usr/lib}"
DIR="$(cd "$(dirname "$0")" && pwd)"

mkdir -p "${DEST_BIN}" "${DEST_LIB}"
install -m 0755 "${DIR}/usr/bin/openhd" "${DEST_BIN}/openhd"

if [ -d "${DIR}/usr/lib" ] && [ "$(ls -A "${DIR}/usr/lib" 2>/dev/null)" ]; then
  cp -af "${DIR}/usr/lib/"* "${DEST_LIB}/"
  ldconfig 2>/dev/null || true
fi

if [ -f "${DIR}/etc/systemd/system/openhd-ground.service" ] && [ -d /etc/systemd/system ]; then
  cp -f "${DIR}/etc/systemd/system/openhd-ground.service" /etc/systemd/system/
  systemctl daemon-reload 2>/dev/null || true
  systemctl enable openhd-ground.service 2>/dev/null || true
fi

echo "OpenHD Ground Module installed for Luckfox Lyra (RK3506)."
"""


def run_cmd(cmd, cwd=None, env=None, check=True):
    print(f"Executing: {' '.join(cmd) if isinstance(cmd, list) else cmd}")
    res = subprocess.run(cmd, shell=isinstance(cmd, str), cwd=cwd, env=env)
    if check and res.returncode != 0:
        sys.exit(res.returncode)
    return res


def get_version(repo_root):
    version_header = os.path.join(
        repo_root, "OpenHD/ohd_common/inc/openhd_global_constants.hpp"
    )
    major, minor, patch = "3", "0", "0"
    if os.path.exists(version_header):
        with open(version_header, "r", encoding="utf-8") as f:
            for line in f:
                if "MAJOR_VERSION =" in line:
                    major = line.split("=")[-1].strip().rstrip(";").strip()
                elif "MINOR_VERSION =" in line:
                    minor = line.split("=")[-1].strip().rstrip(";").strip()
                elif "PATCH_VERSION =" in line:
                    patch = line.split("=")[-1].strip().rstrip(";").strip()

    try:
        git_res = subprocess.run(
            ["git", "-C", repo_root, "rev-parse", "HEAD"],
            capture_output=True,
            text=True,
            check=True,
        )
        commit = git_res.stdout.strip()[:12]
    except Exception:
        commit = "unknown"

    openhd_ver = f"{major}.{minor}.{patch}-evo"
    package_ver = f"{openhd_ver}-{commit}"
    return openhd_ver, package_ver, commit


def resolve_sdk_env(sdk_dir):
    sdk_dir = os.path.abspath(sdk_dir)
    relocate_script = os.path.join(sdk_dir, "relocate-sdk.sh")
    if os.access(relocate_script, os.X_OK):
        print(f"Relocating SDK in {sdk_dir}...")
        subprocess.run([relocate_script], check=False)

    env = os.environ.copy()
    env_setup = os.path.join(sdk_dir, "environment-setup")
    if os.path.exists(env_setup):
        cmd = f"source '{env_setup}' && env"
        res = subprocess.run(cmd, shell=True, executable="/bin/bash", capture_output=True, text=True)
        if res.returncode == 0:
            for line in res.stdout.splitlines():
                if "=" in line:
                    k, v = line.split("=", 1)
                    env[k] = v

    toolchain_file = None
    candidate_tc = os.path.join(sdk_dir, "share/buildroot/toolchainfile.cmake")
    if os.path.exists(candidate_tc):
        toolchain_file = candidate_tc

    return sdk_dir, env, toolchain_file


def copy_library_family(search_roots, pattern, optional, stage_lib_dir):
    matches = []
    for root in search_roots:
        if not root or not os.path.exists(root):
            continue
        p1 = os.path.join(root, "usr/lib", pattern)
        p2 = os.path.join(root, "lib", pattern)
        p3 = os.path.join(root, "usr/lib/arm-linux-gnueabihf", pattern)
        matches.extend(glob.glob(p1))
        matches.extend(glob.glob(p2))
        matches.extend(glob.glob(p3))

    matches = sorted(list(set(matches)))
    if not matches:
        if not optional:
            print(f"Error: Missing required runtime library family: {pattern}", file=sys.stderr)
            sys.exit(1)
        return

    os.makedirs(stage_lib_dir, exist_ok=True)
    for match in matches:
        if os.path.islink(match):
            link_target = os.readlink(match)
            dest_file = os.path.join(stage_lib_dir, os.path.basename(match))
            if os.path.exists(dest_file):
                os.remove(dest_file)
            os.symlink(link_target, dest_file)
        else:
            shutil.copy2(match, stage_lib_dir)


def sha256_file(filepath):
    import hashlib
    h = hashlib.sha256()
    with open(filepath, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()


def main():
    parser = argparse.ArgumentParser(description="OpenHD Component Packager")
    parser.add_argument("--platform", required=True, choices=["x21", "luckfox", "lyra"], help="Target platform")
    parser.add_argument("--sdk", required=True, help="Path to platform SDK / sysroot")
    parser.add_argument("--out", required=True, help="Output directory for packaged component")
    args = parser.parse_args()

    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    cfg = PLATFORM_CONFIGS[args.platform]

    sdk_dir, env, toolchain_file = resolve_sdk_env(args.sdk)
    output_dir = os.path.abspath(args.out)
    os.makedirs(output_dir, exist_ok=True)

    # Initialize .config from defconfig if .config is missing
    dotconfig = os.path.join(repo_root, ".config")
    defconfig_path = os.path.join(repo_root, cfg["defconfig"])
    if not os.path.exists(dotconfig) and os.path.exists(defconfig_path):
        print(f"Copying {cfg['defconfig']} -> .config")
        shutil.copy2(defconfig_path, dotconfig)

    # Ensure autoconf.h and kconfig.cmake are generated
    kconfig_py = os.path.join(repo_root, "scripts/kconfig.py")
    if os.path.exists(kconfig_py):
        run_cmd([sys.executable, kconfig_py, "--kconfig", os.path.join(repo_root, "Kconfig"), "--config", dotconfig], cwd=repo_root)

    with tempfile.TemporaryDirectory(prefix=f"openhd_build_{args.platform}_") as work_dir:
        build_dir = os.path.join(work_dir, "build")
        stage_dir = os.path.join(work_dir, "component")
        stage_bin = os.path.join(stage_dir, "usr/bin")
        stage_lib = os.path.join(stage_dir, "usr/lib")
        os.makedirs(stage_bin, exist_ok=True)
        os.makedirs(stage_lib, exist_ok=True)

        cmake_cmd = ["cmake", "-S", os.path.join(repo_root, "OpenHD"), "-B", build_dir, "-DCMAKE_BUILD_TYPE=Release", "-DBUILD_SHARED_LIBS=OFF"]
        if toolchain_file:
            cmake_cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}")
        elif os.path.exists(os.path.join(repo_root, "OpenHD/cmake/portable-linux-toolchain.cmake")):
            env["OPENHD_SYSROOT"] = sdk_dir
            env["OPENHD_CROSS_TRIPLET"] = "aarch64-linux-gnu" if cfg["arch_name"] == "aarch64" else "arm-linux-gnueabihf"
            cmake_cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={os.path.join(repo_root, 'OpenHD/cmake/portable-linux-toolchain.cmake')}")
        else:
            cmake_cmd.extend([
                "-DCMAKE_SYSTEM_NAME=Linux",
                f"-DCMAKE_SYSTEM_PROCESSOR={'aarch64' if cfg['arch_name'] == 'aarch64' else 'arm'}",
                f"-DCMAKE_SYSROOT={sdk_dir}",
                f"-DCMAKE_FIND_ROOT_PATH={sdk_dir}",
            ])

        # Auto-detect ccache
        ccache_bin = shutil.which("ccache")
        if ccache_bin:
            cmake_cmd.extend([f"-DCMAKE_CXX_COMPILER_LAUNCHER={ccache_bin}", f"-DCMAKE_C_COMPILER_LAUNCHER={ccache_bin}"])

        cmake_cmd.extend(cfg["extra_cmake_args"])
        run_cmd(cmake_cmd, cwd=repo_root, env=env)

        nproc = str(os.cpu_count() or 4)
        run_cmd(["cmake", "--build", build_dir, "--parallel", nproc, "--target", "openhd"], cwd=repo_root, env=env)

        # Copy executable to stage
        target_bin = os.path.join(build_dir, "openhd")
        dest_bin = os.path.join(stage_bin, "openhd")
        shutil.copy2(target_bin, dest_bin)

        strip_cmd = env.get("STRIP") or shutil.which(f"{env.get('CROSS_COMPILE', '')}strip") or shutil.which("strip")
        if strip_cmd:
            subprocess.run([strip_cmd, dest_bin], check=False)

        # Copy runtime libraries
        staging_dir = env.get("STAGING_DIR") or sdk_dir
        for pattern, is_opt in cfg["libraries"]:
            copy_library_family([staging_dir, sdk_dir], pattern, is_opt, stage_lib)

        # Add extra files for specific platforms
        if cfg.get("extra_files") == "lyra_ground":
            svc_dir = os.path.join(stage_dir, "etc/systemd/system")
            os.makedirs(svc_dir, exist_ok=True)
            with open(os.path.join(svc_dir, "openhd-ground.service"), "w", encoding="utf-8") as f:
                f.write(LYRA_SERVICE_CONTENT)
            inst_script = os.path.join(stage_dir, "install-lyra-ground.sh")
            with open(inst_script, "w", encoding="utf-8") as f:
                f.write(LYRA_INSTALLER_CONTENT)
            os.chmod(inst_script, 0o755)

        # Generate manifest
        openhd_ver, package_ver, commit = get_version(repo_root)
        manifest_data = dict(cfg["manifest_extra"])
        manifest_data.update({
            "component_version": openhd_ver,
            "package_version": package_ver,
            "source_commit": commit,
            "generated_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        })

        manifest_path = os.path.join(stage_dir, "component-manifest.json")
        with open(manifest_path, "w", encoding="utf-8") as f:
            json.dump(manifest_data, f, indent=2)

        # Generate sha256sums inside stage_dir
        sums_path = os.path.join(stage_dir, "sha256sums")
        with open(sums_path, "w", encoding="utf-8") as f_out:
            for root, _, files in os.walk(stage_dir):
                for file in sorted(files):
                    if file == "sha256sums":
                        continue
                    full_p = os.path.join(root, file)
                    rel_p = os.path.relpath(full_p, stage_dir)
                    f_hash = sha256_file(full_p)
                    f_out.write(f"{f_hash}  ./{rel_p}\n")

        # Package into tar.gz
        tar_name = f"{cfg['package_prefix']}-{package_ver}.tar.gz"
        final_tar_path = os.path.join(output_dir, tar_name)
        final_manifest_path = os.path.join(output_dir, f"{tar_name}.manifest.json")

        with tarfile.open(final_tar_path, "w:gz") as tar:
            tar.add(stage_dir, arcname=".")

        shutil.copy2(manifest_path, final_manifest_path)

        with open(f"{final_tar_path}.sha256", "w", encoding="utf-8") as f:
            f.write(f"{sha256_file(final_tar_path)}  {tar_name}\n")

        # Create aliases
        for alias in cfg["aliases"]:
            alias_tar = os.path.join(output_dir, alias)
            alias_manifest = os.path.join(output_dir, f"{alias}.manifest.json")
            alias_sha = os.path.join(output_dir, f"{alias}.sha256")

            shutil.copy2(final_tar_path, alias_tar)
            shutil.copy2(final_manifest_path, alias_manifest)
            with open(alias_sha, "w", encoding="utf-8") as f:
                f.write(f"{sha256_file(alias_tar)}  {alias}\n")

        print(f"\nSuccessfully built and packaged {cfg['platform_id']} component into {final_tar_path}")

if __name__ == "__main__":
    main()
