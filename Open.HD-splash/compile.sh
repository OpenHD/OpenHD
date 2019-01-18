#!/usr/bin/env bash
gcc -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads main.c -o splash -lshapes
