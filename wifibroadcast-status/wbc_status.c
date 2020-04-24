// wbc_status by Rodizio, based on:
//
// first OpenVG program
// Anthony Starks (ajstarks@gmail.com)



#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



int main(int argc, char *argv[]) {

    char *text = argv[1];
    float offset = atoi(argv[2]);
    float fontscale = atoi(argv[3]);
    int background = atoi(argv[4]);

    int width, height;

    int widthpos = width / 2;
    int heightpos = height / offset;
    int fontpos = width / fontscale;

    //fprintf(stderr, "before init\n");


    /*
     * Graphics initialization
     */
    init(&width, &height);


    float a = 0;
    for (a = 0; a <= 1; a = a + 0.01) {
        Start(width, height);

        if (background == 1) {
            BackgroundRGB(0, 0, 0, 1);
        }

        Fill(255, 255, 255, a);

        TextMid(width / 2, height / offset, text, SansTypeface, width / fontscale);

        End();

        usleep(20000);
    }


    usleep(50000);


    for (a = 1; a >= 0; a = a - 0.02) {
        Start(width, height);

        if (background == 1) {
            BackgroundRGB(0, 0, 0, 1);
        }

        Fill(255, 255, 255, a);

        TextMid(width / 2, height / offset, text, SansTypeface, width / fontscale);

        End();

        usleep(40000);
    }


    if (background == 1) { 
        /*
         * We wait some more to make the splashscreen cover everything (15s)
         */

        usleep(15000000);
    }

    End();

    /*
     * Graphics cleanup
     */
    finish();


    exit(0);
}
