#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"

/*
 * NOTE: Watch out with multiple transforms, they are applied in
 * reverse order! Due to this if you use rotation/scales then the
 * translate part of DrawImageAt() and DrawImageAtFit() will be
 * applied incorrectly (the translate will effectively happen *before*
 * your custom transforms, not after). I can alter it so it happens
 * last but then it will be wrong for anybody using transforms
 * correctly.
 * 
 * In the case of the rotating cursor here we need to
 * move the origin to the hotspot of the cursor (it's tip at 3,3) so
 * when we say draw it at say (400,500) we mean place the tip at that
 * position. Since the origin is normally at the bottom-left, the
 * hotspot is at (3,height-3) from there. Negative values are used for the
 * hotspot location as we need to move the image left & down.
 * So in full we want to translate the origin to the hotspot, rotate
 * the image and then translate the cursor to it's position. Doing
 * this in reverse we end up with :-
 * Translate the cursor's hotspot into position, rotate the image
 * around the hotspot and finally translate the hotspot to the origin.
 * 
 * DrawImageAt() applies the translation effectively *before* any
 * custom transforms so having the cursor position translated in
 * DrawImageAt() won't work. I'm using the raw vgDrawImage() call to
 * avoid the extra set-up (I could pass the hotspot transform to it
 * but that makes it look wrong). I could change the DrawImageAt()
 * function to apply the translation before the user's transforms...
 */

int main() {
	int width, height;
        InitWindowSize(100, 100, 1720, 880);
        
        if (!InitShapes(&width, &height))
                return 1;
        
	Start(width, height);
	Background(0, 0, 0);
        Fill(44, 77, 232, 1);
        
        int32_t desert_w, desert_h, cursor_w, cursor_h;
        VGImage desert = CreateImageFromJpeg("desert1.jpg");
        if (desert == VG_INVALID_HANDLE)
                fputs("Failed to load desert1.jpg image.\n", stderr);
        else {
                desert_w = vgGetParameteri(desert, VG_IMAGE_WIDTH);
                desert_h = vgGetParameteri(desert, VG_IMAGE_HEIGHT);
        }
        
        VGImage cursor = CreateImageFromPng("Select.png");
        if (cursor == VG_INVALID_HANDLE)
                fputs("Failed to load Select.png cursor.\n", stderr);

        cursor_w = vgGetParameteri(cursor, VG_IMAGE_WIDTH);
        cursor_h = vgGetParameteri(cursor, VG_IMAGE_HEIGHT);
        const VGfloat hotspot_x = -3.0f, hotspot_y = -(cursor_h-3.0f);
        if (desert && cursor) {
                VGfloat desertw = (VGfloat)desert_w;
                VGfloat deserth = (VGfloat)desert_h;
                VGfloat min_sw = 2.0f;
                VGfloat max_sw = desertw * 2.0f;
                VGfloat scaled_ratio = deserth / desertw;
                VGfloat scaled_w = min_sw;
                VGfloat scaled_inc = 2.0f;
                VGfloat rot_angle = 0.0f;
                
                // Set-up for the spinning cursor
                // Pre-set the matrix mode to image as nothing
                // else needs any other mode in this
                // example. Translate the point (6,28) which
                // is the tip of the arrow to (0,0) ready for
                // rotates and save the matrix
                vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);

                // Create cursor - get image from VGimage
                CreateCursorFromVGImage(cursor, 3, 3);
                MoveCursor(0, 0);
                ShowCursor();

                int count;
                for (count = 0; count < 256; count++) {
                        MoveCursor(count % width, count % height);
                        WindowClear();
                        Circle(width / 2, 0, width); // Background
                        vgLoadIdentity(); // Reset transform matrix
                                // Draw desert image at top-left
                        DrawImageAt(0.0f, (VGfloat)(height - desert_h), desert);
                                // Scaled desert in middle of screen
                        VGfloat scaled_h = scaled_w * scaled_ratio;
                        VGfloat xpos = ((VGfloat)width - scaled_w) / 2.0f;
                        VGfloat ypos = ((VGfloat)height - scaled_h) / 2.0f;
                        DrawImageAtFit(xpos, ypos, scaled_w, scaled_h, desert);
                        scaled_w += scaled_inc;
                        if (scaled_w < min_sw || scaled_w > max_sw) {
                                scaled_w -= scaled_inc;
                                scaled_inc = -scaled_inc;
                        }
                                // Draw the cursor in middle of screen
                                // spinning around it's hotspot.
                        vgTranslate((VGfloat)width / 2.0f,
                                    (VGfloat)height / 2.0f);
                        vgRotate(rot_angle);
                        vgTranslate(hotspot_x, hotspot_y);
                        vgDrawImage(cursor);
                        if ((rot_angle += 5.0f) > 360.0f)
                                rot_angle -= 360.0f;
                        if (!End()) {
                                fprintf(stderr, "Error @ count = %d\n", count);
                                break;
                        }
			ScreenBrightness(count * 4) % 255);
                }
                DeleteCursor();
        }
        // It's safe to destroy objects that don't exist -
        // OpenVG will give us an error but we already know
        // that and are exiting anyway.
        vgDestroyImage(cursor);
        vgDestroyImage(desert);
	FinishShapes();
	return 0;
}
