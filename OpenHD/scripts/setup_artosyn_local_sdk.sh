#!/usr/bin/env bash

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

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${SCRIPT_DIR}/resolve_artosyn_sdk.sh"

if [[ -n "${CI:-}" || -n "${GITHUB_ACTIONS:-}" ]]; then
  echo "CI environment detected. This helper is intended for interactive local use only."
  exit 1
fi

if [[ ! -t 0 ]]; then
  echo "Interactive terminal required."
  exit 1
fi

DEST_ROOT_DEFAULT="/opt/openhd-private/artosyn_sdk"
DEST_ROOT="${ARTOSYN_LOCAL_DEST:-${DEST_ROOT_DEFAULT}}"

REPO_DEFAULT="${ARTLINK_REPO:-${ARTLINK_REPO_DEFAULT}}"
BRANCH_DEFAULT="${ARTLINK_BRANCH:-main}"
USER_DEFAULT="${ARTLINK_GIT_AUTH_USERNAME:-}"
if [[ -z "${USER_DEFAULT}" ]]; then
  USER_DEFAULT="$(git config --global user.name 2>/dev/null || true)"
fi
if [[ -z "${USER_DEFAULT}" ]]; then
  USER_DEFAULT="$(git config --global user.email 2>/dev/null || true)"
fi
if [[ -z "${USER_DEFAULT}" ]]; then
  USER_DEFAULT="github-user"
fi

read -r -p "ArtLink repo URL [${REPO_DEFAULT}]: " input_repo
ARTLINK_REPO="${input_repo:-${REPO_DEFAULT}}"

read -r -p "ArtLink branch [${BRANCH_DEFAULT}]: " input_branch
ARTLINK_BRANCH="${input_branch:-${BRANCH_DEFAULT}}"

read -r -p "GitHub username [${USER_DEFAULT}]: " input_user
ARTLINK_GIT_AUTH_USERNAME="${input_user:-${USER_DEFAULT}}"

echo -n "GitHub personal access token: "
read -r -s token
echo
if [[ -z "${token}" ]]; then
  echo "Token is required."
  exit 1
fi

ARTLINK_FETCH_MODE="git-only"
ARTLINK_GIT_AUTH="${ARTLINK_GIT_AUTH_USERNAME}:${token}"

unset token

export ARTLINK_REPO
export ARTLINK_BRANCH
export ARTLINK_FETCH_MODE
export ARTLINK_GIT_AUTH
export ARTLINK_GIT_AUTH_USERNAME

echo "[Artosyn] Resolving SDK via private git access..."
resolve_artosyn_sdk

if [[ -z "${ARTOSYN_SDK_ROOT:-}" || -z "${ARTOSYN_SDK_LIB:-}" ]]; then
  echo "[Artosyn] Failed to resolve SDK."
  exit 1
fi

NEEDS_SUDO=0
dest_parent="$(dirname "${DEST_ROOT}")"
if [[ ! -d "${dest_parent}" || ! -w "${dest_parent}" ]]; then
  NEEDS_SUDO=1
fi

run_as_needed() {
  if [[ "${NEEDS_SUDO}" -eq 1 ]]; then
    if ! command -v sudo >/dev/null 2>&1; then
      echo "Need write access to ${DEST_ROOT}, but sudo is not available."
      exit 1
    fi
    sudo "$@"
  else
    "$@"
  fi
}

copy_tree() {
  local src="$1"
  local dst="$2"
  if command -v rsync >/dev/null 2>&1; then
    run_as_needed mkdir -p "${dst}"
    if [[ "${NEEDS_SUDO}" -eq 1 ]]; then
      sudo rsync -a --delete "${src}/" "${dst}/"
    else
      rsync -a --delete "${src}/" "${dst}/"
    fi
    return 0
  fi
  run_as_needed mkdir -p "${dst}"
  # Fallback without delete semantics when rsync is unavailable.
  if [[ "${NEEDS_SUDO}" -eq 1 ]]; then
    tar -C "${src}" -cf - . | sudo tar -C "${dst}" -xf -
  else
    tar -C "${src}" -cf - . | tar -C "${dst}" -xf -
  fi
}

stage_runtime_file() {
  local src="$1"
  local dst="$2"
  local mode="$3"
  if [[ ! -f "${src}" ]]; then
    return 0
  fi
  run_as_needed install -D -m "${mode}" "${src}" "${dst}"
}

echo "[Artosyn] Copying SDK root to ${DEST_ROOT} ..."
copy_tree "${ARTOSYN_SDK_ROOT}" "${DEST_ROOT}"

IFS=';' read -r -a sdk_libs <<< "${ARTOSYN_SDK_LIB}"
for lib in "${sdk_libs[@]}"; do
  [[ -z "${lib}" ]] && continue
  [[ ! -f "${lib}" ]] && continue
  if [[ "${lib}" == "${DEST_ROOT}"/* ]]; then
    continue
  fi
  lib_name="$(basename "${lib}")"
  case "${lib_name}" in
    libar8030_client*)
      stage_runtime_file "${lib}" "${DEST_ROOT}/host_drv/app/ar8030/${lib_name}" "0644"
      ;;
    libcom*)
      stage_runtime_file "${lib}" "${DEST_ROOT}/host_drv/com/${lib_name}" "0644"
      ;;
  esac
done

if [[ -n "${ARTOSYN_SDK_DAEMON:-}" && -f "${ARTOSYN_SDK_DAEMON}" ]]; then
  if [[ "${ARTOSYN_SDK_DAEMON}" != "${DEST_ROOT}"/* ]]; then
    daemon_name="$(basename "${ARTOSYN_SDK_DAEMON}")"
    stage_runtime_file "${ARTOSYN_SDK_DAEMON}" \
      "${DEST_ROOT}/host_drv/install/bin/${daemon_name}" "0755"
  fi
fi

unset ARTLINK_GIT_AUTH

# Re-resolve against the persisted local location so later builds pick stable paths.
export ARTOSYN_SDK_ROOT="${DEST_ROOT}"
export ARTOSYN_SDK_LIB=""
export ARTOSYN_SDK_DAEMON=""
resolve_artosyn_sdk

echo
echo "[Artosyn] Local SDK setup complete."
echo "[Artosyn] Resolved root: ${ARTOSYN_SDK_ROOT}"
echo "[Artosyn] Resolved lib:  ${ARTOSYN_SDK_LIB}"
if [[ -n "${ARTOSYN_SDK_DAEMON:-}" ]]; then
  echo "[Artosyn] Resolved daemon: ${ARTOSYN_SDK_DAEMON}"
fi
echo
echo "Normal OpenHD build scripts should now auto-detect this SDK path."
