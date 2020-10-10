#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h> 


#include "openhd-status.hpp"

int main(int argc, char **argv) {
    if (argc < 3) {
        exit(1);
    }

    int level = atoi(argv[2]);

    const char* message = argv[1];

    status_message((STATUS_LEVEL)level, message);
    
    return 0;
}
