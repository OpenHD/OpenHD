@echo off
setlocal

rem Check for kconfiglib
python -c "import kconfiglib" 2>nul
if %errorlevel% neq 0 (
    echo kconfiglib not found, installing...
    pip install kconfiglib
)

rem Run menuconfig
python -m menuconfig
if %errorlevel% equ 0 (
    echo Configuration saved to .config
    echo Updating generated files...
    python scripts/kconfig.py
)

endlocal
