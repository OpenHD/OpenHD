//
// clip: test rectangular clipping
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "shapes.h"

int main() {
	int  w, h, cx, cy, cw, ch, midy, fontsize;
	char *message = "Now is the time for all good men to come to the aid of the party";
	char s[3];

	InitShapes(&w, &h);
	float speed, x, rx, ry, rw, rh;
	speed = 15.0;
	x  = 0;
	midy = (h / 2);
	fontsize = w / 50;
	cx = 0;
	ch = fontsize * 2;
	cw = w;
	cy = midy - (ch / 2);

	rx = (float)cx;
	ry = (float)cy;
	rw = (float)cw;
	rh = (float)ch;

	// scroll the text, only in the clipping rectangle
	for (x = 0.0; x < rw+speed; x += speed) {
		Start(w, h);
		Background(255, 255, 255);
		Fill(0, 0, 0, .2);
		Rect(rx, ry, rw, rh);
		ClipRect(cx, cy, cw, ch);
		Translate(x, ry+(fontsize/2));
		Fill(0, 0, 0, 1);
		Text(0, 0, message, SansTypeface, fontsize);
		ClipEnd();
		End();
	}
	fgets(s, 2, stdin);
	FinishShapes();
	exit(0);
}
