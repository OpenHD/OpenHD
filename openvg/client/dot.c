#include <stdio.h>
#include <time.h>
#include "shapes.h"

VGPaint myRed;
VGPaint myWhite;

void allocData()
{
    myWhite = Paint(color_white, 1.0f);
    myRed = Paint(color_red, 1.0f);
}

int count = 50000;

void drawScene()
{
    int i;

    WindowClear();
    FillPaint(myWhite); // Set the fill to the white we defined

    StrokePaint(myRed);
    StrokeWidth(1.0f);  // Dot doesn't use stroke so we shouldn't see red

    for(i = 0; i < count; i++) {
        float xp = (float)(rand() & 0x3ff);
        float yp = (float)(rand() & 0x1ff);
        Dot(xp, yp, rand() & 1 ? true:false);
    }
    
    End();
    WindowSaveAsPng("win.png", 0,0, 640, 640, 9);
}

void freeData()
{
    DeletePaint(myRed);
    DeletePaint(myWhite);
}

int main(int argc, char *argv[])
{
    time_t time_s, time_f;
    double time_diff;
    int w, h, i;

    if (argc == 2) {
        count = atoi(argv[1]);
        if (count <= 0)
            count = 50000;
    }
    
    InitShapes(&w, &h);
    allocData();
    time_s = time(NULL);
    drawScene();
    time_f = time(NULL);
    time_diff = difftime(time_f, time_s);
    if (time_diff > 0.0)
        printf("%d points in %f seconds (%d points/s)\n", count, time_diff, count/(int)time_diff);

    char c[3];
    fgets(c, 2, stdin);
    freeData();
    FinishShapes();
    return 0;
}
