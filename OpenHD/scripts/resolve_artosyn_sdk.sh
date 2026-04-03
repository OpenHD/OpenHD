#!/bin/bash

# Resolve private Artosyn SDK location without committing SDK sources to this repo.
# Expected usage:
#   source scripts/resolve_artosyn_sdk.sh
#   resolve_artosyn_sdk
#   cmake ... -DARTOSYN_SDK_ROOT="${ARTOSYN_SDK_ROOT}" -DARTOSYN_SDK_LIB="${ARTOSYN_SDK_LIB}"

ARTLINK_REPO_DEFAULT="https://github.com/OpenHD-Technologies/OpenHD-ArtLink.git"
ARTLINK_REPO=${ARTLINK_REPO:-${ARTLINK_REPO_DEFAULT}}
ARTLINK_BRANCH=${ARTLINK_BRANCH:-main}
ARTLINK_REPO_DIR=${ARTLINK_REPO_DIR:-OpenHD-ArtLink}

# Reuse the kernel-builder/OpenHD secret contract when present.
ARTLINK_DOWNLOAD_URL=${ARTLINK_DOWNLOAD_URL:-${DOWNLOAD_URL:-}}
ARTLINK_DOWNLOAD_KEY=${ARTLINK_DOWNLOAD_KEY:-${DOWNLOAD_KEY:-}}
ARTLINK_GIT_AUTH_USERNAME=${ARTLINK_GIT_AUTH_USERNAME:-raphael@openhdfpv.org}
ARTLINK_GIT_TOKEN=${ARTLINK_GIT_TOKEN:-${OPENHD_SUBMODULE_TOKEN:-}}
ARTLINK_GIT_AUTH=${ARTLINK_GIT_AUTH:-}

# Prefer dedicated git token for private repo clone.
if [[ -z "${ARTLINK_GIT_AUTH}" && -n "${ARTLINK_GIT_TOKEN}" ]]; then
  ARTLINK_GIT_AUTH="${ARTLINK_GIT_AUTH_USERNAME}:${ARTLINK_GIT_TOKEN}"
fi

# Fallback to DOWNLOAD_KEY only when no explicit git auth is available.
if [[ -z "${ARTLINK_GIT_AUTH}" && -n "${ARTLINK_DOWNLOAD_KEY}" ]]; then
  ARTLINK_GIT_AUTH="${ARTLINK_DOWNLOAD_KEY}"
fi

_artlink_git_auth_basic() {
  if [[ -z "${ARTLINK_GIT_AUTH}" ]]; then
    return 1
  fi
  local raw_auth
  if [[ "${ARTLINK_GIT_AUTH}" == *:* ]]; then
    raw_auth="${ARTLINK_GIT_AUTH}"
  else
    raw_auth="x-access-token:${ARTLINK_GIT_AUTH}"
  fi
  printf '%s' "${raw_auth}" | base64 | tr -d '\n'
}

_artlink_git() {
  local auth_b64
  auth_b64=$(_artlink_git_auth_basic) || true
  if [[ -n "${auth_b64}" && "${ARTLINK_REPO}" == https://github.com/* ]]; then
    GIT_TERMINAL_PROMPT=0 git -c http.https://github.com/.extraheader="AUTHORIZATION: basic ${auth_b64}" "$@"
  else
    GIT_TERMINAL_PROMPT=0 git "$@"
  fi
}

_extract_archive() {
  local archive="$1"
  local extract_dir="$2"

  rm -rf "${extract_dir}" || return 1
  mkdir -p "${extract_dir}" || return 1

  if tar -xf "${archive}" -C "${extract_dir}" >/dev/null 2>&1; then
    return 0
  fi
  if command -v unzip >/dev/null 2>&1; then
    unzip -q "${archive}" -d "${extract_dir}" || return 1
    return 0
  fi
  echo "Unable to extract archive (${archive}). Provide a tar.* archive or install unzip." >&2
  return 1
}

_is_archive_url() {
  local url="$1"
  local lower
  lower="$(printf '%s' "${url}" | tr '[:upper:]' '[:lower:]')"
  case "${lower}" in
    *.tar|*.tar.gz|*.tgz|*.tar.xz|*.txz|*.zip|*.7z)
      return 0
      ;;
  esac
  # Common query-parameter style download links
  if [[ "${lower}" == *".tar?"* || "${lower}" == *".tar.gz?"* || \
        "${lower}" == *".tgz?"* || "${lower}" == *".zip?"* || \
        "${lower}" == *"format=tar"* || "${lower}" == *"format=zip"* ]]; then
    return 0
  fi
  return 1
}

_find_sdk_root() {
  local search_root="$1"
  if [[ -d "${search_root}/host_drv/app/ar8030" && -d "${search_root}/host_drv/com" ]]; then
    echo "${search_root}"
    return 0
  fi
  local hit
  hit="$(find "${search_root}" -type d -path "*/host_drv/app/ar8030" | head -n 1 || true)"
  if [[ -z "${hit}" ]]; then
    return 1
  fi
  # .../<sdk-root>/host_drv/app/ar8030 -> sdk-root
  echo "$(dirname "$(dirname "$(dirname "${hit}")")")"
}

_fetch_from_download_url() {
  if [[ -z "${ARTLINK_DOWNLOAD_URL}" ]]; then
    return 1
  fi
  # If DOWNLOAD_URL points to a git repo (not an archive), let git path handle it.
  if ! _is_archive_url "${ARTLINK_DOWNLOAD_URL}"; then
    return 1
  fi
  echo "[Artosyn] Trying archive fetch path (DOWNLOAD_URL archive)." >&2
  local fetch_root="/tmp/openhd_artosyn_sdk_fetch"
  local archive_path="${fetch_root}/artlink-source.archive"
  local extract_dir="${fetch_root}/extract"

  rm -rf "${fetch_root}" || return 1
  mkdir -p "${fetch_root}" || return 1

  if [[ -n "${ARTLINK_DOWNLOAD_KEY}" ]]; then
    curl --fail --location --retry 3 -u "${ARTLINK_DOWNLOAD_KEY}" --output "${archive_path}" "${ARTLINK_DOWNLOAD_URL}" || return 1
  else
    curl --fail --location --retry 3 --output "${archive_path}" "${ARTLINK_DOWNLOAD_URL}" || return 1
  fi

  _extract_archive "${archive_path}" "${extract_dir}" || return 1
  _find_sdk_root "${extract_dir}" || return 1
}

_fetch_from_git() {
  echo "[Artosyn] Trying git clone path for private ArtLink SDK." >&2
  local repo_root="/tmp/openhd_artosyn_sdk_repo/${ARTLINK_REPO_DIR}"
  rm -rf "${repo_root}" || return 1
  mkdir -p "$(dirname "${repo_root}")" || return 1

  _artlink_git clone "${ARTLINK_REPO}" "${repo_root}" || return 1
  if [[ -n "${ARTLINK_BRANCH}" && "${ARTLINK_BRANCH}" != "latest" ]]; then
    pushd "${repo_root}" >/dev/null || return 1
      _artlink_git fetch --all --tags --prune || return 1
      _artlink_git checkout -f "${ARTLINK_BRANCH}" || return 1
    popd >/dev/null || return 1
  fi
  _find_sdk_root "${repo_root}" || return 1
}

_build_client_lib_from_source() {
  local sdk_root="$1"
  local host_drv_dir="${sdk_root}/host_drv"
  if [[ ! -f "${host_drv_dir}/CMakeLists.txt" ]]; then
    return 1
  fi
  if ! command -v cmake >/dev/null 2>&1; then
    return 1
  fi

  local build_dir="/tmp/openhd_artosyn_sdk_build"
  rm -rf "${build_dir}" || return 1
  echo "[Artosyn] libar8030_client missing, building ar8030_client from source." >&2

  cmake -S "${host_drv_dir}" -B "${build_dir}" \
    -DAPP_STATIC_LIB=ON \
    -DBUILD_TEST_APP=OFF \
    -DBUILD_ARTOSYN_EXAMPLE=OFF \
    -DBUILD_RAM_INIT=OFF \
    -DBUILD_TUNTAP=OFF \
    -DBUILD_BW_UPDATE_DEMO=OFF \
    -DBUILD_IMG_UPGRADE=OFF \
    -DBUILD_XDATA_TEST=OFF \
    -DBUILD_REPEATER_TEST=OFF \
    -DBUILD_BB_TEST=OFF \
    -DBUILD_WORK_MODE_CFG=OFF \
    -DBUILD_NET_DEV_DEMO=OFF \
    -DENABLE_PYTHON=OFF \
    -DENABLE_JAVA=OFF \
    -DUSING_8030USB=OFF \
    -DUSING_8030SDIO=OFF \
    -DUSING_8030UART=ON \
    -DUSING_8030DRV=OFF >/dev/null || return 1

  cmake --build "${build_dir}" --target ar8030_client >/dev/null || return 1

  local built_lib
  built_lib="$(find "${build_dir}" -type f -name "libar8030_client.a" | head -n 1 || true)"
  if [[ -z "${built_lib}" ]]; then
    built_lib="$(find "${build_dir}" -type f -name "libar8030_client.so" | head -n 1 || true)"
  fi
  if [[ -z "${built_lib}" ]]; then
    return 1
  fi
  echo "${built_lib}"
}

resolve_artosyn_sdk() {
  local sdk_root="${ARTOSYN_SDK_ROOT:-}"
  local sdk_lib="${ARTOSYN_SDK_LIB:-}"
  local fetch_mode="${ARTLINK_FETCH_MODE:-auto}"
  echo "[Artosyn] Resolving SDK (mode=${fetch_mode})." >&2

  # Kernel-builder secret reuse:
  # If DOWNLOAD_URL is actually a git repository URL, use it as the ArtLink repo.
  if [[ -n "${ARTLINK_DOWNLOAD_URL}" && "${ARTLINK_REPO}" == "${ARTLINK_REPO_DEFAULT}" ]]; then
    if ! _is_archive_url "${ARTLINK_DOWNLOAD_URL}"; then
      ARTLINK_REPO="${ARTLINK_DOWNLOAD_URL}"
    fi
  fi

  # Optional archive injection for CI/private builders.
  # If set, extract archive into /tmp/openhd_artosyn_sdk and use it as root.
  local sdk_archive="${ARTOSYN_SDK_ARCHIVE:-}"
  if [[ -z "${sdk_root}" && -n "${sdk_archive}" ]]; then
    if [[ ! -f "${sdk_archive}" ]]; then
      echo "ARTOSYN_SDK_ARCHIVE points to missing file: ${sdk_archive}" >&2
      return 1
    fi
    local extract_root="/tmp/openhd_artosyn_sdk"
    rm -rf "${extract_root}"
    mkdir -p "${extract_root}"
    case "${sdk_archive}" in
      *.tar.gz|*.tgz)
        tar -xzf "${sdk_archive}" -C "${extract_root}"
        ;;
      *.tar)
        tar -xf "${sdk_archive}" -C "${extract_root}"
        ;;
      *)
        echo "Unsupported ARTOSYN_SDK_ARCHIVE format: ${sdk_archive}" >&2
        echo "Supported: .tar, .tar.gz, .tgz" >&2
        return 1
        ;;
    esac
    # Assume archive either contains SDK root directly or one top-level dir.
    if [[ -d "${extract_root}/host_drv" ]]; then
      sdk_root="${extract_root}"
    else
      local first_dir
      first_dir="$(find "${extract_root}" -mindepth 1 -maxdepth 1 -type d | head -n1 || true)"
      if [[ -n "${first_dir}" ]]; then
        sdk_root="${first_dir}"
      fi
    fi
  fi

  # Common private locations (outside repo) for local/chroot/CI builders.
  if [[ -z "${sdk_root}" ]]; then
    local candidates=(
      "${ARTLINK_SOURCE_DIR:-}"
      "${OPENHD_KERNEL_BUILDER_DIR:-}/workdir/mods/${ARTLINK_REPO_DIR}"
      "${OPENHD_KERNEL_BUILDER_DIR:-}/workdir/mods/OpenHD-ArtLink"
      "/opt/openhd-private/artosyn_sdk"
      "/opt/openhd/artosyn_sdk"
      "/opt/artosyn_sdk"
      "/usr/local/share/openhd/artosyn_sdk"
    )
    local candidate
    for candidate in "${candidates[@]}"; do
      if [[ -z "${candidate}" ]]; then
        continue
      fi
      local resolved
      resolved="$(_find_sdk_root "${candidate}" || true)"
      if [[ -n "${resolved}" ]]; then
        echo "[Artosyn] Found SDK in local candidate path." >&2
        sdk_root="${resolved}"
        break
      fi
    done
  fi

  # Prefer git clone + build (same model as kernel-builder), then fallback to
  # download URL only if needed.
  if [[ -z "${sdk_root}" ]]; then
    case "${fetch_mode}" in
      git-first|auto|"")
        sdk_root="$(_fetch_from_git || true)"
        if [[ -z "${sdk_root}" ]]; then
          sdk_root="$(_fetch_from_download_url || true)"
        fi
        ;;
      download-first)
        sdk_root="$(_fetch_from_download_url || true)"
        if [[ -z "${sdk_root}" ]]; then
          sdk_root="$(_fetch_from_git || true)"
        fi
        ;;
      git-only)
        sdk_root="$(_fetch_from_git || true)"
        ;;
      download-only)
        sdk_root="$(_fetch_from_download_url || true)"
        ;;
      *)
        echo "Unknown ARTLINK_FETCH_MODE=${fetch_mode} (use git-first, download-first, git-only, download-only)" >&2
        ;;
    esac
  fi

  if [[ -n "${sdk_root}" && -z "${sdk_lib}" ]]; then
    local lib_candidates=(
      "${sdk_root}/host_drv/app/ar8030/libar8030_client.a"
      "${sdk_root}/host_drv/app/ar8030/libar8030_client.so"
      "${sdk_root}/lib/libar8030_client.a"
      "${sdk_root}/lib/libar8030_client.so"
      "${sdk_root}/libar8030_client.a"
      "${sdk_root}/libar8030_client.so"
    )
    local lib_candidate
    for lib_candidate in "${lib_candidates[@]}"; do
      if [[ -f "${lib_candidate}" ]]; then
        sdk_lib="${lib_candidate}"
        break
      fi
    done
  fi

  if [[ -n "${sdk_root}" && -z "${sdk_lib}" ]]; then
    sdk_lib="$(_build_client_lib_from_source "${sdk_root}" || true)"
  fi

  if [[ -z "${sdk_root}" || -z "${sdk_lib}" ]]; then
    cat >&2 <<'EOF'
Unable to resolve private Artosyn SDK.
Set one of:
  - ARTOSYN_SDK_ROOT + ARTOSYN_SDK_LIB
  - ARTOSYN_SDK_ARCHIVE (tar/tar.gz/tgz containing the SDK)
  - DOWNLOAD_URL (+ optional DOWNLOAD_KEY) as used by kernel builder
  - OPENHD_SUBMODULE_TOKEN / ARTLINK_GIT_AUTH for private ArtLink git access
Or place SDK under one of:
  /opt/openhd-private/artosyn_sdk
  /opt/openhd/artosyn_sdk
  /opt/artosyn_sdk
  /usr/local/share/openhd/artosyn_sdk
If running via sudo, preserve env (e.g. sudo -E ...).
EOF
    return 1
  fi

  echo "[Artosyn] SDK resolved: root=${ARTOSYN_SDK_ROOT:-${sdk_root}}" >&2
  echo "[Artosyn] SDK lib resolved: ${ARTOSYN_SDK_LIB:-${sdk_lib}}" >&2

  export ARTOSYN_SDK_ROOT="${sdk_root}"
  export ARTOSYN_SDK_LIB="${sdk_lib}"
}
