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
        
    /* 
     * The path to the splash screen
     */
    char filepath[] = { "/home/pi/wifibroadcast-splash/splash.jpg" };

    /*
     * Graphics initialization
     */
    InitShapes(&width, &height);
    Start(width, height);
    
    /*
     * Load the image
     */
    VGImage splash = CreateImageFromJpeg(filepath);

    /*
     * Draw stretched across entire screen
     */
    DrawImageAtFit(0, 0, width, height, splash);
    
    End();
    usleep(7000000);

    /*
     * Graphics cleanup
     */
    FinishShapes();


    
    exit(0);
}
