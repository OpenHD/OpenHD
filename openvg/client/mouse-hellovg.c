// first OpenVG program, with mouse 
// Anthony Starks (ajstarks@gmail.com)

// revision history:  LW = Leycester Whewell (leycester@btconnect.com)
// when         who             what
// 04Mar13      LW                      mouse movement added, small red cross-hair cursor displayed
// 2013-03-06   ajstarks        cleanup, translucent circle cursor sized by CUR_SIZ

// to do list:
// show cursor before first mouse movement
// init cursor in centre of screen, not at 400,300 -- done
// cater for different screen sizes, not just 1280x1024 -- done
// cater for cursor images, eg arrows. -- done

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"

#include <linux/input.h>
#include <fcntl.h>
#include <pthread.h>

// Mouse state structure
typedef struct {
	int fd;
	struct input_event ev;
	VGfloat x, y;
	int left, middle, right;
	int max_x, max_y;
} mouse_t;

mouse_t mouse;			// global mouse state
int left_count = 0;
int quitState = 0;
#define    CUR_SIZ  16					   // cursor size, pixels beyond centre dot

// evenThread reads from the mouse input file
void *eventThread(void *arg) {

	// Open mouse driver
	if ((mouse.fd = open("/dev/input/event0", O_RDONLY)) < 0) {
		fprintf(stderr, "Error opening Mouse!\n");
		quitState = 1;
		return &quitState;
	}
	mouse.x = mouse.max_x / 2;			   //Reset mouse
	mouse.y = mouse.max_y / 2;

	while (1) {
		read(mouse.fd, &mouse.ev, sizeof(struct input_event));
		// printf("[%4.0f,%4.0f]\r",mouse.x,mouse.y);

		// Check events
		mouse.left = CUR_SIZ * 2;		   // Reset Mouse button states
		mouse.right = CUR_SIZ * 2;

		if (mouse.ev.type == EV_REL) {
			if (mouse.ev.code == REL_X) {
				mouse.x += (VGfloat) mouse.ev.value;
				if (mouse.x < 0) {
					mouse.x = 0;
				}
				if (mouse.x > mouse.max_x) {
					mouse.x = mouse.max_x;
				}
			}
			if (mouse.ev.code == REL_Y) {	   //This ones goes backwards hence the minus
				mouse.y -= (VGfloat) mouse.ev.value;
				if (mouse.y < 0) {
					mouse.y = 0;
				}
				if (mouse.y > mouse.max_y) {
					mouse.y = mouse.max_y;
				}
			}
		}

		if (mouse.ev.type == EV_KEY) {
			//printf("Time Stamp:%d - type %d, code %d, value %d\n",
			//      mouse.ev.time.tv_usec,mouse.ev.type,mouse.ev.code,mouse.ev.value);
			if (mouse.ev.code == BTN_LEFT) {
				mouse.left = 1;
				//   printf("Left button\n");
				left_count++;
				// printf("User Quit\n");
				// quitState = 1;
				// return &quitState;  //Left mouse to quit
			}
			if (mouse.ev.code == BTN_RIGHT) {
				mouse.right = 1;
				//  printf("Right button\n");
			}
		}
	}
}

static int cur_sx, cur_sy, cur_w, cur_h;	// cursor location and dimensions
static int cur_saved = 0;	// amount of data saved in cursor image backup

// saveCursor saves the pixels under the mouse cursor
void saveCursor(VGImage CursorBuffer, int curx, int cury, int screen_width, int screen_height, int s) {
	int sx, sy, ex, ey;

	sx = curx - s;					   // horizontal 
	if (sx < 0) {
		sx = 0;
	}
	ex = curx + s;
	if (ex > screen_width) {
		ex = screen_width;
	}
	cur_sx = sx;
	cur_w = ex - sx;

	sy = cury - s;					   // vertical 
	if (sy < 0) {
		sy = 0;
	}
	ey = cury + s;
	if (ey > screen_height) {
		ey = screen_height;
	}
	cur_sy = sy;
	cur_h = ey - sy;

	vgGetPixels(CursorBuffer, 0, 0, cur_sx, cur_sy, cur_w, cur_h);
	cur_saved = cur_w * cur_h;
}

// restoreCursor restores the pixels under the mouse cursor
void restoreCursor(VGImage CursorBuffer) {
	if (cur_saved != 0) {
		vgSetPixels(cur_sx, cur_sy, CursorBuffer, 0, 0, cur_w, cur_h);
	}
}

// circleCursor draws a translucent circle as the mouse cursor
void circleCursor(int curx, int cury, int width, int height, int s) {
	Fill(100, 0, 0, 0.50);
	Circle(curx, cury, s);
	Fill(0, 0, 0, 1);
	Circle(curx, cury, 2);
}

// mouseinit starts the mouse event thread
int mouseinit(int w, int h) {
	pthread_t inputThread;
	mouse.max_x = w;
	mouse.max_y = h;
	return pthread_create(&inputThread, NULL, &eventThread, NULL);
}

int main() {
	int width, height, cursorx, cursory, cbsize;

	InitShapes(&width, &height);				   // Graphics initialization
	cursorx = width / 2;
	cursory = height / 2;
	cbsize = (CUR_SIZ * 2) + 1;
	VGImage CursorBuffer = vgCreateImage(VG_sABGR_8888, cbsize, cbsize, VG_IMAGE_QUALITY_BETTER);

	if (mouseinit(width, height) != 0) {
		fprintf(stderr, "Unable to initialize the mouse\n");
		exit(1);
	}
	Start(width, height);				   // Start the picture
	Background(0, 0, 0);				   // Black background
	Fill(44, 77, 232, 1);				   // Big blue marble
	Circle(width / 2, 0, width);			   // The "world"
	Fill(255, 255, 255, 1);				   // White text
	TextMid(width / 2, height / 2, "hello, world", SansTypeface, width / 10);	// Greetings 
	End();						   // update picture

	// MAIN LOOP
	while (left_count < 2) {			   // Loop until the left mouse button pressed & released
		// if the mouse moved...
		if (mouse.x != cursorx || mouse.y != cursory) {
			restoreCursor(CursorBuffer);
			cursorx = mouse.x;
			cursory = mouse.y;
			saveCursor(CursorBuffer, cursorx, cursory, width, height, CUR_SIZ);
			circleCursor(cursorx, cursory, width, height, CUR_SIZ);
			End();				   // update picture
		}
	}
	restoreCursor(CursorBuffer);			   // not strictly necessary as display will be closed
	vgDestroyImage(CursorBuffer);			   // tidy up memory
	FinishShapes();					   // Graphics cleanup
	exit(0);
}
