#!/bin/bash
set -e

# Check for kconfiglib
if ! python3 -c "import kconfiglib" 2>/dev/null; then
    echo "kconfiglib not found, installing..."
    python3 -m pip install kconfiglib
fi

# Run menuconfig
python3 -m menuconfig
echo "Configuration saved to .config"
echo "Updating generated files..."
python3 scripts/kconfig.py
