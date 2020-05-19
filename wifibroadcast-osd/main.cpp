/*
Copyright (c) 2015, befinitiv
Copyright (c) 2012, Broadcom Europe Ltd
modified by Samuel Brucksch https://github.com/SamuelBrucksch/wifibroadcast_osd
modified by Rodizio

All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the copyright holder nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <locale.h>

#include <tuple>
#include <string>
#include <iostream>
#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>


#include "render.h"
#include "telemetry.h"
#include "settings.h"


#include "frsky.h"
#include "ltm.h"
#include "mavlink.h"
#include "smartport.h"
#include "vot.h"





long long current_timestamp() {
    struct timeval te;

    gettimeofday(&te, NULL);

    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;

    return milliseconds;
}


fd_set set;


struct timeval timeout;


int main(int argc, char *argv[]) {
    fprintf(stderr,"OSD started\n=====================================\n\n");

    setpriority(PRIO_PROCESS, 0, 10);

    setlocale(LC_ALL, "en_GB.UTF-8");


    telemetry_file = fopen("/wbc_tmp/telemetrydowntmp.txt", "w+");

    if (!telemetry_file) {
        fprintf(stderr, "WARNING: cannot open telemetry file!");
        exit(1);
    }

    load_settings();

    /*
     * Mavlink maximum packet length
     */
    uint8_t buf[263];


    size_t n;

    long long fpscount_ts = 0;
    long long fpscount_ts_last = 0;
    int fpscount = 0;
    int fpscount_last = 0;
    int fps;

    int do_render = 0;
    int counter = 0;


    frsky_state_t fs;

    struct stat fdstatus;

    signal(SIGPIPE, SIG_IGN);

    char fifonam[100];
    sprintf(fifonam, "/root/telemetryfifo1");


    int readfd;
    readfd = open(fifonam, O_RDONLY | O_NONBLOCK);
    if (readfd == -1) {
        perror("ERROR: Could not open /root/telemetryfifo1");
        exit(EXIT_FAILURE);
    }

    if (fstat(readfd, &fdstatus) == -1) {
        perror("ERROR: fstat /root/telemetryfifo1");
        close(readfd);
        exit(EXIT_FAILURE);
    }


    fprintf(stderr,"OSD: Initializing sharedmem ...\n");

    telemetry_data_t_osd td;
    telemetry_init(&td);

    fprintf(stderr,"OSD: Sharedmem init done\n");


    //fprintf(stderr,"OSD: Initializing render engine ...\n");
    render_init();
    //fprintf(stderr,"OSD: Render init done\n");



    long long prev_time = current_timestamp();
    long long prev_time2 = current_timestamp();

    long long prev_cpu_time = current_timestamp();
    long long delta = 0;



    FILE *datarate_file = fopen("/tmp/DATARATE.txt", "r");

    if (datarate_file == NULL) {
        perror("ERROR: Could not open /tmp/DATARATE.txt");

        exit(EXIT_FAILURE);
    }

    double datarate = 0.0;

    fscanf(datarate_file, "%lf", &datarate);

    fclose(datarate_file);

    td.datarate = datarate;


    int cpuload_gnd = 0;
    int temp_gnd = 0;
    int undervolt_gnd = 0;

    FILE *fp;
    FILE *fp2;
    FILE *fp3;

    long double a[4], b[4];


    fp3 = fopen("/tmp/undervolt","r");
    if (NULL == fp3) {
        perror("ERROR: Could not open /tmp/undervolt");
        exit(EXIT_FAILURE);
    }

    fscanf(fp3, "%d", &undervolt_gnd);
    fclose(fp3);
    

    while (1) {
        FD_ZERO(&set);
        FD_SET(readfd, &set);

        timeout.tv_sec = 0;
        timeout.tv_usec = 50 * 1000;


        /*
         * Look for data for 50ms, then timeout
         */
        n = select(readfd + 1, &set, NULL, NULL, &timeout);

        if (n > 0) { 
            /*
             * If data there, read it and parse it
             */
            n = read(readfd, buf, sizeof(buf));

            /*
             * EOF
             */
            if (n == 0) { 
                continue; 
            }

            if (n <0 ) {
                perror("OSD: read");
                exit(-1);
            }


            if (FRSKY) {
                frsky_parse_buffer(&fs, &td, buf, n);
            } else if (LTM) {
                do_render = ltm_read(&td, buf, n);
            } else if (MAVLINK) {
                do_render = mavlink_read(&td, buf, n);
            } else if (SMARTPORT) {
                smartport_read(&td, buf, n);
            } else if (VOT) {
                vot_read(&td, buf, n);
            }

        }


        counter++;


        /* 
         * Only render if we have data that needs to be processed as quick as possible (attitude),
         * or if three iterations (~150ms) have passed without rendering
         */
        if ((do_render == 1) || (counter == 3)) {
            prev_time = current_timestamp();

            fpscount++;

            render(&td, cpuload_gnd, temp_gnd/1000, undervolt_gnd,fps);

            long long took = current_timestamp() - prev_time;

            do_render = 0;

            counter = 0;
        }


        delta = current_timestamp() - prev_cpu_time;
        

        /*
         * Read ground CPU/temp statistics once per second
         */
        if (delta > 1000) {
            prev_cpu_time = current_timestamp();

            fp2 = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

            fscanf(fp2, "%d", &temp_gnd);
            fclose(fp2);

            fp = fopen("/proc/stat", "r");

            fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
            fclose(fp);

            cpuload_gnd = (((b[0] + b[1] + b[2]) - (a[0] + a[1] + a[2])) / ((b[0] + b[1] + b[2] + b[3]) - (a[0] + a[1] + a[2] + a[3]))) * 100;

            fp = fopen("/proc/stat", "r");
            fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
            fclose(fp);
        }


        long long fpscount_timer = current_timestamp() - fpscount_ts_last;


        /*
         * Update the FPS every ~2 seconds
         */
        if (fpscount_timer > 2000) {
            fpscount_ts_last = current_timestamp();

            fps = (fpscount - fpscount_last) / 2;
            
            fpscount_last = fpscount;
        }
    }

    return 0;
}
