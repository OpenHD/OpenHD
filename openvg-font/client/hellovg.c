// first OpenVG program
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"

int main() {
	int width, height;
	char s[3];
	char hello1[] = {'H','e','j',',',' ','v', 0xc3, 0xa4,'r' , 'l','d' ,'e','n',0};
	char hello2[] = {'H','e','l','l',0xc3,0xb3,' ', 'V', 'i', 'l', 0xc3,0xa1,'g',0};
	char hello3[] = {'A','h','o','j',' ','s','v',0xc4,0x95,'t','e',0};
        setlocale(LC_ALL, "");
        InitShapes(&width, &height);				   // Graphics initialization

	Start(width, height);				   // Start the picture
	Background(0, 0, 0);				   // Black background
	Fill(44, 77, 232, 1);				   // Big blue marble
	Circle(width / 2, 0, width);			   // The "world"
	Fill(255, 255, 255, 1);				   // White text
	TextMid(width / 2, (height * 0.7), "hello, world", SansTypeface, width / 15);	// Greetings 
	TextMid(width / 2, (height * 0.5), hello1 , SansTypeface, width / 15);
	TextMid(width / 2, (height * 0.3), hello2 , SansTypeface, width / 15);
	TextMid(width / 2, (height * 0.1), hello3 , SansTypeface, width / 15);
	End();						   // End the picture

        WindowSaveAsPng("hello.png", 0, 0, 0, 0, 9);
        
	fgets(s, 2, stdin);				   // look at the pic, end with [RETURN]
	FinishShapes();					   // Graphics cleanup
	exit(0);
}
