#!/usr/bin/env bash
set -euo pipefail

architecture="${1:?Usage: validate_cross_sysroot.sh <arm64|armhf> <sysroot>}"
sysroot="$(realpath "${2:?Usage: validate_cross_sysroot.sh <arm64|armhf> <sysroot>}")"

case "${architecture}" in
  arm64) triplet="aarch64-linux-gnu" ;;
  armhf) triplet="arm-linux-gnueabihf" ;;
  *)
    echo "Unsupported architecture: ${architecture}" >&2
    exit 1
    ;;
esac

grep -qx "architecture=${architecture}" "${sysroot}/openhd-sysroot.manifest"
compiler="${triplet}-g++-10"
command -v "${compiler}" >/dev/null

export PKG_CONFIG_SYSROOT_DIR="${sysroot}"
export PKG_CONFIG_LIBDIR="${sysroot}/usr/lib/${triplet}/pkgconfig:${sysroot}/usr/lib/pkgconfig:${sysroot}/usr/share/pkgconfig"
gcc_runtime_dir="$(find "${sysroot}/usr/lib/gcc/${triplet}" -mindepth 1 -maxdepth 1 -type d -print -quit)"
if [[ -z "${gcc_runtime_dir}" ]]; then
  echo "Target GCC runtime directory is missing from the sysroot." >&2
  exit 1
fi

output="$(mktemp)"
trap 'rm -f "${output}"' EXIT
read -r -a pkg_config_flags <<<"$(
  pkg-config --cflags --libs \
    gstreamer-1.0 libdrm gbm egl glesv2 freetype2 zlib
)"
printf '%s\n' \
  '#include <Poco/Net/IPAddress.h>' \
  '#include <gst/gst.h>' \
  '#include <xf86drm.h>' \
  '#include <gbm.h>' \
  '#include <EGL/egl.h>' \
  '#include <GLES2/gl2.h>' \
  '#include <ft2build.h>' \
  '#include FT_FREETYPE_H' \
  '#include <zlib.h>' \
  'int main() {' \
  '  Poco::Net::IPAddress address;' \
  '  gst_init(nullptr, nullptr);' \
  '  drmVersionPtr drm_version = drmGetVersion(-1);' \
  '  gbm_device* gbm = gbm_create_device(-1);' \
  '  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);' \
  '  const GLubyte* gl_version = glGetString(GL_VERSION);' \
  '  FT_Library freetype = nullptr;' \
  '  int ft_status = FT_Init_FreeType(&freetype);' \
  '  const char* linked_zlib_version = zlibVersion();' \
  '  return address.isWildcard() + (drm_version != nullptr) + (gbm != nullptr)' \
  '      + (display != EGL_NO_DISPLAY) + (gl_version != nullptr) + ft_status' \
  '      + (linked_zlib_version == nullptr);' \
  '}' \
  | "${compiler}" --sysroot="${sysroot}" -x c++ - \
      "${pkg_config_flags[@]}" \
      -L"${gcc_runtime_dir}" \
      -L"${sysroot}/usr/lib/${triplet}" \
      -L"${sysroot}/lib/${triplet}" \
      -Wl,-rpath-link,"${sysroot}/usr/lib/${triplet}" \
      -Wl,-rpath-link,"${sysroot}/lib/${triplet}" \
      -pthread \
      -lPocoNet -lPocoFoundation \
      -o "${output}"

file "${output}"
readelf -d "${output}" | grep NEEDED
if [[ "${architecture}" == "arm64" ]]; then
  grep -qx 'rockchip_mpp_commit=2e93ab791e0b2c803022622c30fea79b80450830' \
    "${sysroot}/openhd-sysroot.manifest"
  read -r -a mpp_flags <<<"$(pkg-config --cflags --libs rockchip_mpp)"
  printf '%s\n' \
    '#include <rk_mpi.h>' \
    'int main() {' \
    '  return mpp_check_support_format(MPP_CTX_ENC, MPP_VIDEO_CodingAVC);' \
    '}' \
    | "${compiler}" --sysroot="${sysroot}" -x c++ - \
        "${mpp_flags[@]}" \
        -Wl,-rpath-link,"${sysroot}/usr/lib" \
        -o "${output}"
  readelf -d "${output}" | grep 'librockchip_mpp.so.1'
fi
echo "Validated ${architecture} OpenHD cross sysroot."
