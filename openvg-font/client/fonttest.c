/*
 * fonttest: New font system for libshapes
 * Paeryn (paeryn8@gmail.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "VG/openvg.h"
#include "fontinfo.h"
#include "shapes.h"

int main(int argc, char *argv[])
{
        setlocale(LC_CTYPE, "");

        int width, height;
        InitShapes(&width, &height);

        char *fontname = argc > 1 ? argv[1] : "DejaVuSerif";
        char *fontstyle = argc > 2 ? argv[2] : "Regular";
        char fname[strlen(fontname) + strlen(fontstyle) + 2];
        strcpy(fname, fontname);
        strcat(fname, ":");
        strcat(fname, fontstyle);

        Start(width, height);
        Background(color_black);
        Fill(color_white, 1.0f);
        
        Fontinfo myfont = LoadTTF(fname);
        if (!myfont) {
                puts("Error loading font");
                FinishShapes();
                return 1;
        }

        char buffer[255];
        snprintf(buffer, 255, "Font: %s, Style: %s, Kerning %s available", myfont->Name, myfont->Style, myfont->Kerning ? "is" : "is not");
        Text(20, 20, buffer, SerifTypeface, 20);
        char *string = "Without kerning: ";
        VGfloat start_pos = 10 + TextWidth(string, SerifTypeface, 32);
        VGfloat line_height = TextLineHeight(myfont, 32);
        Text(10, 100, string, SerifTypeface, 32);
        Text(10, 100+line_height, "With kerning:", SerifTypeface, 32);

                // Test string, kerning should have a big effect on
                // these letters.
        string = "WAWAWAWA";
                //Get the width of the string with kerning.
        VGfloat textwidth = TextWidth(string, myfont, 32);
        
        FontKerning(myfont, 0); // Turn kerning off
        Text(start_pos, 100, string, myfont, 32);
        FontKerning(myfont, 1); // Turn kerning on if available
        Text(start_pos, 100+line_height, string, myfont, 32);

                // Draw a box around the strings, the non-kern string
                // should extend beyond the box if kerning is working.
        StrokeWidth(1);
        Stroke(color_green, 1.0f);
        RectOutline(start_pos, 100, textwidth, line_height*2);
        End();
        UnloadFont(myfont);
        fgetc(stdin);
        FinishShapes();
        return 0;
}
