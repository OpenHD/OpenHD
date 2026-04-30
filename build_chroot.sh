#!/bin/bash
################################################################################
# OpenHD
# 
# Licensed under the GNU General Public License (GPL) Version 3.
# 
# This software is provided "as-is," without warranty of any kind, express or 
# implied, including but not limited to the warranties of merchantability, 
# fitness for a particular purpose, and non-infringement. For details, see the 
# full license in the LICENSE file provided with this source code.
# 
# Non-Military Use Only:
# This software and its associated components are explicitly intended for 
# civilian and non-military purposes. Use in any military or defense 
# applications is strictly prohibited unless explicitly and individually 
# licensed otherwise by the OpenHD Team.
# 
# Contributors:
# A full list of contributors can be found at the OpenHD GitHub repository:
# https://github.com/OpenHD
# 
# © OpenHD, All Rights Reserved.
################################################################################

# Install script for CHROOT build using cloudsmith-cli to upload files

# Ensure /tmp has correct permissions
chmod 1777 /tmp || { echo "Failed to set permissions on /tmp"; exit 1; }

# Disable stale bullseye-backports entries that now 404
if grep -Rq "bullseye-backports" /etc/apt/sources.list /etc/apt/sources.list.d 2>/dev/null; then
    for source_file in /etc/apt/sources.list /etc/apt/sources.list.d/*.list; do
        [[ -f "$source_file" ]] && sed -i '/bullseye-backports/s/^/#/' "$source_file"
    done
fi

# Update package lists and install necessary packages as root
su -c "apt-get update --fix-missing && apt-get install -y sudo" || { echo "Failed to update and install sudo"; exit 1; }

# Install required packages for the script
apt-get install -y python3-pip git || { echo "Failed to install python3-pip and git"; exit 1; }

# Install or upgrade cloudsmith-cli
pip3 install --upgrade cloudsmith-cli || { echo "Failed to install cloudsmith-cli"; exit 1; }

# List all files in the current directory for debugging purposes
ls -a

# Load configuration variables from files and verify they exist
if [[ -f cloudsmith_api_key.txt && -f distro.txt && -f flavor.txt && -f repo.txt && -f custom.txt && -f arch.txt ]]; then
    API_KEY=$(cat cloudsmith_api_key.txt)
    DISTRO=$(cat distro.txt)
    FLAVOR=$(cat flavor.txt)
    REPO=$(cat repo.txt)
    CUSTOM=$(cat custom.txt)
    ARCH=$(cat arch.txt)
    QCOM=""
    if [[ -f qcom.txt ]]; then
        QCOM=$(tr -d '\r\n' < qcom.txt)
    fi
else
    echo "One or more required configuration files are missing."
    exit 1
fi

# Display loaded configuration for debugging
echo "Distro: ${DISTRO}"
echo "Flavor: ${FLAVOR}"
echo "Custom: ${CUSTOM}"
echo "Arch: ${ARCH}"
echo "Arch: ${QCOM}"

read_optional_file() {
    local file_path="$1"
    if [[ -f "${file_path}" ]]; then
        # Remove trailing CR/LF from file-based values.
        tr -d '\r\n' < "${file_path}"
    fi
}

ARTLINK_REPO_FILE_VALUE="$(read_optional_file artlink_repo.txt)"
if [[ -n "${ARTLINK_REPO_FILE_VALUE}" ]]; then
    export ARTLINK_REPO="${ARTLINK_REPO_FILE_VALUE}"
fi

ARTLINK_BRANCH_FILE_VALUE="$(read_optional_file artlink_branch.txt)"
if [[ -n "${ARTLINK_BRANCH_FILE_VALUE}" ]]; then
    export ARTLINK_BRANCH="${ARTLINK_BRANCH_FILE_VALUE}"
fi

ARTLINK_FETCH_MODE_FILE_VALUE="$(read_optional_file artlink_fetch_mode.txt)"
if [[ -n "${ARTLINK_FETCH_MODE_FILE_VALUE}" ]]; then
    export ARTLINK_FETCH_MODE="${ARTLINK_FETCH_MODE_FILE_VALUE}"
fi

ARTLINK_GIT_AUTH_FILE_VALUE="$(read_optional_file artlink_git_auth.txt)"
if [[ -n "${ARTLINK_GIT_AUTH_FILE_VALUE}" ]]; then
    export ARTLINK_GIT_AUTH="${ARTLINK_GIT_AUTH_FILE_VALUE}"
fi

ARTLINK_GIT_USERNAME_FILE_VALUE="$(read_optional_file artlink_git_username.txt)"
if [[ -n "${ARTLINK_GIT_USERNAME_FILE_VALUE}" ]]; then
    export ARTLINK_GIT_AUTH_USERNAME="${ARTLINK_GIT_USERNAME_FILE_VALUE}"
fi

ARTLINK_GIT_TOKEN_FILE_VALUE="$(read_optional_file artlink_git_token.txt)"
if [[ -n "${ARTLINK_GIT_TOKEN_FILE_VALUE}" ]]; then
    export ARTLINK_GIT_TOKEN="${ARTLINK_GIT_TOKEN_FILE_VALUE}"
fi

DOWNLOAD_URL_FILE_VALUE="$(read_optional_file download_url.txt)"
if [[ -n "${DOWNLOAD_URL_FILE_VALUE}" ]]; then
    export DOWNLOAD_URL="${DOWNLOAD_URL_FILE_VALUE}"
    export ARTLINK_DOWNLOAD_URL="${DOWNLOAD_URL_FILE_VALUE}"
fi

DOWNLOAD_KEY_FILE_VALUE="$(read_optional_file download_key.txt)"
if [[ -n "${DOWNLOAD_KEY_FILE_VALUE}" ]]; then
    export DOWNLOAD_KEY="${DOWNLOAD_KEY_FILE_VALUE}"
    export ARTLINK_DOWNLOAD_KEY="${DOWNLOAD_KEY_FILE_VALUE}"
fi

ARTOSYN_SDK_ARCHIVE_FILE_VALUE="$(read_optional_file artosyn_sdk_archive.txt)"
if [[ -n "${ARTOSYN_SDK_ARCHIVE_FILE_VALUE}" ]]; then
    export ARTOSYN_SDK_ARCHIVE="${ARTOSYN_SDK_ARCHIVE_FILE_VALUE}"
fi
if [[ -z "${ARTOSYN_SDK_ARCHIVE:-}" ]]; then
    for archive_candidate in ./artosyn_sdk.tar.gz ./artosyn_sdk.tgz ./artosyn_sdk.tar; do
        if [[ -f "${archive_candidate}" ]]; then
            export ARTOSYN_SDK_ARCHIVE="${archive_candidate}"
            break
        fi
    done
fi

OPENHD_REQUIRE_ARTOSYN_FILE_VALUE="$(read_optional_file openhd_require_artosyn.txt)"
if [[ -n "${OPENHD_REQUIRE_ARTOSYN_FILE_VALUE}" ]]; then
    export OPENHD_REQUIRE_ARTOSYN="${OPENHD_REQUIRE_ARTOSYN_FILE_VALUE}"
fi

OPENHD_REQUIRE_ARTOSYN_DAEMON_FILE_VALUE="$(read_optional_file openhd_require_artosyn_daemon.txt)"
if [[ -n "${OPENHD_REQUIRE_ARTOSYN_DAEMON_FILE_VALUE}" ]]; then
    export OPENHD_REQUIRE_ARTOSYN_DAEMON="${OPENHD_REQUIRE_ARTOSYN_DAEMON_FILE_VALUE}"
fi

if [[ "${REPO}" == "openhd-3.0" || "${REPO}" == "openhd-3.0-test" ]]; then
    # OpenHD 3.0 builds should produce packages with full Artosyn support.
    export OPENHD_REQUIRE_ARTOSYN="${OPENHD_REQUIRE_ARTOSYN:-1}"
    export OPENHD_REQUIRE_ARTOSYN_DAEMON="${OPENHD_REQUIRE_ARTOSYN_DAEMON:-1}"
fi

ensure_artosyn_cmake() {
    apt-get update --fix-missing
    apt-get install -y build-essential make gcc g++ libc6-dev || {
        echo "Failed to install compiler toolchain for Artosyn SDK build"
        exit 1
    }

    if python3 - <<'PY'
import re
import subprocess
import sys

try:
    output = subprocess.check_output(["cmake", "--version"], text=True)
except Exception:
    sys.exit(1)
match = re.search(r"version\s+(\d+)\.(\d+)", output)
if not match:
    sys.exit(1)
major, minor = map(int, match.groups())
sys.exit(0 if (major, minor) >= (3, 22) else 1)
PY
    then
        return 0
    fi

    echo "Installing newer CMake for Artosyn SDK build."
    python3 -m pip install --upgrade "cmake>=3.22,<4" || { echo "Failed to install newer CMake"; exit 1; }
    hash -r
    cmake --version
}

if [[ -n "${ARTLINK_REPO:-}" ]]; then
    echo "ArtLink repo override: ${ARTLINK_REPO}"
fi
if [[ -n "${ARTLINK_BRANCH:-}" ]]; then
    echo "ArtLink branch override: ${ARTLINK_BRANCH}"
fi
if [[ -n "${ARTLINK_FETCH_MODE:-}" ]]; then
    echo "ArtLink fetch mode: ${ARTLINK_FETCH_MODE}"
fi
if [[ -n "${ARTOSYN_SDK_ARCHIVE:-}" ]]; then
    echo "Artosyn SDK archive configured: ${ARTOSYN_SDK_ARCHIVE}"
fi
if [[ -n "${ARTLINK_GIT_AUTH:-}" || -n "${ARTLINK_GIT_TOKEN:-}" || -n "${DOWNLOAD_KEY:-}" ]]; then
    echo "Artosyn credentials: configured"
else
    echo "Artosyn credentials: not configured"
fi
if [[ -n "${OPENHD_REQUIRE_ARTOSYN:-}" ]]; then
    echo "OPENHD_REQUIRE_ARTOSYN=${OPENHD_REQUIRE_ARTOSYN}"
fi

if [[ "${OPENHD_REQUIRE_ARTOSYN:-0}" == "1" ]]; then
    ensure_artosyn_cmake
    source ./OpenHD/scripts/resolve_artosyn_sdk.sh
    resolve_artosyn_sdk
    if [[ -z "${ARTOSYN_SDK_ROOT:-}" || -z "${ARTOSYN_SDK_LIB:-}" ]]; then
        echo "Artosyn is required for this build target, but SDK resolution failed."
        exit 1
    fi
    if [[ "${OPENHD_REQUIRE_ARTOSYN_DAEMON:-0}" == "1" && -z "${ARTOSYN_SDK_DAEMON:-}" ]]; then
        echo "Artosyn daemon is required for this build target, but daemon resolution failed."
        exit 1
    fi
fi

# Install dependencies based on DISTRO or ARCH
if [[ "${QCOM}" == "coretronic" ]]; then

# Create a directory for the downloaded .deb files
mkdir -p poco_debs

# Clean the APT cache
apt-get clean

# Download only the required package without installing
apt-get --download-only install -y libpoco-dev

# Copy all .deb files to the poco_debs directory
cp /var/cache/apt/archives/*.deb poco_debs/

# Create a directory to extract the .deb files
mkdir -p poco_debs_extracted

# Extract all .deb files into the poco_debs_extracted directory
for deb in poco_debs/*.deb; do
    dpkg-deb -x "$deb" poco_debs_extracted/
done

# Create a tarball of the extracted files
tar -cvf poco_debs_extracted.tar poco_debs_extracted

# Rename the tarball with the .deb extension
cp poco_debs_extracted.tar /out/poco_debs.deb

# Cleanup (optional)
rm -rf poco_debs poco_debs_extracted poco_debs_extracted.tar
else
    if [[ "${ARCH}" == "arm64" ]]; then
        chmod +x ./install_build_dep.sh
        ./install_build_dep.sh rock5 || { echo "Failed to install build dependencies"; exit 1; }
    elif [[ "${DISTRO}" == "focal" ]]; then
        apt-get update || { echo "Failed to update and upgrade packages"; exit 1; }
        chmod +x ./install_build_dep.sh
        ./install_build_dep.sh rock5 || { echo "Failed to install build dependencies"; exit 1; }
        apt-get install -y libv4l-dev || { echo "Failed to install libv4l-dev"; exit 1; }
        gcc -v
        g++ -v
        cmake -v
    fi
    # Package the build using custom configurations
    chmod +x ./package.sh
    ./package.sh "${CUSTOM}" "${ARCH}" "${DISTRO}" "${FLAVOR}" || { echo "Packaging failed"; exit 1; }

    echo "Script execution completed successfully."
fi
