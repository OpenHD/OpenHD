# bin/bash

# build and install the single OpenHD executable

./build_cmake.sh

cd build
sudo cmake --install .
