#!/usr/bin/env bash
# setup_rpi_imx415.sh
# Run this on the Raspberry Pi air unit BEFORE using the Arducam IMX415 camera.
# Covers system-level requirements (kernel driver + libcamera tuning) that the
# OpenHD code changes alone cannot provide.
#
# Requirements: Raspberry Pi OS Bookworm, kernel >= 6.12.25, internet access.
# Tested on: RPi 4B, RPi CM4, RPi 5.
set -euo pipefail

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
info()  { echo -e "${GREEN}[INFO]${NC}  $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; exit 1; }

# ── 1. OS check ───────────────────────────────────────────────────────────────
info "Checking OS version..."
if ! grep -qi "bookworm" /etc/os-release 2>/dev/null; then
    error "Raspberry Pi OS Bookworm required. Your OS: $(grep PRETTY_NAME /etc/os-release | cut -d= -f2)
    OpenHD's current Pi image is built on Bullseye which lacks the imx415 kernel driver.
    Flash a Bookworm image, install OpenHD on top, then re-run this script."
fi
info "OS: OK (Bookworm)"

# ── 2. Kernel version check ───────────────────────────────────────────────────
info "Checking kernel version..."
KERNEL=$(uname -r)
MAJOR=$(echo "$KERNEL" | cut -d. -f1)
MINOR=$(echo "$KERNEL" | cut -d. -f2)
PATCH=$(echo "$KERNEL" | cut -d. -f3 | cut -d- -f1)

if [[ "$MAJOR" -lt 6 ]] || { [[ "$MAJOR" -eq 6 ]] && [[ "$MINOR" -lt 12 ]]; }; then
    warn "Kernel $KERNEL is too old. Upgrading firmware and kernel..."
    sudo apt-get update -qq
    sudo apt-get install -y --only-upgrade raspberrypi-kernel raspberrypi-bootloader
    info "Kernel upgraded. A reboot is required before continuing."
    warn "Rebooting in 5 seconds — rerun this script after reboot."
    sleep 5; sudo reboot
fi

if [[ "$MAJOR" -eq 6 ]] && [[ "$MINOR" -eq 12 ]] && [[ "$PATCH" -lt 25 ]]; then
    warn "Kernel $KERNEL has a known Arducam IMX415 stripe bug. Upgrading to >= 6.12.25..."
    sudo apt-get update -qq
    sudo apt-get install -y --only-upgrade raspberrypi-kernel raspberrypi-bootloader
    info "Kernel upgraded. Rebooting..."
    sleep 5; sudo reboot
fi
info "Kernel: OK ($KERNEL)"

# ── 3. libcamera version check ────────────────────────────────────────────────
info "Checking libcamera version..."
if ! command -v libcamera-hello &>/dev/null; then
    info "libcamera-apps not found — installing..."
    sudo apt-get update -qq
    sudo apt-get install -y libcamera-apps libcamera-dev
fi

LC_VERSION=$(libcamera-hello --version 2>&1 | grep -oP '\d+\.\d+\.\d+' | head -1 || echo "0.0.0")
info "libcamera version: $LC_VERSION"

# IMX415 tuning support arrived in libcamera 0.5.0+rpi20250707
LC_MAJOR=$(echo "$LC_VERSION" | cut -d. -f1)
LC_MINOR=$(echo "$LC_VERSION" | cut -d. -f2)
if [[ "$LC_MAJOR" -eq 0 ]] && [[ "$LC_MINOR" -lt 5 ]]; then
    info "Upgrading libcamera to get IMX415 tuning support..."
    sudo apt-get update -qq
    sudo apt-get install -y --only-upgrade libcamera-apps libcamera-dev libcamera0
fi

# ── 4. Verify IMX415 tuning file exists ───────────────────────────────────────
info "Checking for IMX415 tuning file..."
TUNING_DIRS=(
    "/usr/share/libcamera/ipa/rpi/pisp"
    "/usr/share/libcamera/ipa/rpi/vc4"
    "/usr/share/libcamera/ipa/rpi/pisp/data"
    "/usr/share/libcamera/ipa/rpi/vc4/data"
)
IMX415_TUNING=""
for dir in "${TUNING_DIRS[@]}"; do
    if [[ -f "$dir/imx415.json" ]]; then
        IMX415_TUNING="$dir/imx415.json"
        break
    fi
done

if [[ -z "$IMX415_TUNING" ]]; then
    warn "IMX415 tuning file (imx415.json) not found in any libcamera data directory."
    warn "Available tuning files:"
    find /usr/share/libcamera -name "*.json" 2>/dev/null | sort
    error "IMX415 tuning file missing. Your libcamera version may not include IMX415 support.
    Try: sudo apt-get install --only-upgrade libcamera0 libcamera-apps
    Or flash a Raspberry Pi OS image from July 2025 or later."
fi
info "Tuning file: OK ($IMX415_TUNING)"

# ── 5. Check for imx415.dtbo overlay ─────────────────────────────────────────
info "Checking for imx415 device-tree overlay..."
if [[ ! -f "/boot/firmware/overlays/imx415.dtbo" ]]; then
    error "imx415.dtbo not found in /boot/firmware/overlays/.
    This overlay requires raspberrypi-firmware >= May 2025.
    Run: sudo apt-get install --only-upgrade raspberrypi-bootloader"
fi
info "Overlay: OK (/boot/firmware/overlays/imx415.dtbo)"

# ── 6. Configure config.txt ───────────────────────────────────────────────────
CONFIG="/boot/firmware/config.txt"
[[ ! -f "$CONFIG" ]] && CONFIG="/boot/config.txt"  # Bullseye fallback path

info "Configuring $CONFIG..."

# Detect CSI lane count from Pi model
PI_MODEL=$(cat /proc/device-tree/model 2>/dev/null || echo "unknown")
info "Pi model: $PI_MODEL"

# Remove any existing imx415 overlay lines to avoid duplicates
sudo sed -i '/dtoverlay=imx415/d' "$CONFIG"

if echo "$PI_MODEL" | grep -qi "Raspberry Pi 5"; then
    OVERLAY="dtoverlay=imx415,4lane"
    info "RPi 5 detected — using 4-lane mode (full 4K@30fps capability)"
else
    OVERLAY="dtoverlay=imx415"
    info "RPi 4/CM4 detected — using 2-lane mode (max practical: 1080p30 hw-encode, 4K15 sw-encode)"
fi

# Append to [all] section or just append at end
if grep -q "^\[all\]" "$CONFIG"; then
    sudo sed -i "/^\[all\]/a $OVERLAY" "$CONFIG"
else
    echo "$OVERLAY" | sudo tee -a "$CONFIG" > /dev/null
fi
info "Added: $OVERLAY to $CONFIG"

# ── 7. Verify camera is seen by libcamera ─────────────────────────────────────
info "Running libcamera camera list (camera must be physically connected)..."
if libcamera-hello --list-cameras 2>&1 | grep -qi "imx415"; then
    info "SUCCESS: IMX415 detected by libcamera!"
    libcamera-hello --list-cameras 2>&1 | grep -A5 -i "imx415"
else
    warn "IMX415 not detected yet. This is expected if you haven't rebooted since config.txt was changed."
    warn "Reboot and rerun: libcamera-hello --list-cameras"
fi

# ── 8. Done ───────────────────────────────────────────────────────────────────
echo ""
info "Setup complete. Summary:"
echo "  Kernel:        $(uname -r)"
echo "  libcamera:     $LC_VERSION"
echo "  Tuning file:   $IMX415_TUNING"
echo "  config.txt:    $OVERLAY added"
echo ""
info "Next steps:"
echo "  1. Reboot: sudo reboot"
echo "  2. In OpenHD UI: Camera → ARDUCAM → IMX415 4K"
echo "  3. Select resolution: 1280x720@60 (FPV) or 1920x1080@30 (HD)"
echo "  4. For 4K: enable 'Software Encode' in OpenHD settings first"
echo ""
warn "NOTE: 4K streaming requires software encode (high CPU). Use 1080p30 for FPV."
