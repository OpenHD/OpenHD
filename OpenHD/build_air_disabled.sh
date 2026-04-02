# bin/bash

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
###############################################################################

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mkdir build_air_disabled
cd build_air_disabled

source "${SCRIPT_DIR}/scripts/resolve_artosyn_sdk.sh"
resolve_artosyn_sdk
cmake .. \
  -DENABLE_AIR=false \
  -DARTOSYN_SDK_ROOT="${ARTOSYN_SDK_ROOT}" \
  -DARTOSYN_SDK_LIB="${ARTOSYN_SDK_LIB}"
make -j4
