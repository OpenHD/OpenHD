#!/bin/bash
set -e

# Patch rockchip stream
python3 patch_mpp.py

# Patch OHDMainComponent
python3 patch_storage_fix.py
