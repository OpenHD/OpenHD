# bin/bash

mkdir build_air_disabled
cd build_air_disabled

cmake .. -DENABLE_AIR=false
make -j4
