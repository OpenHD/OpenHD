// ptest.c - Simple particles example using the OpenVG Testbed
// via Nick Williams (github.com/nilliams)
// https://gist.githubusercontent.com/nilliams/7705819/raw/9cbb5a1298d6eef858639095148ede2c33cb6d40/particles.c
//
// Modified by paeryn (paeryn8@gmail.com) to create particles as path
// objects.
//
// Usage: ./particles [OPTIONS]
//
// Options:
//  -t    show trails
//  -n #  draw # number of particles per frame
//  -g #  set gravity to #
//  -r    right-to-left only  (direction alternates by default)
//  -l    left-to-right only

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "shapes.h"

// ~55 seems to be the limit before jankiness kicks in
// New path system is quicker so we can have more!
// Not used now, it can be set on the cmdline with -n
#define NUM_PARTICLES 200

typedef struct particle {
	int x, y;
	double vx, vy;
	int r, g, b;
	int radius;
        VGPath path;
        VGPaint colour;
} particle_t;

unsigned int num_particles = 50;
particle_t *particles;
VGPaint textColour, bgFade;
VGPath bgFadeRect;

int showTrails = 0;
int directionRTL = 0;
int alternate = 1;
double gravity = 0.5;

Fontinfo myFont;
char demoText[64];

// Initialize _all_ the particles
void initParticles(int w, int h) {
	int i;
	for (i = 0; i < num_particles; i++) {
		particle_t *p = &particles[i];

		p->x = 0;
		p->y = 0;
		p->vx = (rand() % 30) + 30;
		p->vy = (rand() % 20) + 40;
		p->r = rand() % 256;
		p->g = rand() % 256;
		p->b = rand() % 256;
		p->radius = (rand() % 20) + 20;
                p->colour = Paint(p->r, p->g, p->b, 1.0);
                p->path = CirclePath(0, 0, p->radius);
                if (p->path == VG_INVALID_HANDLE) {
                        printf("Ran out of gpu memory at %d particles\n", i);
                        exit(1);
                }

                if (directionRTL) {
			p->vx *= -1;
			p->x = w;
		}
	}
}

// Free _all_ the particles
void deinitParticles() {
	int i;
	for (i = 0; i < num_particles; i++) {
		particle_t *p = &particles[i];
                DeletePaint(p->colour);
                DeletePath(p->path);
        }
}

void paintBG(int w, int h) {
 	if (!showTrails)
		return WindowClear();

        FillPaint(bgFade);
	DrawPath(bgFadeRect);
}

void draw(int w, int h) {
	int i;
	particle_t *p;

	paintBG(w, h);

        FillPaint(textColour);
        TextMid(w/2, 10, demoText, myFont, 36);

	for (i = 0; i < num_particles; i++) {
		p = &particles[i];

                FillPaint(p->colour);
                DrawPathAt(p->x, p->y, p->path);
                
		// Apply the velocity
		p->x += p->vx;
		p->y += p->vy;

		p->vx *= 0.98;
		if (p->vy > 0)
			p->vy *= 0.97;

		// Gravity
		p->vy -= gravity;

		// Stop particles leaving the canvas  
		if (p->x < -50)
			p->x = w + 50;
		if (p->x > w + 50)
			p->x = -50;

		// When particle reaches the bottom of screen reset velocity & start posn
		if (p->y < -50) {
			p->x = 0;
			p->y = 0;
			p->vx = (rand() % 30) + 30;
			p->vy = (rand() % 20) + 40;

			if (directionRTL) {
				p->vx *= -1;
				p->x = w;
			}
		}

		if (p->y > h + 50)
			p->y = -50;
	}

	End();
}

void setOptions(int argc, char **argv) {
	int i = argc;
	while (i--) {
		char option = argv[i][1];

		if (option == 't') {
			showTrails = 1;
			printf("Displaying trails\n");
		}
		// If r or l is set, disable alternation
		if (option == 'r' || option == 'l')
			alternate = 0;

		// Set direction
		if (option == 'r') {
			directionRTL = 1;
			printf("Displaying in right-to-left mode\n");
		}

		if (option == 'l') {
			directionRTL = 0;
			printf("Displaying in left-to-right mode\n");
		}
		if (option == 'g' && i+1 < argc) {
			gravity = atof(argv[i+1]);
			printf("Gravity set to %.2f\n", gravity);
		}
                if (option == 'n' && i+1 < argc) {
                        num_particles = atoi(argv[i+1]);
                        if (num_particles < 1) num_particles = 1;
                        if (num_particles > 10000) num_particles = 10000;
                        printf("Particle count set to %d\n", num_particles);
                }
                
	}
}

// Display Options
// -t  show trails
// -g[value] gravity
//
// Direction (alternates by default)
// -r  right-to-left
// -l  left-to-right
int main(int argc, char **argv) {
	srand(time(NULL));

	setOptions(argc, argv);
        particles = malloc(num_particles * sizeof *particles);
        
	int w, h;
	InitShapes(&w, &h);
	initParticles(w, h);

        snprintf(demoText, 63, "Particle demo with %d particles", num_particles);
        myFont = LoadTTF("URW Chancery L");
        textColour = Paint(255, 255, 255, 1.0f);
        bgFade = Paint(0, 0, 0, 0.3f);
        bgFadeRect = RectPath(0, 0, w, h);
        
	Start(w, h);
        Background(0, 0, 0);
        
	int i = 0;
        int frames = 100000;
        while (frames--) {
		draw(w, h);

		// NOTE: Consider a `usleep()` in here to not tie up the CPU if you intend serious use

		// Change launch direction every 100 draws
		i++;
		if (alternate && i == 100) {
			directionRTL = directionRTL ? 0 : 1;
			i = 0;
		}
	}
        deinitParticles();
        UnloadFont(myFont);
        DeletePaint(textColour);
        DeletePaint(bgFade);
        DeletePath(bgFadeRect);
        FinishShapes();
        return 0;
}
