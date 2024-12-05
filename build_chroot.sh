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
    QCOM=$(cat qcom.txt)
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