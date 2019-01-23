/*
 *   This file is part of EZ-Wifibroadcast: https://github.com/bortek/EZ-WifiBroadcast
 *
 *   Copyright 2018 Jelle Tigchelaar
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"

int main(int argc, char *argv[]) {
    int width, height;
		
    char filepath[] = {"/home/pi/wifibroadcast-splash/splash.jpg"};	// The path to the splash screen

    InitShapes(&width, &height);            						// Graphics initialization

    Start(width, height);
	
	VGImage splash = CreateImageFromJpeg(filepath);					// Load the image

	DrawImageAtFit(0, 0, width, height, splash);    				// Draw stretched across entire screen
    
    End();
    usleep(7000000);

    FinishShapes();                 								// Graphics cleanup
    exit(0);
}
