#!/bin/bash

# Resolve the Artosyn L4 SDK location without committing SDK sources to this repo.
# Expected usage:
#   source scripts/resolve_artosyn_sdk.sh
#   resolve_artosyn_sdk
#   cmake ... -DARTOSYN_SDK_ROOT="${ARTOSYN_SDK_ROOT}" -DARTOSYN_SDK_LIB="${ARTOSYN_SDK_LIB}"
#          -DARTOSYN_SDK_DAEMON="${ARTOSYN_SDK_DAEMON}"
#          -DARTOSYN_SDK_TUNTAP="${ARTOSYN_SDK_TUNTAP}"

ARTLINK_RESOLVER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENHD_REPO_ROOT="$(cd "${ARTLINK_RESOLVER_DIR}/../.." && pwd)"
ARTOSYN_WORK_ARCH=${OPENHD_CROSS_TRIPLET:-native}
ARTOSYN_WORK_ROOT=${OPENHD_ARTOSYN_WORK_ROOT:-${OPENHD_REPO_ROOT}/out/build/artosyn-sdk/${ARTOSYN_WORK_ARCH}}

ARTLINK_REPO_DEFAULT="https://github.com/KUTIAN-VT/L4_Linux_SDK.git"
ARTLINK_REPO=${ARTLINK_REPO:-${ARTLINK_REPO_DEFAULT}}
ARTLINK_BRANCH=${ARTLINK_BRANCH:-main}
ARTLINK_REPO_DIR=${ARTLINK_REPO_DIR:-L4_Linux_SDK}

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

_artlink_cmake_configure() {
  local toolchain_args=()
  if [[ -n "${OPENHD_CMAKE_TOOLCHAIN_FILE:-}" ]]; then
    toolchain_args+=(
      -DCMAKE_TOOLCHAIN_FILE="${OPENHD_CMAKE_TOOLCHAIN_FILE}"
    )
  fi
  cmake "$@" "${toolchain_args[@]}"
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
  if [[ -z "${search_root}" || ! -d "${search_root}" ]]; then
    return 1
  fi
  if [[ -d "${search_root}/host_drv/app/ar8030" && \
        ( -d "${search_root}/host_drv/com" || -d "${search_root}/host_drv/install/include" ) ]]; then
    echo "${search_root}"
    return 0
  fi
  if [[ -f "${search_root}/CMakeLists.txt" && \
        -d "${search_root}/app/ar8030" && -d "${search_root}/com" && \
        -d "${search_root}/daemon" ]]; then
    echo "${search_root}"
    return 0
  fi
  local hit
  while IFS= read -r hit; do
    local candidate_root
    candidate_root="$(dirname "$(dirname "$(dirname "${hit}")")")"
    if [[ -d "${candidate_root}/host_drv/app/ar8030" && \
          ( -d "${candidate_root}/host_drv/com" || -d "${candidate_root}/host_drv/install/include" ) ]]; then
      echo "${candidate_root}"
      return 0
    fi
  done < <(find "${search_root}" -type d \
    \( -path "*/host_drv/app/ar8030" -o -path "*/host_drv/install/include" \) \
    2>/dev/null)
  while IFS= read -r hit; do
    local candidate_root
    candidate_root="$(dirname "$(dirname "${hit}")")"
    if [[ -f "${candidate_root}/CMakeLists.txt" && \
          -d "${candidate_root}/com" && -d "${candidate_root}/daemon" ]]; then
      echo "${candidate_root}"
      return 0
    fi
  done < <(find "${search_root}" -type d -path "*/app/ar8030" 2>/dev/null)
  return 1
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
  local fetch_root="${ARTOSYN_WORK_ROOT}/fetch"
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
  local repo_root="${ARTOSYN_WORK_ROOT}/source/${ARTLINK_REPO_DIR}"
  local clone_manifest="${ARTOSYN_WORK_ROOT}/clone-manifest.log"
  rm -rf "${repo_root}" || return 1
  mkdir -p "$(dirname "${repo_root}")" || return 1

  _artlink_git clone --quiet "${ARTLINK_REPO}" "${repo_root}" || return 1
  if [[ -n "${ARTLINK_BRANCH}" && "${ARTLINK_BRANCH}" != "latest" ]]; then
    pushd "${repo_root}" >/dev/null || return 1
      _artlink_git fetch --all --tags --prune --quiet || return 1
      _artlink_git checkout -q -f "${ARTLINK_BRANCH}" || return 1
    popd >/dev/null || return 1
  fi

  {
    echo "[Artosyn] Clone manifest"
    echo "[Artosyn] repo_root=${repo_root}"
    echo "[Artosyn] top-level:"
    ls -la "${repo_root}"
    echo "[Artosyn] key paths:"
    ls -la "${repo_root}/host_drv" 2>/dev/null || true
    ls -la "${repo_root}/host_drv/app" 2>/dev/null || true
    ls -la "${repo_root}/host_drv/com" 2>/dev/null || true
    echo "[Artosyn] archive hints:"
    find "${repo_root}" -type f \
      \( -iname "*.tar.gz" -o -iname "*.tgz" -o -iname "*.tar" -o -iname "*.zip" \) \
      | head -n 10 || true
  } | tee "${clone_manifest}" >&2

  local resolved
  resolved="$(_find_sdk_root "${repo_root}" || true)"
  if [[ -z "${resolved}" ]]; then
    local archive_hit=""
    local fallback_archive=""
    while IFS= read -r candidate; do
      if [[ -z "${fallback_archive}" ]]; then
        fallback_archive="${candidate}"
      fi
      local lower
      lower="$(printf '%s' "${candidate}" | tr '[:upper:]' '[:lower:]')"
      if [[ "${lower}" == *"host_drv"* || "${lower}" == *"artosyn"* || "${lower}" == *"artlink"* ]]; then
        archive_hit="${candidate}"
        break
      fi
    done < <(find "${repo_root}" -type f \
      \( -iname "*.tar.gz" -o -iname "*.tgz" -o -iname "*.tar" -o -iname "*.zip" \) \
      | sort)
    if [[ -z "${archive_hit}" ]]; then
      archive_hit="${fallback_archive}"
    fi
    if [[ -n "${archive_hit}" ]]; then
      echo "[Artosyn] Found SDK archive in cloned repo: ${archive_hit}" >&2
      local extract_dir="${ARTOSYN_WORK_ROOT}/source-extract/${ARTLINK_REPO_DIR}"
      if _extract_archive "${archive_hit}" "${extract_dir}"; then
        resolved="$(_find_sdk_root "${extract_dir}" || true)"
      fi
    fi
  fi
  if [[ -z "${resolved}" ]]; then
    return 1
  fi
  echo "${resolved}"
}

_configure_host_drv_build_dir() {
  local host_drv_dir="$1"
  local build_dir="$2"
  if [[ ! -f "${host_drv_dir}/CMakeLists.txt" ]]; then
    return 1
  fi
  if ! command -v cmake >/dev/null 2>&1; then
    return 1
  fi
  rm -rf "${build_dir}" || return 1
  _artlink_cmake_configure -S "${host_drv_dir}" -B "${build_dir}" \
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
    -DUSING_8030USB=ON \
    -DUSING_8030SDIO=OFF \
    -DUSING_8030UART=OFF \
    -DUSING_8030DRV=OFF >&2 || return 1
}

_sdk_source_dir() {
  local sdk_root="$1"
  if [[ -f "${sdk_root}/CMakeLists.txt" && -d "${sdk_root}/app/ar8030" ]]; then
    echo "${sdk_root}"
  elif [[ -f "${sdk_root}/host_drv/CMakeLists.txt" ]]; then
    echo "${sdk_root}/host_drv"
  else
    return 1
  fi
}

_configure_sdk_build_dir() {
  local sdk_root="$1"
  local build_dir="$2"
  local source_dir
  source_dir="$(_sdk_source_dir "${sdk_root}")" || return 1
  if [[ "${source_dir}" == "${sdk_root}/host_drv" ]]; then
    _configure_host_drv_build_dir "${source_dir}" "${build_dir}"
    return
  fi
  _artlink_cmake_configure -S "${source_dir}" -B "${build_dir}" \
    -DAPP_STATIC_LIB=ON \
    -DUSING_8030USB=ON \
    -DUSING_8030SDIO=OFF \
    -DUSING_8030UART=OFF >&2
}

_build_client_lib_from_source() {
  local sdk_root="$1"
  if ! _sdk_source_dir "${sdk_root}" >/dev/null; then
    return 1
  fi
  if ! command -v cmake >/dev/null 2>&1; then
    return 1
  fi

  # Keep independently resolved artifacts in separate build trees. The host_drv
  # configurator recreates its build directory, so a later daemon/tuntap probe
  # must not delete a client library that was already selected for packaging.
  local build_dir="${ARTOSYN_WORK_ROOT}/sdk-client"
  echo "[Artosyn] libar8030_client missing, building ar8030_client from source." >&2

  _configure_sdk_build_dir "${sdk_root}" "${build_dir}" || return 1

  cmake --build "${build_dir}" --target ar8030_client >&2 || return 1

  local built_lib
  built_lib="$(find "${build_dir}" -type f -name "libar8030_client.a" | head -n 1 || true)"
  if [[ -z "${built_lib}" ]]; then
    built_lib="$(find "${build_dir}" -type f -name "libar8030_client.so" | head -n 1 || true)"
  fi
  if [[ -z "${built_lib}" ]]; then
    return 1
  fi
  if [[ "${built_lib}" == *.a ]]; then
    local dep_candidates=(
      "${build_dir}/com/libcom.a"
      "${sdk_root}/build/com/libcom.a"
      "${sdk_root}/host_drv/build/com/libcom.a"
      "${sdk_root}/host_drv/install/bin/libcom.a"
      "${sdk_root}/host_drv/com/libcom.a"
    )
    local dep
    local out="${built_lib}"
    for dep in "${dep_candidates[@]}"; do
      if [[ -f "${dep}" ]]; then
        out="${out};${dep}"
        break
      fi
    done
    echo "${out}"
    return 0
  fi
  echo "${built_lib}"
}

_find_daemon_binary_in_tree() {
  local sdk_root="$1"
  if [[ -z "${sdk_root}" || ! -d "${sdk_root}" ]]; then
    return 1
  fi
  local daemon_candidates=(
    "${sdk_root}/install/arm64/bin/l4_daemon"
    "${sdk_root}/install/armhf/bin/l4_daemon"
    "${sdk_root}/install/x86_64/bin/l4_daemon"
    "${sdk_root}/build/arm64/daemon/l4_daemon"
    "${sdk_root}/build/armhf/daemon/l4_daemon"
    "${sdk_root}/build/x86_64/daemon/l4_daemon"
    "${sdk_root}/host_drv/app/ar8030/artosyn_daemon"
    "${sdk_root}/host_drv/app/ar8030/ar8030_daemon"
    "${sdk_root}/host_drv/app/ar8030/artlinkd"
    "${sdk_root}/host_drv/app/ar8030/bbd"
    "${sdk_root}/host_drv/app/ar8030/bb_daemon"
    "${sdk_root}/host_drv/daemon/daemon"
    "${sdk_root}/host_drv/build/app/ar8030/artosyn_daemon"
    "${sdk_root}/host_drv/build/app/ar8030/ar8030_daemon"
    "${sdk_root}/host_drv/build/app/ar8030/artlinkd"
    "${sdk_root}/host_drv/build/app/ar8030/bbd"
    "${sdk_root}/host_drv/build/app/ar8030/bb_daemon"
    "${sdk_root}/host_drv/build/daemon/daemon"
    "${sdk_root}/host_drv/install/bin/artosyn_daemon"
    "${sdk_root}/host_drv/install/bin/ar8030_daemon"
    "${sdk_root}/host_drv/install/bin/artlinkd"
    "${sdk_root}/host_drv/install/bin/bbd"
    "${sdk_root}/host_drv/install/bin/bb_daemon"
    "${sdk_root}/host_drv/install/bin/daemon"
  )
  local candidate
  for candidate in "${daemon_candidates[@]}"; do
    if [[ -f "${candidate}" ]]; then
      echo "${candidate}"
      return 0
    fi
  done
  local hit
  hit="$(find "${sdk_root}" -type f \
    \( -iname "l4_daemon" -o -iname "artosyn_daemon" -o -iname "ar8030_daemon" -o -iname "artlinkd" -o -iname "bbd" -o -iname "bb_daemon" -o -iname "daemon" \) \
    | head -n 1 || true)"
  if [[ -n "${hit}" ]]; then
    echo "${hit}"
    return 0
  fi
  return 1
}

_find_tuntap_binary_in_tree() {
  local sdk_root="$1"
  if [[ -z "${sdk_root}" || ! -d "${sdk_root}" ]]; then
    return 1
  fi
  local tuntap_candidates=(
    "${sdk_root}/install/arm64/bin/l4_tuntap"
    "${sdk_root}/install/armhf/bin/l4_tuntap"
    "${sdk_root}/install/x86_64/bin/l4_tuntap"
    "${sdk_root}/build/arm64/app/tuntap/l4_tuntap"
    "${sdk_root}/build/armhf/app/tuntap/l4_tuntap"
    "${sdk_root}/build/x86_64/app/tuntap/l4_tuntap"
    "${sdk_root}/host_drv/install/dev_helper/tuntap_bb"
    "${sdk_root}/host_drv/dev_helper/tuntap_bb"
    "${sdk_root}/host_drv/build/dev_helper/tuntap_bb"
    "${sdk_root}/host_drv/build/dev_helper/tuntap_bb/tuntap_bb"
  )
  local candidate
  for candidate in "${tuntap_candidates[@]}"; do
    if [[ -f "${candidate}" ]]; then
      echo "${candidate}"
      return 0
    fi
  done
  local hit
  hit="$(find "${sdk_root}" -type f \
    \( -name "l4_tuntap" -o -name "tuntap_bb" \) | head -n 1 || true)"
  if [[ -n "${hit}" ]]; then
    echo "${hit}"
    return 0
  fi
  return 1
}

_build_daemon_from_source() {
  local sdk_root="$1"
  if ! _sdk_source_dir "${sdk_root}" >/dev/null; then
    return 1
  fi
  if ! command -v cmake >/dev/null 2>&1; then
    return 1
  fi
  local build_dir="${ARTOSYN_WORK_ROOT}/sdk-daemon"
  echo "[Artosyn] daemon missing, trying to build daemon targets from source." >&2

  _configure_sdk_build_dir "${sdk_root}" "${build_dir}" || return 1

  local available_targets=""
  local targets_help_file="${build_dir}/.openhd_targets_help.txt"
  if cmake --build "${build_dir}" --target help >"${targets_help_file}" 2>/dev/null; then
    available_targets="$(tr '[:upper:]' '[:lower:]' < "${targets_help_file}")"
  fi

  local targets=(
    "l4_daemon"
    "artosyn_daemon"
    "ar8030_daemon"
    "artlinkd"
    "bbd"
    "bb_daemon"
    "daemon"
  )
  target_declared() {
    local target_name="$1"
    if [[ -z "${available_targets}" ]]; then
      # Unknown target set: keep previous behavior and try building.
      return 0
    fi
    grep -Eq "(^|[[:space:]])${target_name,,}([[:space:]]|$)" <<<"${available_targets}"
  }
  local t
  for t in "${targets[@]}"; do
    if ! target_declared "${t}"; then
      echo "[Artosyn] Skipping unavailable daemon build target '${t}'." >&2
      continue
    fi
    if cmake --build "${build_dir}" --target "${t}" >&2; then
      local built
      built="$(_find_daemon_binary_in_tree "${sdk_root}" || true)"
      if [[ -z "${built}" ]]; then
        built="$(find "${build_dir}" -type f \
          \( -iname "l4_daemon" -o -iname "artosyn_daemon" -o -iname "ar8030_daemon" -o -iname "artlinkd" -o -iname "bbd" -o -iname "bb_daemon" -o -iname "daemon" \) \
          | head -n 1 || true)"
      fi
      if [[ -n "${built}" ]]; then
        echo "${built}"
        return 0
      fi
    else
      echo "[Artosyn] Build failed for daemon target '${t}', trying next candidate." >&2
    fi
  done
  # Final fallback: try default build and search again.
  cmake --build "${build_dir}" >&2 || true
  local built
  built="$(_find_daemon_binary_in_tree "${sdk_root}" || true)"
  if [[ -z "${built}" ]]; then
    built="$(find "${build_dir}" -type f \
      \( -iname "l4_daemon" -o -iname "artosyn_daemon" -o -iname "ar8030_daemon" -o -iname "artlinkd" -o -iname "bbd" -o -iname "bb_daemon" -o -iname "daemon" \) \
      | head -n 1 || true)"
  fi
  if [[ -n "${built}" ]]; then
    echo "${built}"
    return 0
  fi
  return 1
}

_build_tuntap_from_source() {
  local sdk_root="$1"
  if ! _sdk_source_dir "${sdk_root}" >/dev/null; then
    return 1
  fi
  if ! command -v cmake >/dev/null 2>&1; then
    return 1
  fi
  # Tuntap is optional and is resolved last. Isolate its configure step so it
  # cannot erase the required daemon and client-library artifacts.
  local build_dir="${ARTOSYN_WORK_ROOT}/sdk-tuntap"
  echo "[Artosyn] tunnel helper missing, trying to build it from source." >&2

  _configure_sdk_build_dir "${sdk_root}" "${build_dir}" || return 1

  local available_targets=""
  local targets_help_file="${build_dir}/.openhd_targets_help_tuntap.txt"
  if cmake --build "${build_dir}" --target help >"${targets_help_file}" 2>/dev/null; then
    available_targets="$(tr '[:upper:]' '[:lower:]' < "${targets_help_file}")"
  fi

  local target
  for target in l4_tuntap tuntap_bb; do
    if [[ -z "${available_targets}" ]] ||
       grep -Eq "(^|[[:space:]])${target}([[:space:]]|$)" <<<"${available_targets}"; then
      cmake --build "${build_dir}" --target "${target}" >&2 || continue
      break
    fi
  done

  local built
  built="$(_find_tuntap_binary_in_tree "${sdk_root}" || true)"
  if [[ -z "${built}" ]]; then
    built="$(find "${build_dir}" -type f \
      \( -name "l4_tuntap" -o -name "tuntap_bb" \) | head -n 1 || true)"
  fi
  if [[ -n "${built}" ]]; then
    echo "${built}"
    return 0
  fi
  return 1
}

resolve_artosyn_sdk() {
  local sdk_root="${ARTOSYN_SDK_ROOT:-}"
  local sdk_lib="${ARTOSYN_SDK_LIB:-}"
  local sdk_daemon="${ARTOSYN_SDK_DAEMON:-}"
  local sdk_tuntap="${ARTOSYN_SDK_TUNTAP:-}"
  local fetch_mode="${ARTLINK_FETCH_MODE:-auto}"
  local force_source_build="${OPENHD_ARTOSYN_FORCE_SOURCE_BUILD:-0}"
  if [[ "${force_source_build}" == "1" ]]; then
    sdk_lib=""
    sdk_daemon=""
    sdk_tuntap=""
  fi
  echo "[Artosyn] Resolving SDK (mode=${fetch_mode})." >&2

  # Keep git repo explicit for git fetch flow. DOWNLOAD_URL is treated as archive input only.
  echo "[Artosyn] Git repo: ${ARTLINK_REPO}" >&2

  # Optional archive injection for CI/private builders.
  local sdk_archive="${ARTOSYN_SDK_ARCHIVE:-}"
  if [[ -z "${sdk_root}" && -n "${sdk_archive}" ]]; then
    if [[ ! -f "${sdk_archive}" ]]; then
      echo "ARTOSYN_SDK_ARCHIVE points to missing file: ${sdk_archive}" >&2
      return 1
    fi
    local extract_root="${ARTOSYN_WORK_ROOT}/archive"
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
    if [[ -d "${extract_root}/host_drv" || \
          ( -d "${extract_root}/app/ar8030" && -d "${extract_root}/daemon" ) ]]; then
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
      "${ARTLINK_RESOLVER_DIR}/../../out/vendor/L4_Linux_SDK"
      "${ARTLINK_RESOLVER_DIR}/../../${ARTLINK_REPO_DIR}"
      "${OPENHD_KERNEL_BUILDER_DIR:-}/workdir/mods/${ARTLINK_REPO_DIR}"
      "${OPENHD_KERNEL_BUILDER_DIR:-}/workdir/mods/OpenHD-ArtLink"
      "/opt/openhd-private/artosyn_sdk"
      "/opt/openhd/artosyn_sdk"
      "/opt/artosyn_sdk"
      "/usr/local/share/openhd/artosyn_sdk"
    )
    local win_mount_candidates=()
    local nullglob_was_set=0
    if shopt -q nullglob; then
      nullglob_was_set=1
    fi
    shopt -s nullglob
    win_mount_candidates+=(/mnt/c/Users/*/OpenHD-ArtLink /c/Users/*/OpenHD-ArtLink)
    if [[ "${nullglob_was_set}" -eq 0 ]]; then
      shopt -u nullglob
    fi
    candidates+=("${win_mount_candidates[@]}")
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

  if [[ -n "${sdk_root}" && -z "${sdk_lib}" &&
        "${force_source_build}" != "1" ]]; then
    local lib_candidates=(
      "${sdk_root}/install/arm64/lib/libar8030_client.a"
      "${sdk_root}/install/arm64/lib/libar8030_client.so"
      "${sdk_root}/install/armhf/lib/libar8030_client.a"
      "${sdk_root}/install/armhf/lib/libar8030_client.so"
      "${sdk_root}/install/x86_64/lib/libar8030_client.a"
      "${sdk_root}/install/x86_64/lib/libar8030_client.so"
      "${sdk_root}/build/arm64/app/ar8030/libar8030_client.a"
      "${sdk_root}/build/arm64/app/ar8030/libar8030_client.so"
      "${sdk_root}/build/armhf/app/ar8030/libar8030_client.a"
      "${sdk_root}/build/armhf/app/ar8030/libar8030_client.so"
      "${sdk_root}/build/x86_64/app/ar8030/libar8030_client.a"
      "${sdk_root}/build/x86_64/app/ar8030/libar8030_client.so"
      "${sdk_root}/host_drv/install/bin/libar8030_client.a"
      "${sdk_root}/host_drv/install/bin/libar8030_client.so"
      "${sdk_root}/host_drv/build/app/ar8030/libar8030_client.a"
      "${sdk_root}/host_drv/build/app/ar8030/libar8030_client.so"
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

  if [[ -n "${sdk_root}" && -z "${sdk_lib}" &&
        "${force_source_build}" != "1" ]]; then
    # Fallback for SDK trees that keep prebuilt libs in non-standard host_drv paths.
    sdk_lib="$(find "${sdk_root}" -type f \( -name "libar8030_client.a" -o -name "libar8030_client.so" \) | head -n 1 || true)"
  fi

  if [[ -n "${sdk_root}" && -z "${sdk_lib}" ]]; then
    sdk_lib="$(_build_client_lib_from_source "${sdk_root}" || true)"
  fi

  if [[ -n "${sdk_root}" && -z "${sdk_daemon}" &&
        "${force_source_build}" != "1" ]]; then
    sdk_daemon="$(_find_daemon_binary_in_tree "${sdk_root}" || true)"
  fi

  if [[ -n "${sdk_root}" && -z "${sdk_daemon}" ]]; then
    sdk_daemon="$(_build_daemon_from_source "${sdk_root}" || true)"
  fi

  if [[ -n "${sdk_root}" && -z "${sdk_tuntap}" &&
        "${force_source_build}" != "1" ]]; then
    sdk_tuntap="$(_find_tuntap_binary_in_tree "${sdk_root}" || true)"
  fi

  if [[ -n "${sdk_root}" && -z "${sdk_tuntap}" ]]; then
    sdk_tuntap="$(_build_tuntap_from_source "${sdk_root}" || true)"
  fi

  if [[ -n "${sdk_lib}" && "${sdk_lib}" == *.a* ]]; then
    local primary_lib="${sdk_lib%%;*}"
    local lib_dir
    lib_dir="$(dirname "${primary_lib}")"
    local build_root
    build_root="$(dirname "$(dirname "${lib_dir}")")"
    local dep_candidates=(
      "${build_root}/com/libcom.a"
      "${sdk_root}/build/arm64/com/libcom.a"
      "${sdk_root}/build/armhf/com/libcom.a"
      "${sdk_root}/build/x86_64/com/libcom.a"
      "${sdk_root}/host_drv/build/com/libcom.a"
      "${sdk_root}/host_drv/install/bin/libcom.a"
      "${sdk_root}/host_drv/com/libcom.a"
    )
    local dep
    for dep in "${dep_candidates[@]}"; do
      if [[ -f "${dep}" ]]; then
        case ";${sdk_lib};" in
          *";${dep};"*) ;;
          *) sdk_lib="${sdk_lib};${dep}" ;;
        esac
        break
      fi
    done
  fi

  if [[ -z "${sdk_root}" || -z "${sdk_lib}" ]]; then
    cat >&2 <<'EOF'
[Artosyn] SDK not resolved; continuing with Artosyn integration disabled.
To enable Artosyn, set one of:
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
    export ARTOSYN_SDK_ROOT=""
    export ARTOSYN_SDK_LIB=""
    export ARTOSYN_SDK_DAEMON=""
    export ARTOSYN_SDK_TUNTAP=""
    return 0
  fi

  echo "[Artosyn] SDK resolved: root=${ARTOSYN_SDK_ROOT:-${sdk_root}}" >&2
  echo "[Artosyn] SDK lib resolved: ${ARTOSYN_SDK_LIB:-${sdk_lib}}" >&2
  if [[ -n "${sdk_daemon}" ]]; then
    echo "[Artosyn] SDK daemon resolved: ${ARTOSYN_SDK_DAEMON:-${sdk_daemon}}" >&2
  else
    echo "[Artosyn] SDK daemon unresolved; packaging step may fail if daemon is required." >&2
  fi
  if [[ -n "${sdk_tuntap}" ]]; then
    echo "[Artosyn] SDK tuntap resolved: ${ARTOSYN_SDK_TUNTAP:-${sdk_tuntap}}" >&2
  else
    echo "[Artosyn] SDK tuntap unresolved; runtime LAN tunnel helper may be unavailable." >&2
  fi

  export ARTOSYN_SDK_ROOT="${sdk_root}"
  export ARTOSYN_SDK_LIB="${sdk_lib}"
  export ARTOSYN_SDK_DAEMON="${sdk_daemon}"
  export ARTOSYN_SDK_TUNTAP="${sdk_tuntap}"
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  resolve_artosyn_sdk
  if [[ -n "${ARTOSYN_SDK_ROOT:-}" && -n "${ARTOSYN_SDK_LIB:-}" ]]; then
    echo "ARTOSYN_SDK_ROOT=${ARTOSYN_SDK_ROOT}"
    echo "ARTOSYN_SDK_LIB=${ARTOSYN_SDK_LIB}"
    if [[ -n "${ARTOSYN_SDK_DAEMON:-}" ]]; then
      echo "ARTOSYN_SDK_DAEMON=${ARTOSYN_SDK_DAEMON}"
    fi
    if [[ -n "${ARTOSYN_SDK_TUNTAP:-}" ]]; then
      echo "ARTOSYN_SDK_TUNTAP=${ARTOSYN_SDK_TUNTAP}"
    fi
  fi
fi
