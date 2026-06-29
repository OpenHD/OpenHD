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

sanitize_radxa_sources_for_current_distro() {
    local codename=""
    codename="$(. /etc/os-release; echo "${VERSION_CODENAME:-}")"
    [[ -n "${codename}" ]] || return 0

    for source_file in /etc/apt/sources.list /etc/apt/sources.list.d/*.list /etc/apt/sources.list.d/*.sources; do
        [[ -f "$source_file" ]] || continue
        if grep -Eq 'dl\.cloudsmith\.io/public/openhd/.*/deb/debian' "$source_file"; then
            echo "Disabling stale OpenHD Cloudsmith apt source in ${source_file}"
            sed -i -E '/dl\.cloudsmith\.io\/public\/openhd\/.*\/deb\/debian/s/^[[:space:]]*deb/# deb/' "$source_file"
            sed -i -E '/URIs:.*dl\.cloudsmith\.io\/public\/openhd\/.*\/deb\/debian/,/^$/s/^/# /' "$source_file"
        fi
        if grep -Eq 'download\.vscodium\.com/debs' "$source_file"; then
            echo "Disabling VSCodium apt source in ${source_file}"
            sed -i -E '/download\.vscodium\.com\/debs/s/^[[:space:]]*deb/# deb/' "$source_file"
            sed -i -E '/URIs:.*download\.vscodium\.com\/debs/,/^$/s/^/# /' "$source_file"
        fi
        if grep -Eq 'radxa-repo\.github\.io' "$source_file"; then
            if grep -Eq '^[[:space:]]*deb[[:space:]]+(\[[^]]+\][[:space:]]+)?https?://radxa-repo\.github\.io/bullseye/?[[:space:]]+bullseye[[:space:]]' "$source_file"; then
                echo "Disabling stale Radxa plain Bullseye apt source in ${source_file}"
                sed -i -E '\|^[[:space:]]*deb[[:space:]]+(\[[^]]+\][[:space:]]+)?https?://radxa-repo\.github\.io/bullseye/?[[:space:]]+bullseye[[:space:]]|s/^[[:space:]]*deb/# deb/' "$source_file"
            fi
            if grep -Eq '^[[:space:]]*URIs:.*radxa-repo\.github\.io/bullseye/?' "$source_file" \
                && grep -Eq '^[[:space:]]*Suites:[[:space:]]+bullseye([[:space:]]|$)' "$source_file"; then
                echo "Disabling stale Radxa plain Bullseye deb822 apt source in ${source_file}"
                sed -i -E '/URIs:.*radxa-repo\.github\.io\/bullseye\/?/,/^$/s/^/# /' "$source_file"
            fi
            if grep -E '^[[:space:]]*deb[[:space:]].*radxa-repo\.github\.io' "$source_file" | grep -Ev 'signed-by=' >/dev/null; then
                echo "Disabling unsigned Radxa apt source in ${source_file}"
                sed -i -E '/radxa-repo\.github\.io/{/signed-by=/!s/^[[:space:]]*deb/# deb/}' "$source_file"
            fi
            if grep -Eq '^[[:space:]]*URIs:.*radxa-repo\.github\.io' "$source_file" \
                && ! grep -Eq '^[[:space:]]*Signed-By:' "$source_file"; then
                echo "Disabling unsigned Radxa deb822 apt source in ${source_file}"
                sed -i -E '/URIs:.*radxa-repo\.github\.io/,/^$/s/^/# /' "$source_file"
            fi
        fi
        if [[ "${codename}" == "bookworm" ]]; then
            if grep -Eq 'radxa-repo\.github\.io.*bullseye' "$source_file"; then
                echo "Disabling stale Bullseye apt source in ${source_file}"
                sed -i -E '/radxa-repo\.github\.io.*bullseye/s/^[[:space:]]*deb/# deb/' "$source_file"
                sed -i -E '/URIs:.*radxa-repo\.github\.io.*bullseye/,/^$/s/^/# /' "$source_file"
            fi
        elif [[ "${codename}" == "bullseye" ]]; then
            sed -i -E '/radxa-repo\.github\.io\/bookworm|radxa-repo\.github\.io\/rk3566-bookworm|radxa-repo\.github\.io[[:space:]]+bookworm|radxa-repo\.github\.io[[:space:]]+rk3566-bookworm/s/^[[:space:]]*deb/# deb/' "$source_file"
        fi
    done
}

ensure_debian_sources_for_current_distro() {
    local codename=""
    codename="$(. /etc/os-release; echo "${VERSION_CODENAME:-}")"
    [[ -n "${codename}" ]] || return 0

    mkdir -p /etc/apt/sources.list.d
    if grep -RqsE "^[[:space:]]*deb[[:space:]].*[[:space:]]${codename}[[:space:]-]" /etc/apt/sources.list /etc/apt/sources.list.d 2>/dev/null; then
        return 0
    fi

    echo "Adding minimal Debian ${codename} apt sources."
    if [[ "${codename}" == "bookworm" ]]; then
        cat >/etc/apt/sources.list.d/openhd-bookworm-ci.list <<'EOF'
deb http://deb.debian.org/debian bookworm main contrib non-free non-free-firmware
deb http://deb.debian.org/debian bookworm-updates main contrib non-free non-free-firmware
deb http://security.debian.org/debian-security bookworm-security main contrib non-free non-free-firmware
EOF
    elif [[ "${codename}" == "bullseye" ]]; then
        cat >/etc/apt/sources.list.d/openhd-bullseye-ci.list <<'EOF'
deb http://archive.debian.org/debian bullseye main contrib non-free
deb http://archive.debian.org/debian bullseye-updates main contrib non-free
deb https://security.debian.org/debian-security bullseye-security main contrib non-free
EOF
    fi
}

prepare_apt_space_for_ci() {
    echo "Preparing lean apt/dpkg state for CI package build."
    if [[ -f /var/lib/dpkg/info/radxa-sddm-theme.postrm ]]; then
        echo "Neutralizing broken radxa-sddm-theme postrm for CI cleanup."
        cat >/var/lib/dpkg/info/radxa-sddm-theme.postrm <<'EOF'
#!/bin/sh
exit 0
EOF
        chmod 755 /var/lib/dpkg/info/radxa-sddm-theme.postrm
    fi
    mkdir -p /usr/share/sddm/themes/breeze || true
    touch /usr/share/sddm/themes/breeze/Main.qml || true

    cat >/etc/apt/apt.conf.d/99openhd-ci-lean <<'EOF'
Acquire::Languages "none";
Acquire::IndexTargets::deb::Contents-deb::DefaultEnabled "false";
Acquire::IndexTargets::deb::DEP-11::DefaultEnabled "false";
Acquire::IndexTargets::deb::DEP-11-icons-small::DefaultEnabled "false";
Acquire::IndexTargets::deb::DEP-11-icons::DefaultEnabled "false";
Acquire::IndexTargets::deb::DEP-11-icons-hidpi::DefaultEnabled "false";
EOF
    mkdir -p /etc/dpkg/dpkg.cfg.d
    cat >/etc/dpkg/dpkg.cfg.d/99openhd-ci-lean <<'EOF'
path-exclude=/usr/share/doc/*
path-exclude=/usr/share/man/*
path-exclude=/usr/share/locale/*
path-include=/usr/share/doc/*/copyright
EOF
    apt-get clean || true
    rm -rf /var/cache/apt/archives/*.deb /var/cache/man/* /var/lib/apt/lists/* || true
    rm -rf /usr/share/doc/* /usr/share/man/* /usr/share/locale/* || true

    local purge_packages=""
    purge_packages="$(dpkg-query -W -f='${binary:Package}\n' 2>/dev/null \
        | grep -E '^(akonadi|baloo|calligra|kde|kio|kmail|konsole|kscreen|kwin|libreoffice|plasma|sddm|xorg|xserver-xorg)' \
        | tr '\n' ' ' || true)"
    if [[ -n "${purge_packages}" ]]; then
        apt-get purge -y ${purge_packages} || true
        apt-get autoremove -y || true
        apt-get clean || true
        rm -rf /var/cache/apt/archives/*.deb /var/cache/man/* /var/lib/apt/lists/* || true
    fi

    df -h /
}

sanitize_radxa_sources_for_current_distro
ensure_debian_sources_for_current_distro
prepare_apt_space_for_ci

# Update package lists and install necessary packages as root
su -c "apt-get -o Acquire::Check-Valid-Until=false update --fix-missing && apt-get install -y sudo" || { echo "Failed to update and install sudo"; exit 1; }

# Install required packages for the script
apt-get install -y python3-pip git || { echo "Failed to install python3-pip and git"; exit 1; }

if [[ -f skip_cloudsmith_cli_install.txt ]]; then
    SKIP_CLOUDSMITH_CLI_INSTALL="$(tr -d '\r\n' < skip_cloudsmith_cli_install.txt)"
    export SKIP_CLOUDSMITH_CLI_INSTALL
fi

if [[ "${SKIP_CLOUDSMITH_CLI_INSTALL:-0}" != "1" ]]; then
    # Install or upgrade cloudsmith-cli
    pip3 install --upgrade cloudsmith-cli \
        || pip3 install --upgrade cloudsmith-cli --break-system-packages \
        || { echo "Failed to install cloudsmith-cli"; exit 1; }
else
    echo "Skipping cloudsmith-cli install in chroot."
fi

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

free_chroot_space_for_ci() {
    echo "Freeing image space for CI package build."
    prepare_apt_space_for_ci
    if command -v mandb >/dev/null 2>&1 && [[ ! -e /usr/bin/mandb.distrib ]]; then
        dpkg-divert --local --rename --add /usr/bin/mandb || true
    fi
    if [[ -e /usr/bin/mandb.distrib ]]; then
        ln -sf /bin/true /usr/bin/mandb || true
    fi
    apt-get clean || true
    rm -rf /var/cache/apt/archives/*.deb /var/cache/man/* || true
    df -h /
}

ensure_artosyn_cmake() {
    free_chroot_space_for_ci
    apt-get update --fix-missing
    apt-get install -y build-essential make gcc g++ libc6-dev cmake ninja-build || {
        echo "Failed to install compiler toolchain for Artosyn SDK build"
        exit 1
    }
    apt-get clean || true
    rm -rf /var/cache/apt/archives/*.deb /var/cache/man/* || true

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
    python3 -m pip install --upgrade "cmake>=3.22,<4" \
        || python3 -m pip install --upgrade --break-system-packages "cmake>=3.22,<4" \
        || { echo "Failed to install newer CMake"; exit 1; }
    hash -r
    cmake --version
}

compact_artosyn_artifacts_for_ci() {
    [[ -n "${ARTOSYN_SDK_LIB:-}" ]] || return 0

    local artifact_dir="/opt/openhd-artosyn-artifacts"
    mkdir -p "${artifact_dir}"

    local client_lib=""
    local com_lib=""
    client_lib="$(printf '%s' "${ARTOSYN_SDK_LIB}" | tr ';' '\n' | grep 'libar8030_client\.a$' | head -n1 || true)"
    com_lib="$(printf '%s' "${ARTOSYN_SDK_LIB}" | tr ';' '\n' | grep 'libcom\.a$' | head -n1 || true)"

    if [[ -n "${client_lib}" && -f "${client_lib}" ]]; then
        cp -f "${client_lib}" "${artifact_dir}/libar8030_client.a"
        client_lib="${artifact_dir}/libar8030_client.a"
    fi
    if [[ -n "${com_lib}" && -f "${com_lib}" ]]; then
        cp -f "${com_lib}" "${artifact_dir}/libcom.a"
        com_lib="${artifact_dir}/libcom.a"
    fi
    if [[ -n "${client_lib}" && -n "${com_lib}" ]]; then
        export ARTOSYN_SDK_LIB="${client_lib};${com_lib}"
    fi

    if [[ -n "${ARTOSYN_SDK_DAEMON:-}" && -f "${ARTOSYN_SDK_DAEMON}" ]]; then
        cp -f "${ARTOSYN_SDK_DAEMON}" "${artifact_dir}/daemon"
        export ARTOSYN_SDK_DAEMON="${artifact_dir}/daemon"
    fi
    if [[ -n "${ARTOSYN_SDK_TUNTAP:-}" && -f "${ARTOSYN_SDK_TUNTAP}" ]]; then
        cp -f "${ARTOSYN_SDK_TUNTAP}" "${artifact_dir}/tuntap_bb"
        export ARTOSYN_SDK_TUNTAP="${artifact_dir}/tuntap_bb"
    fi

    rm -rf /tmp/openhd_artosyn_sdk_build_client \
           /tmp/openhd_artosyn_sdk_build_daemon \
           /tmp/openhd_artosyn_sdk_build_tuntap || true
    rm -rf /tmp/openhd_artosyn_sdk_repo/.git || true
    apt-get clean || true
    rm -rf /var/cache/apt/archives/*.deb /var/cache/man/* || true
    df -h /
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
    compact_artosyn_artifacts_for_ci
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
        free_chroot_space_for_ci
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
