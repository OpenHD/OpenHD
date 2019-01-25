// first OpenVG program
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"

Fontinfo myfont;

int main(int argc, char *argv[]) {
	int width, height;
	VGfloat font_width, font_height;
	char s[3];
	char hello1[] = {'H','e','j',',',' ','v', 0xc3, 0xa4,'r' , 'l','d' ,'e','n',0};
	char hello2[] = {'H','e','l','l',0xc3,0xb3,' ', 'V', 'i', 'l', 0xc3,0xa1,'g',0};
	char hello3[] = {'A','h','o','j',' ','s','v',0xc4,0x95,'t','e',0};
	setlocale(LC_CTYPE, "");
	InitShapes(&width, &height);				   // Graphics
                                                           // initialization
        if (vg_error) {
                FinishShapes();
                return -1;
        }
     
	Start(width, height);				   // Start the picture

	// Dynamically load a font, pass the font name on the command
	// line, else "helvetica" will be used (which on the RPi will
	// load "Nimbus Sans L:Regular").
	// If you know the font filename (including full path) you can
	// use LoadTTFFile(filename);
        const char *fontname = (argc == 2 ? argv[1] : "DejaVuSerif");
        myfont = LoadTTF(fontname);
        if (!myfont) {
                printf("Failed to load \"%s\" font.\n", fontname);
                myfont = SerifTypeface;
        }
        
	Background(0, 0, 0);				   // Black background
	Translate(200, 200);
	Scale(0.5, 0.5);
	Rotate(45);
	Fill(44, 77, 232, 1);				   // Big blue marble
	Circle(width / 2, 0, width);			   // The "world"
	Fill(255, 255, 255, 1);				   // White text
	Stroke(255, 0, 0, 1);
	StrokeWidth(1);
	TextMid(width / 2, (height * 0.7), "hello, world", SerifTypeface, width / 15);	// Greetings 
	TextMid(width / 2, (height * 0.5), hello1 , SerifTypeface, width / 15);
	TextMid(width / 2, (height * 0.3), hello2 , SerifTypeface, width / 15);
	TextMid(width / 2, (height * 0.1), hello3 , SerifTypeface, width / 15);

	vgLoadIdentity();
	StrokeWidth(0);

	Text(20, 20, "`1234567890-=qwertyuiop[]asdfghjkl;'#\\zxcvbnm,./", myfont, 24);
	Text(20, 50, "¬!\"£$%^&*()_+QWERTYUIOP{}ASDFGHJKL:@~|ZXCVBNM<>?", myfont, 24);
                // TextLineHeight() returns the font's suggested
                // height to the next baseline. Built-in fonts and
                // those loaded with the old method have this
                // defaulting to TextHeight+TextDepth+1
                
                // Test Kerning. Only allowable on the new font
                // system, and if the font has kerning data. Safe to
                // try setting on old fonts - it ignores it.
        FontKerning(myfont, 0);
        font_width = TextWidth("WAW", myfont, 48);
        font_height = TextLineHeight(myfont, 48);
        Text(20, 80, "WAW", myfont, 48);
        FontKerning(myfont, 1);
        Text(20, 80+font_height, "WAW", myfont, 48);
        Stroke(color_green, 1);
        StrokeWidth(1);
        Line(20+font_width, 80, 20+font_width, 80+2*font_height);
        End();						   // End the picture
        if (vg_error)
                printf("oops, an error occured in libshapes (%d).\n", vg_error);
        
	fgets(s, 2, stdin);				   // look at the pic, end with [RETURN]

                // unloadfont safely handles new fonts, it will call
                // the correct UnloadTTF() function for you.
        if (myfont != SerifTypeface)
                UnloadFont(myfont);
        
        FinishShapes();					   // Graphics cleanup
	exit(0);
}
