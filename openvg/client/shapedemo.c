//
// shapedemo: testbed for OpenVG APIs
// Anthony Starks (ajstarks@gmail.com)
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"

// randcolor returns a random number 0..255
unsigned int randcolor() {
	return (unsigned int)(drand48() * 255.0);
}

// randf returns a floating point number bounded by n
VGfloat randf(int n) {
	return drand48() * n;
}

// coordpoint marks a coordinate, preserving a previous color
void coordpoint(VGfloat x, VGfloat y, VGfloat size, VGfloat pcolor[4]) {
	Fill(128, 0, 0, 0.3);
	Circle(x, y, size);
	SetFill(pcolor);
}

// grid draws a grid
void grid(VGfloat x, VGfloat y, int n, int w, int h) {
	VGfloat ix, iy;
	Stroke(128, 128, 128, 0.5);
	StrokeWidth(2);
	for (ix = x; ix <= x + w; ix += n) {
		Line(ix, y, ix, y + h);
	}

	for (iy = y; iy <= y + h; iy += n) {
		Line(x, iy, x + w, iy);
	}
}
// gradient demos linear and radial gradients
void gradient(int width, int height) {
	VGfloat x1, y1, x2, y2;
	VGfloat w = (VGfloat)width;
	VGfloat h = (VGfloat)height;
	VGfloat dotcolor[4] = {0, 0, 0, 0.3};


	VGfloat stops[] = {
//		0.0, 1.0, 1.0, 1.0, 1.0,
//		0.5, 0.5, 0.5, 0.5, 1.0,
//		1.0, 0.0, 0.0, 0.0, 1.0
		0.1, 0.0, 0.0, 0.0, 0.01,
		0.2, 0.0, 0.0, 0.0, 0.1,
		0.5, 0.0, 0.0, 0.0, 0.5,
		1.0, 0.0, 0.0, 0.0, 1.0
	};


	x1 = w/8;
	x2 = (w*3)/8;
	y1 = h/3;
	y2 = (h*2)/3;

//	x1 = 1400;
//	y1 = 950;

//	x2 = 1919;
//	y2 = 1079;

	Start(w, h);

//	Background(128, 128, 128);


	FillLinearGradient(x1, y1, x2, y2, stops, 4);
	Rect(x1, y1, x2-x1, y2-y1);


//	FillRadialGradient(cx, cy, fx, fy, r, stops, 3);
//	Circle(cx, cy, r);

//	RGBA(.5, 0, 0, 0.3, dotcolor);
//	SetFill(dotcolor);
//	Circle(x1, y1, 10);
//	Circle(x2, y2, 10);
//	Circle(cx, cy, 10);
//	Circle(cx+r/2, cy, 10);
//	Circle(fx, fy, 10);
	
	RGB(0,0,0,dotcolor);
	SetFill(dotcolor);
	TextMid(x1, y1-20, "(x1, y1)", SansTypeface, 18);
	TextMid(x2, y2+10, "(x2, y2)", SansTypeface, 18);

	TextMid(x1+((x2-x1)/2), h/6, "Linear Gradient", SansTypeface, 36);
//	TextMid(cx, h/6, "Radial Gradient", SansTypeface, 36);
	
	
	End();

}
// makepi draws the Raspberry Pi
void makepi(VGfloat x, VGfloat y, int w, int h) {
	// dimensions
	VGfloat socw = h / 5,
	    compw = h / 5,
	    cjw = h / 10,
	    cjh = h / 8,
	    audw = h / 5,
	    aujw = h / 10,
	    aujh = cjh / 2,
	    hdw = w / 6,
	    hdh = w / 10,
	    gpw = w / 3,
	    gph = h / 8,
	    pw = h / 10,
	    usw = w / 5,
	    ush = h / 5,
	    etw = w / 5,
	    eth = h / 5,
	    sdw = w / 6, sdh = w / 4, offset = (w / 2) / 10, w34 = (w * 3) / 4, w2 = w / 2, h2 = h / 2, h40 = (h * 2) / 5;

	Fill(0, 128, 0, 1);
	Rect(x, y, w, h);				   // board

	Fill(255, 255, 0, 1);
	Rect(x + w2, (y + h) - compw, compw, compw);	   // composite
	Fill(192, 192, 192, 1);
	Rect(x + w2 + (cjw / 2), y + h, cjw, cjh);	   // composite jack

	Fill(0, 0, 0, 1);
	Rect(x + w34, y + h - audw, audw, audw);	   // audio
	Rect(x + w34 + (aujw / 2), y + h, aujw, aujh);	   // audio jack

	Fill(192, 192, 192, 1);
	Rect(x + w2, y, hdw, hdh);			   // HDMI
	Rect((x + w) - etw, y, etw, eth);		   // Ethernet
	Rect((x + w + offset) - usw, y + h40, usw, ush);   // USB
	Rect(x, y, pw, pw);				   // Power

	Fill(0, 0, 0, 1);
	Rect(x + (w * 2) / 5, y + h40, socw, socw);	   // SoC
	Rect(x, (y + h) - gph, gpw, gph);		   // GPIO
	Fill(0, 0, 255, 1);
	Rect(x - sdw, (y + h2) - (sdh / 2), sdw, sdh);	   // SD card
}

// raspi draws the raspberry pi, scaled to the screen dimensions
void raspi(int w, int h, char *s) {
	VGfloat midx = w / 2, midy = h / 2;
	int rw = midx, rh = (rw * 2) / 3, fontsize = w * 0.03;

	Start(w, h);
	Background(255, 255, 255);
	makepi(midx - (rw / 2), midy - (rh / 2), rw, rh);
	Fill(128, 0, 0, 1);
	TextMid(midx, midy - (rh / 2) - (fontsize * 2), s, SansTypeface, fontsize);
	End();
}

typedef struct {
	Fontinfo font;
	VGfloat tw;
	int fontsize;
} FW;

void textbbox(char *s, Fontinfo f, int pointsize) {

}

// adjust the font to fit within a width
void fitwidth(int width, int adj, char *s, FW * f) {
	f->tw = TextWidth(s, f->font, f->fontsize);
	while (f->tw > width) {
		f->fontsize -= adj;
		f->tw = TextWidth(s, f->font, f->fontsize);
	}
}

// testpattern shows a test pattern 
void testpattern(int w, int h, char *s) {
	VGfloat midx, midy1, midy2, midy3;
	int fontsize = 256, h2 = h / 2;
	FW tw1 = { SansTypeface, 0, fontsize };
	FW tw2 = { SansTypeface, 0, fontsize };
	FW tw3 = { SansTypeface, 0, fontsize };

	Start(w, h);

	// colored squares in the corners
	Fill(255, 0, 0, 1);
	Rect(0, 0, 100, 100);
	Fill(0, 255, 0, 1);
	Rect(0, h - 100, 100, 100);
	Fill(0, 0, 255, 1);
	Rect(w - 100, 0, 100, 100);
	Fill(128, 128, 128, 1);
	Rect(w - 100, h - 100, 100, 100);

	// for each font, (Sans, Serif, Mono), adjust the string to the w
	fitwidth(w, 20, s, &tw1);
	fitwidth(w, 20, s, &tw2);
	fitwidth(w, 20, s, &tw3);

	midx = w / 2;

	// Adjust the baselines to be medial
	midy1 = h2 + 20 + (tw1.fontsize) / 2;
	midy2 = h2 - (tw2.fontsize) / 2;
	midy3 = h2 - 20 - tw2.fontsize - (tw3.fontsize) / 2;

	Fill(128, 128, 128, 1);
	TextMid(midx, midy1, s, tw1.font, tw1.fontsize);
	Fill(128, 0, 0, 1);
	TextMid(midx, midy2, s, tw2.font, tw2.fontsize);
	Fill(0, 0, 128, 1);
	TextMid(midx, midy3, s, tw3.font, tw3.fontsize);
	End();
}

// textlines writes lines of text
void textlines(VGfloat x, VGfloat y, char *s[], Fontinfo f, int fontsize, VGfloat leading) {
	int i;
	for (i = 0;; i++) {
		if (s[i] == NULL) {
			break;
		}
		TextMid(x, y, s[i], f, fontsize);
		y -= leading;
	}
}

// tb draws a block of text
void tb(int w, int h) {
	char *para[] = {
		"For lo,",
		"the winter is past,",
		"the rain is over and gone",
		"the flowers appear on the earth;",
		"the time for the singing of birds is come,",
		"and the voice of the turtle is heard in our land",
		NULL
	};

	VGfloat tmargin = w * 0.5, lmargin = w * 0.10, top = h * .9, mid = h * .6, bot = h * .3;

	int fontsize = 24, leading = 40, lfontsize = fontsize * 2, midb = ((leading * 2) + (leading / 2)) - (lfontsize / 2);

	Start(w, h);
	Fill(49, 79, 79, 1);
	textlines(tmargin, top, para, SansTypeface, fontsize, leading);
	textlines(tmargin, mid, para, SansTypeface, fontsize, leading);
	textlines(tmargin, bot, para, SansTypeface, fontsize, leading);
	Text(lmargin, top - midb, "Serif", SansTypeface, lfontsize);
	Text(lmargin, mid - midb, "Sans", SansTypeface, lfontsize);
	Text(lmargin, bot - midb, "Mono", SansTypeface, lfontsize);
	End();
}

void imagetest(int w, int h) {
	int imgw = 422, imgh = 238;
	VGfloat cx = (w / 2) - (imgw / 2), cy = (h / 2) - (imgh / 2);
	VGfloat ulx = 0, uly = h - imgh;
	VGfloat urx = w - imgw, ury = uly;
	VGfloat llx = 0, lly = 0;
	VGfloat lrx = urx, lry = lly;
	Start(w, h);
	Background(0, 0, 0);
	Image(cx, cy, imgw, imgh, "desert1.jpg");
	Image(ulx, uly, imgw, imgh, "desert2.jpg");
	Image(urx, ury, imgw, imgh, "desert3.jpg");
	Image(llx, lly, imgw, imgh, "desert4.jpg");
	Image(lrx, lry, imgw, imgh, "desert5.jpg");
	End();
}

void imagetable(int w, int h) {
	int imgw = 422, imgh = 238;
	char *itable[] = {
		"desert0.jpg",
		"desert1.jpg",
		"desert2.jpg",
		"desert3.jpg",
		"desert4.jpg",
		"desert5.jpg",
		"desert6.jpg",
		"desert7.jpg",
		NULL
	};
	VGfloat left = 50.0;
	VGfloat bot = h - imgh - 50.0;
	VGfloat gutter = 50.0;

	VGfloat x = left;
	VGfloat y = bot;
	int i;
	Start(w, h);
	Background(0, 0, 0);
	for (i = 0; itable[i] != NULL; i++) {
		Image(x, y, imgw, imgh, itable[i]);
		Fill(255, 255, 255, 0.3);
		Rect(x, y, imgw, 32);
		Fill(0, 0, 0, 1);
		TextMid(x + (imgw / 2), y + 10, itable[i], SansTypeface, 16);

		x += imgw + gutter;
		if (x > w) {
			x = left;
			y -= imgh + gutter;
		}
	}
	y = h * 0.1;
	Fill(128, 128, 128, 1);
	TextMid(w / 2, 100, "Joshua Tree National Park", SansTypeface, 48);
	End();
}

// fontrange shows a range of fonts
void fontrange(int w, int h) {
	int *s, sizes[] = { 6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 21, 24, 36, 48, 60, 72, 96, 0 };
	VGfloat x, y = h / 2, spacing = 50, s2 = spacing / 2, len, lx;
	char num[4];

	Start(w, h);
	Background(255, 255, 255);

	// compute the length so we can center
	for (len = 0, s = sizes; *s; s++) {
		len += *s + spacing;
	}
	len -= spacing;
	lx = (w / 2) - (len / 2);			   // center point

	// for each size, display a character and label
	for (x = lx, s = sizes; *s; s++) {
		Fill(128, 0, 0, 1);
		TextMid(x, y, "a", SansTypeface, *s);
		Fill(128, 128, 128, 1);
		snprintf(num, 3, "%d", *s);
		TextMid(x, y - spacing, num, SansTypeface, 16);
		x += *s + spacing;
	}
	// draw a line below the characters, a curve above
	x -= spacing;
	Stroke(150, 150, 150, 0.5);
	StrokeWidth(2);
	Line(lx, y - s2, x, y - s2);
	Fill(255, 255, 255, 1);
	Qbezier(lx, y + s2, x, y + s2, x, y + (spacing * 3));
	End();
}

// refcard shows a reference card of shapes
void refcard(int width, int height) {
	char *shapenames[] = {
		"Circle",
		"Ellipse",
		"Rectangle",
		"Rounded Rectangle",
		"Line",
		"Polyline",
		"Polygon",
		"Arc",
		"Quadratic Bezier",
		"Cubic Bezier",
		"Image"
	};
	VGfloat shapecolor[4];
	RGB(202, 225, 255, shapecolor);
	VGfloat top = height * .95, sx = 500, sy = top, sw = width * .05, sh = height * .045, dotsize = 7, spacing = 2.0;

	int i, ns = sizeof(shapenames) / sizeof(char *), fontsize = height * .033;
	Start(width, height);
	sx = width * 0.10;

	Fill(128, 0, 0, 1);
	TextEnd(width - 20, height / 2, "OpenVG on the Raspberry Pi", SansTypeface, fontsize + (fontsize / 2));
	Fill(0, 0, 0, 1);
	for (i = 0; i < ns; i++) {
		Text(sx + sw + sw / 2, sy, shapenames[i], SansTypeface, fontsize);
		sy -= sh * spacing;
	}
	sy = top;
	VGfloat cx = sx + (sw / 2), ex = sx + sw;
	SetFill(shapecolor);
	Circle(cx, sy, sw);
	coordpoint(cx, sy, dotsize, shapecolor);
	sy -= sh * spacing;
	Ellipse(cx, sy, sw, sh);
	coordpoint(cx, sy, dotsize, shapecolor);
	sy -= sh * spacing;
	Rect(sx, sy, sw, sh);
	coordpoint(sx, sy, dotsize, shapecolor);
	sy -= sh * spacing;
	Roundrect(sx, sy, sw, sh, 20, 20);
	coordpoint(sx, sy, dotsize, shapecolor);
	sy -= sh * spacing;

	StrokeWidth(1);
	Stroke(204, 204, 204, 1);
	Line(sx, sy, ex, sy);
	coordpoint(sx, sy, dotsize, shapecolor);
	coordpoint(ex, sy, dotsize, shapecolor);
	sy -= sh;

	VGfloat px[5] = { sx, sx + (sw / 4), sx + (sw / 2), sx + ((sw * 3) / 4), sx + sw };
	VGfloat py[5] = { sy, sy - sh, sy, sy - sh, sy };

	Polyline(px, py, 5);
	coordpoint(px[0], py[0], dotsize, shapecolor);
	coordpoint(px[1], py[1], dotsize, shapecolor);
	coordpoint(px[2], py[2], dotsize, shapecolor);
	coordpoint(px[3], py[3], dotsize, shapecolor);
	coordpoint(px[4], py[4], dotsize, shapecolor);
	sy -= sh * spacing;

	py[0] = sy;
	py[1] = sy - sh;
	py[2] = sy - (sh / 2);
	py[3] = py[1] - (sh / 4);
	py[4] = sy;
	Polygon(px, py, 5);
	sy -= (sh * spacing) + sh;

	Arc(sx + (sw / 2), sy, sw, sh, 0, 180);
	coordpoint(sx + (sw / 2), sy, dotsize, shapecolor);
	sy -= sh * spacing;

	VGfloat cy = sy + (sh / 2), ey = sy;
	Qbezier(sx, sy, cx, cy, ex, ey);
	coordpoint(sx, sy, dotsize, shapecolor);
	coordpoint(cx, cy, dotsize, shapecolor);
	coordpoint(ex, ey, dotsize, shapecolor);
	sy -= sh * spacing;

	ey = sy;
	cy = sy + sh;
	Cbezier(sx, sy, cx, cy, cx, sy, ex, ey);
	coordpoint(sx, sy, dotsize, shapecolor);
	coordpoint(cx, cy, dotsize, shapecolor);
	coordpoint(cx, sy, dotsize, shapecolor);
	coordpoint(ex, ey, dotsize, shapecolor);

	sy -= (sh * spacing * 1.5);
	Image(sx, sy, 100, 100, "starx.jpg");

	End();
}

// rotext draws text, rotated around the center of the screen, progressively faded
void rotext(int w, int h, int n, char *s) {
	VGfloat fade = (100.0 / (VGfloat) n) / 100.0;
	VGfloat deg = 360.0 / n;
	VGfloat x = w / 2, y = h / 2;
	VGfloat alpha = 1.0;	// start solid
	int i, size = w / 8;

	Start(w, h);
	Background(0, 0, 0);
	Translate(x, y);
	for (i = 0; i < n; i++) {
		Fill(255, 255, 255, alpha);
		Text(0, 0, s, SansTypeface, size);
		alpha -= fade;				   // fade
		size += n;				   // enlarge
		Rotate(deg);
	}
	End();
}

// rseed seeds the random number generator from the random device
void rseed(void) {
	unsigned char d[sizeof(long int)];
	long int s;
	int fd;

	// read bytes from the random device,
	// pack them into a long int.
	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		srand48(1);
		return;
	}
	read(fd, (void *)d, (size_t) sizeof(long int));
	s = d[3] | (d[2] << 8) | (d[1] << 16) | (d[0] << 24);
	srand48(s);
	close(fd);
}

// rshapes draws shapes with random colors, strokes, and sizes. 
void rshapes(int width, int height, int n) {
	int i, j, np = 10;
	VGfloat sx, sy, cx, cy, px, py, ex, ey, pox, poy;
	VGfloat polyx[np], polyy[np];
	rseed();
	Start(width, height);
	for (i = 0; i < n; i++) {
		Fill(randcolor(), randcolor(), randcolor(), drand48());
		Ellipse(randf(width), randf(height), randf(200), randf(100));
		Circle(randf(width), randf(height), randf(100));
		Rect(randf(width), randf(height), randf(200), randf(100));
		Arc(randf(width), randf(height), randf(200), randf(200), randf(360), randf(360));

		sx = randf(width);
		sy = randf(height);
		Stroke(randcolor(), randcolor(), randcolor(), 1);
		StrokeWidth(randf(5));
		Line(sx, sy, sx + randf(200), sy + randf(100));
		StrokeWidth(0);

		sx = randf(width);
		sy = randf(height);
		ex = sx + randf(200);
		ey = sy;
		cx = sx + ((ex - sx) / 2.0);
		cy = sy + randf(100);
		Qbezier(sx, sy, cx, cy, ex, ey);

		sx = randf(width);
		sy = randf(height);
		ex = sx + randf(200);
		ey = sy;
		cx = sx + ((ex - sx) / 2.0);
		cy = sy + randf(100);
		px = cx;
		py = sy - randf(100);
		Cbezier(sx, sy, cx, cy, px, py, ex, ey);

		pox = randf(width);
		poy = randf(height);
		for (j = 0; j < np; j++) {
			polyx[j] = pox + randf(200);
			polyy[j] = poy + randf(100);
		}
		Polygon(polyx, polyy, np);

		pox = randf(width);
		poy = randf(height);
		for (j = 0; j < np; j++) {
			polyx[j] = pox + randf(200);
			polyy[j] = poy + randf(100);
		}
		Polyline(polyx, polyy, np);
	}
	Fill(128, 0, 0, 1);
	Text(20, 20, "OpenVG on the Raspberry Pi", SansTypeface, 32);
	End();
}

// sunearth shows the relative sizes of the sun and the earth
void sunearth(int w, int h) {
	VGfloat sun, earth, x, y;
	int i;

	rseed();
	Start(w, h);
	Background(0, 0, 0);
	Fill(255, 255, 255, 1);
	for (i = 0; i < w / 4; i++) {
		x = randf(w);
		y = randf(h);
		Circle(x, y, 2);
	}
	earth = (VGfloat) w *0.010;
	sun = earth * 109;
	Fill(0, 0, 255, 1);
	Circle(w / 3, h - (h / 10), earth);
	Fill(255, 255, 224, 1);
	Circle(w, 0, sun);
	End();
}

// advert is an ad for the package 
void advert(int w, int h) {
	VGfloat y = h/4;
	int fontsize = (w * 4) / 100;
	char *s = "github.com/ajstarks/openvg";
	char *a = "ajstarks@gmail.com";
	int imw = 110, imh = 110, rw = w/4, rh = (rw*2/3);
	VGfloat tw = TextWidth(s, SansTypeface, fontsize);

	Start(w, h);
	makepi((w/2) - (rw/2), h/2, rw, rh);
	Fill(128, 0, 0, 1);
	Text(w / 2 - (tw / 2), y - (fontsize / 4), s, SansTypeface, fontsize);
	y -= 100;
	tw = TextWidth(a, SansTypeface, fontsize / 3);
	Fill(128, 128, 128, 1);
	Text(w / 2 - (tw / 2), y, a, SansTypeface, fontsize / 3);
	Image((w / 2) - (imw / 2), 20, imw, imh, "starx.jpg");
	End();
}

// demo shows a timed demonstration
void demo(int w, int h, int sec) {
	refcard(w, h);
	sleep(sec);
//	rshapes(w, h, 50);
//	sleep(sec);
//	testpattern(w, h, "OpenVG on RasPi");
//	sleep(sec);
//	imagetable(w, h);
//	sleep(sec);
//	rotext(w, h, 30, "Raspi");
//	sleep(sec);
//	tb(w, h);
//	sleep(sec);
//	fontrange(w, h);
//	sleep(sec);
//	sunearth(w, h);
//	sleep(sec);
//	raspi(w, h, "The Raspberry Pi");
//	sleep(sec);
	gradient(w,h);
	sleep(sec);
	advert(w, h);
}

// wait for a specific character 
void waituntil(int endchar) {
    int key;

    for (;;) {
        key = getchar();
        if (key == endchar || key == '\n') {
            break;
        }
    }
}
// main initializes the system and shows the picture. 
// Exit and clean up when you hit [RETURN].
int main(int argc, char **argv) {
	int w, h, n;
	char *usage =
	    "%s [command]\n\tdemo sec\n\tastro\n\ttest ...\n\trand n\n\trotate n ...\n\timage\n\ttext\n\tfontsize\n\traspi\n\tadvert\n\tgradient\n";
	char *progname = argv[0];
	SaveTerm();
	InitShapes(&w, &h);
	RawTerm();
	switch (argc) {
	case 2:
		if (strncmp(argv[1], "image", 5) == 0) {
			imagetable(w, h);
		} else if (strncmp(argv[1], "text", 4) == 0) {
			tb(w, h);
		} else if (strncmp(argv[1], "astro", 5) == 0) {
			sunearth(w, h);
		} else if (strncmp(argv[1], "fontsize", 8) == 0) {
			fontrange(w, h);
		} else if (strncmp(argv[1], "advert", 6) == 0) {
			advert(w,h);
		} else if (strncmp(argv[1], "raspi", 5) == 0) {
			raspi(w, h, "The Raspberry Pi");
		} else if (strncmp(argv[1], "gradient", 8) == 0) {
			gradient(w,h);
		} else {
			RestoreTerm();
			fprintf(stderr, usage, progname);
			return 1;
		}
		break;
	case 3:
		n = atoi(argv[2]);
		if (strncmp(argv[1], "demo", 4) == 0) {
			if (n < 1 || n > 30) {
				n = 5;
			}
			demo(w, h, n);
		} else if (strncmp(argv[1], "rand", 4) == 0) {
			if (n < 1 || n > 1000) {
				n = 100;
			}
			rshapes(w, h, n);
		} else if (strncmp(argv[1], "test", 4) == 0) {
			testpattern(w, h, argv[2]);
		} else {
			RestoreTerm();
			fprintf(stderr, usage, progname);
			return 1;
		}
		break;

	case 4:
		if (strncmp(argv[1], "rotate", 6) == 0) {
			rotext(w, h, atoi(argv[2]), argv[3]);
		} else {
			RestoreTerm();
			fprintf(stderr, usage, progname);
			return 1;
		}
		break;

	default:
		refcard(w, h);
	}
	waituntil(0x1b);
	RestoreTerm();
	FinishShapes();
	return 0;
}
