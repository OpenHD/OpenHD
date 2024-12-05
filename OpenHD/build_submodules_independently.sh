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

# For the github CIs
# Here we build all the submodules independently from each other to make sure
# no unwanted dependencies were introduced by accident during OpenHD development
cd ohd_common || exit
cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug || exit
cmake --build build_debug || exit

cd ../ohd_interface || exit
cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug || exit
cmake --build build_debug || exit

cd ../ohd_telemetry || exit
cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug || exit
cmake --build build_debug || exit

cd ../ohd_video || exit
cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug || exit
cmake --build build_debug || exit
