#!/bin/bash
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
else
    echo "One or more required configuration files are missing."
    exit 1
fi

# Display loaded configuration for debugging
echo "Distro: ${DISTRO}"
echo "Flavor: ${FLAVOR}"
echo "Custom: ${CUSTOM}"
echo "Arch: ${ARCH}"

# Install dependencies based on DISTRO or ARCH

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
