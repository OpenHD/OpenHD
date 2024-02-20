#!/bin/bash

# OpenHD clang-format checking.
# Step1: Generate list of all .cpp / .h / .hpp files of this project
#        (excluding subdirectories)
# Step 2: Run clang-format


function append_all_sources_headers() {
    # We use .h, .cpp and .hpp in OpenHD
    TMP_FILE_LIST="$(find "$1" | grep -E ".*(\.cpp|\.h|\.hpp)$")"
    #TMP_FILE_LIST+='\n'
    FILE_LIST+=$'\n'
    FILE_LIST+=$TMP_FILE_LIST
}


THIS_PATH="$(realpath "$0")"
THIS_DIR="$(dirname "$THIS_PATH")"


append_all_sources_headers "$THIS_DIR/ohd_common/inc"
append_all_sources_headers "$THIS_DIR/ohd_common/src"

append_all_sources_headers "$THIS_DIR/ohd_interface/inc"
append_all_sources_headers "$THIS_DIR/ohd_interface/src"

append_all_sources_headers "$THIS_DIR/ohd_telemetry/src"

append_all_sources_headers "$THIS_DIR/ohd_video/inc"
append_all_sources_headers "$THIS_DIR/ohd_video/src"

echo -e "Files found to format = \n\"\"\"\n$FILE_LIST\n\"\"\""

# Format each file.
# - NB: do NOT put quotes around `$FILE_LIST` below or else the `clang-format` command will
#   mistakenly see the entire blob of newline-separated file names as a SINGLE file name instead
#   of as a new-line separated list of *many* file names!
clang-format --dry-run --Werror --verbose -i --style=file $FILE_LIST

if [ "$?" -eq "0" ]; then
  echo "Everything formatted correctly"
else
  echo "There are formatting errors ! Please fix first."
  exit 1 # terminate and indicate error
fi