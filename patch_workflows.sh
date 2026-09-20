#!/bin/bash

# Ensure we use master instead of main for private video crypto submodule since it failed due to not finding main branch on the private repo
sed -i 's/git -C OpenHD\/ohd_interface\/video_crypto_private fetch origin main/git -C OpenHD\/ohd_interface\/video_crypto_private fetch origin master/g' ./.github/workflows/openhd_3_0_build_test.yml
sed -i 's/git -C OpenHD\/ohd_interface\/video_crypto_private checkout -f origin\/main/git -C OpenHD\/ohd_interface\/video_crypto_private checkout -f origin\/master/g' ./.github/workflows/openhd_3_0_build_test.yml

sed -i 's/git -C OpenHD\/ohd_interface\/video_crypto_private fetch origin main/git -C OpenHD\/ohd_interface\/video_crypto_private fetch origin master/g' ./.github/workflows/sonar.yml
sed -i 's/git -C OpenHD\/ohd_interface\/video_crypto_private checkout -f origin\/main/git -C OpenHD\/ohd_interface\/video_crypto_private checkout -f origin\/master/g' ./.github/workflows/sonar.yml
