#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <libgen.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"

Fontinfo myfont;

int main(int argc, char *argv[]) {
	char *fontpaths[] = {
		"/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
		"/usr/share/fonts/truetype/droid/DroidSerif-Bold.ttf",
		"/usr/share/fonts/truetype/droid/DroidSerif-Italic.ttf",
		"/usr/share/fonts/truetype/droid/DroidSansArmenian.ttf",
		"/usr/share/fonts/truetype/droid/DroidNaskh-Bold.ttf",
		"/usr/share/fonts/truetype/droid/DroidSerif-BoldItalic.ttf",
		"/usr/share/fonts/truetype/droid/DroidSansJapanese.ttf",
		"/usr/share/fonts/truetype/droid/DroidSansArabic.ttf",
		"/usr/share/fonts/truetype/droid/DroidSans-Bold.ttf",
		"/usr/share/fonts/truetype/droid/DroidSansMono.ttf",
		"/usr/share/fonts/truetype/droid/DroidNaskhUI-Regular.ttf",
		"/usr/share/fonts/truetype/droid/DroidKufi-Bold.ttf",
		"/usr/share/fonts/truetype/droid/DroidSans.ttf",
		"/usr/share/fonts/truetype/droid/DroidSansEthiopic-Bold.ttf",
		"/usr/share/fonts/truetype/droid/DroidSerif-Regular.ttf",
		"/usr/share/fonts/truetype/droid/DroidNaskh-Regular.ttf",
		"/usr/share/fonts/truetype/droid/DroidKufi-Regular.ttf",
		"/usr/share/fonts/truetype/droid/DroidSansEthiopic-Regular.ttf",
		"/usr/share/fonts/truetype/droid/DroidSansGeorgian.ttf",
		"/usr/share/fonts/truetype/droid/DroidSansHebrew-Bold.ttf",
		"/usr/share/fonts/truetype/droid/DroidSansHebrew-Regular.ttf",
		"/usr/share/fonts/truetype/freefont/FreeSans.ttf",
		"/usr/share/fonts/truetype/freefont/FreeSerif.ttf",
		"/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf",
		"/usr/share/fonts/truetype/freefont/FreeMonoBoldOblique.ttf",
		"/usr/share/fonts/truetype/freefont/FreeSansBoldOblique.ttf",
		"/usr/share/fonts/truetype/freefont/FreeSansOblique.ttf",
		"/usr/share/fonts/truetype/freefont/FreeSerifBoldItalic.ttf",
		"/usr/share/fonts/truetype/freefont/FreeMono.ttf",
		"/usr/share/fonts/truetype/freefont/FreeMonoOblique.ttf",
		"/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
		"/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf",
		"/usr/share/fonts/truetype/freefont/FreeSerifItalic.ttf",
		"/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans-Bold.ttf",
		"/usr/share/fonts/truetype/ttf-dejavu/DejaVuSerif-Bold.ttf",
		"/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono-Bold.ttf",
		"/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf",
		"/usr/share/fonts/truetype/ttf-dejavu/DejaVuSerif.ttf",
		"/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSerifCondensed-Italic.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Oblique.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSerifCondensed.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSansMono-BoldOblique.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans-ExtraLight.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans-Oblique.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Oblique.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-BoldOblique.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans-BoldOblique.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSerif-BoldItalic.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSerifCondensed-BoldItalic.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSerifCondensed-Bold.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSerif-Italic.ttf",
		"/usr/share/fonts/truetype/roboto/Roboto-Regular.ttf",
		"/usr/share/fonts/truetype/roboto/Roboto-Italic.ttf",
		"/usr/share/fonts/truetype/roboto/RobotoCondensed-Italic.ttf",
		"/usr/share/fonts/truetype/roboto/RobotoCondensed-Regular.ttf",
		"/usr/share/fonts/truetype/roboto/Roboto-Bold.ttf",
		"/usr/share/fonts/truetype/roboto/RobotoCondensed-Bold.ttf",
		"/usr/share/fonts/truetype/roboto/Roboto-Thin.ttf",
		"/usr/share/fonts/truetype/roboto/Roboto-BoldItalic.ttf",
		"/usr/share/fonts/truetype/roboto/RobotoCondensed-BoldItalic.ttf",
		"/usr/share/fonts/truetype/roboto/Roboto-ThinItalic.ttf",
		"/usr/share/fonts/truetype/roboto/Roboto-LightItalic.ttf",
		"/usr/share/fonts/truetype/roboto/Roboto-Light.ttf"
	};
	int width, height, i, beginlist, endlist, fontsize;
	char s[3];

	if (argc != 3) {
		fprintf(stderr, "specify the begin and end of the list (0-71)\n");
		exit(1);
	}
	beginlist = atoi(argv[1]);
	endlist = atoi(argv[2]);

	if (beginlist < 0 || endlist > 71) {
                fprintf(stderr, "begin and end must be between 0, 71\n");
		exit(2);
	}
	if (beginlist >= endlist) {
		fprintf(stderr, "begin must be less than end\n");
		exit(3);
	}

	InitShapes(&width, &height);
	setlocale(LC_CTYPE, "");

                // Unload the default fonts else the Free* fonts won't
                // load. Set the variables to NULL after freeing them
                // otherwise finish() will try to free them too.
        UnloadFont(SansTypeface);
        SansTypeface = NULL;
        UnloadFont(SerifTypeface);
        SerifTypeface = NULL;
        UnloadFont(MonoTypeface);
        MonoTypeface = NULL;
        
	VGfloat x, y, top, bottom, left, colwidth, vspace;
	fontsize = 20;
	colwidth = (VGfloat) fontsize *30;
	left = (VGfloat) width *0.025;
	bottom = (VGfloat) height *0.05; // Lowered to fit all on screen
	top = (VGfloat) height *0.90;
	colwidth = (VGfloat) fontsize *30;
	vspace = (VGfloat) fontsize *2.0;

	x = left;
	y = top;

	Start(width, height);
	Background(255, 255, 255);
	Fill(0, 0, 0, 1);
	for (i = beginlist; i <= endlist; i++) {
		printf("loading %s\n", fontpaths[i]);
		myfont = LoadTTFFile(fontpaths[i]);
		if (!myfont) {
			printf("load FAILED\n");
			continue;
		}
		Text(x, y, basename(fontpaths[i]), myfont, fontsize);
		UnloadFont(myfont);
                End(); // This takes a while - update after each one.
                y = y - vspace;
		if (y < bottom) {
			y = top;
			x = x + colwidth;
		}
	}
	End();
	fgets(s, 2, stdin);
	FinishShapes();
	exit(0);
}
