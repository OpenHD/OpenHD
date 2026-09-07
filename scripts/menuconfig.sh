#!/bin/bash

# Check for kconfiglib
python3 -c "import kconfiglib" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "kconfiglib not found, installing..."
    pip3 install kconfiglib
fi

# Run menuconfig
python3 -m menuconfig
if [ $? -eq 0 ]; then
    echo "Configuration saved to .config"
    echo "Updating generated files..."
    python3 scripts/kconfig.py
fi
