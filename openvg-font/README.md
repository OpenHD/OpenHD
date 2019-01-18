# Testbed for exploring OpenVG on the Raspberry Pi.

<a href="http://www.flickr.com/photos/ajstarks/7811750326/" title="rotext by ajstarks, on Flickr"><img src="http://farm8.staticflickr.com/7249/7811750326_614ea891ae.jpg" width="500" height="281" alt="rotext"></a>

Experimental extension of ajstarks' library by Paeryn.
  Added direct font loading (no longer need to use font2openvg to convert).
  Changed some of the basic shape drawing routines to avoid constant memory allocation/deallocations.

## First program

Here is the graphics equivalent of "hello, world"

	// first OpenVG program
	// Anthony Starks (ajstarks@gmail.com)
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include "VG/openvg.h"
	#include "VG/vgu.h"
	#include "fontinfo.h"
	#include "shapes.h"
	
	int main() {
		int width, height;
		char s[3];
	
		init(&width, &height);					// Graphics initialization
	
		Start(width, height);					// Start the picture
		Background(0, 0, 0);					// Black background
		Fill(44, 77, 232, 1);					// Big blue marble
		Circle(width / 2, 0, width);			// The "world"
		Fill(255, 255, 255, 1);					// White text
		TextMid(width / 2, height / 2, "hello, world", SerifTypeface, width / 10);	// Greetings 
		End();						   			// End the picture
	
		fgets(s, 2, stdin);				   		// look at the pic, end with [RETURN]
		finish();					            // Graphics cleanup
		exit(0);
	}

<a href="http://www.flickr.com/photos/ajstarks/7828969180/" title="hellovg by ajstarks, on Flickr"><img src="http://farm9.staticflickr.com/8436/7828969180_b73db3bf19.jpg" width="500" height="281" alt="hellovg"></a>

Due to the new string handling in Text() the string format is defined by the locale system.
By default in C this is basic ASCII (chars 0 -> 127), if your string contains characters outside this
range then you need to #include &lt;locale.h&gt; and set the locale to your system default with

	setlocale(LC_CTYPE, "");


## API

<a href="http://www.flickr.com/photos/ajstarks/7717370238/" title="OpenVG refcard by ajstarks, on Flickr"><img src="http://farm8.staticflickr.com/7256/7717370238_1d632cb179.jpg" width="500" height="281" alt="OpenVG refcard"></a>

Coordinates are VGfloat values, with the origin at the lower left, with x increasing to the right, and y increasing up.
OpenVG specifies colors as a VGfloat array containing red, green, blue, alpha values ranging from 0.0 to 1.0, but typically colors are specified as RGBA (0-255 for RGB, A from 0.0 to 1.0)

### Window (canvas) functions

	void WindowClear() 
WindowClear clears the window to previously set background colour

	void AreaClear(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
AreaClear clears a given rectangle in window coordinates

	void WindowOpacity(unsigned int a)
WindowOpacity sets the  window opacity

	void WindowPosition(int x, int y)
WindowPosition moves the window to given position

### Setup and shutdown

	void InitShapes(int *w, int *h)
Initialize the graphics: width and height of the canvas are returned.  This should begin every program.

	void InitWindowSize(int x, int y, unsigned int w, unsigned int h)
Initialize with specific dimensions

	void FinishShapes() 
Shutdown the graphics. This should end every program.

	void EnableOpenVGErrorChecks(bool check)
Enables or disables checking of OpenVG errors when the error status is unknown.

	uint32_t CheckErrorStatus()
Returns the last known OpenVG error status (and checks current state if enabled).

	void Start(int width, int height)
Begin the picture, clear the screen with a default white, set the stroke and fill to black.

	void End()
End the picture, rendering to the screen.

	void SaveEnd(char *filename)
End the picture, rendering to the screen, save the raster to the named file as 4-byte RGBA words, with a stride of
width*4 bytes. The program raw2png converts the "raw" raster to png.

	bool WindowSaveAsPNG(const char *filename, VGint x, VGint y, VGint w, VGint h, int zlib_level)
Saves an area of the screen the sized of (w,h) from (x,y) to the named file as a png. zlib_level is the compression from 0=none to 9=best. Returns false if there was an error.

        VGImage LoadImageFromPNG(const char *filename, VGint *w, VGint *h)
Loads a png into a VGImage, w and h are pointers where the width and height of the image will be stored.

        DrawImageAt(VGfloat x, VGfloat y, VGImage image)
Draws the image at the specified location.

        DrawImageAtFit(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGImage image)
Draws the image scaled to fit the given size at the given location.

        void CopyMatrixPathToImage()
Copies the Path transformation matrix to the Image transformation matrix.

	void SaveTerm(), RestoreTerm(), RawTerm()
Terminal settings, save current settings, restore settings, put the terminal in raw mode.

### Hardware cursor support
A hardware cursor image has been implemented. It will appear alpha-blended over the screen but is not part of the screen so won't appear in screen-grabs or affect what is drawn under it.
The cursor, when created will not be initially visible. This is so you can position it first.
There can be only one cursor at a time. If you want to change the image then you need to DeleteCursor() first and create a new one.

        bool CreateCursor(uint32_t *data, uint32_t width, uint32_t height, uint32_t hot_x, uint32_t hot_y)
Creates a hardware cursor from raw RGBA data. hot_x,hot_y specifies the location of the point in the image that is used for the cursor's location.
Returns false if a cursor already exists or there is an error.
*NOTE* The cursor's origin is top-left going down rather than OpenVG's bottom-left going up.

        bool CreateCursorFromVGImage(VGImage image, uint32_t hot_x, uint32_t hot_y)
Creates a cursor as CreateCursor() but takes the image from a VGImage. The difference in origin is taken into account.

	void ShowCursor()
Makes the cursor visible.

        void HideCursor()
Makes the cursor invisible.

        void MoveCursor(int32_t x, int32_t y)
Places the cursor at the specified coordinte.
*NOTE* the coordinte is specified in pixels with (0,0) being top-left of the window.

        void DeleteCursor()
Deletes the cursor.

### Attributes

	void SetFill(float color[4])
Set the fill color

	void Background(unsigned int r, unsigned int g, unsigned int b)
Fill the screen with the background color defined from RGB values.

	void BackgroundRGBA(unsigned int r, unsigned int g, unsigned int b, VGfloat a)
clears the screen to a background color with alpha

	void StrokeWidth(float width)
Set the stroke width.

	void RGBA(unsigned int r, unsigned int g, unsigned int b, VGfloat a, VGfloat color[4])
fill a color vector from RGBA values.

	void RGB(unsigned int r, unsigned int g, unsigned int b, VGfloat color[4])
fill a color vector from RGB values.

	void Stroke(unsigned int r, unsigned int g, unsigned int b, VGfloat a)
Set the Stroke color using RGBA values.

	void Fill(unsigned int r, unsigned int g, unsigned int b, VGfloat a)
Set the Fill color using RGBA values.

	void FillLinearGradient(VGfloat x1, VGfloat y1, VGfloat x2, VGfloat y2, VGfloat *stops, int n)
Set the fill to a linear gradient bounded by (x1, y1) and (x2, y2). using offsets and colors specified in n number of stops

	void FillRadialGradient(VGfloat cx, VGfloat cy, VGfloat fx VGfloat fy, VGfloat r, VGfloat *stops, int n)
Set the fill to a radial gradient centered at (cx, cy) with radius r, and focal point at (fx, ry), using offsets and colors specified in n number of stops

### Shapes

	void Line(VGfloat x1, VGfloat y1, VGfloat x2, VGfloat y2)
Draw a line between (x1, y1) and (x2, y2).

	void Rect(VGfloat x, VGfloat y, VGfloat w, VGfloat h)
Draw a rectangle with its origin (lower left) at (x,y), and size is (width,height).

	void RectOutline(VGfloat x, VGfloat y, VGfloat w, VGfloat h)
Outlined version

	void Roundrect(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat rw, VGfloat rh)
Draw a rounded rectangle with its origin (lower left) at (x,y), and size is (width,height).  
The width and height of the corners are specified with (rw,rh).

	void RoundrectOutline(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat rw, VGfloat rh)
Outlined version

	void Polygon(VGfloat *x, VGfloat *y, VGint n)
Draw a polygon using the coordinates in arrays pointed to by x and y.  The number of coordinates is n.

	void Polyline(VGfloat *x, VGfloat *y, VGint n)
Draw a polyline using the coordinates in arrays pointed to by x and y.  The number of coordinates is n.

	void Circle(VGfloat x, VGfloat y, VGfloat r)
Draw a circle centered at (x,y) with radius r.

	void CircleOutline(VGfloat x, VGfloat y, VGfloat r)
Outlined version

	void Ellipse(VGfloat x, VGfloat y, VGfloat w, VGfloat h)
Draw an ellipse centered at (x,y) with radii (w, h).

	void EllipseOutline(VGfloat x, VGfloat y, VGfloat w, VGfloat h)
Outlined version

	void Qbezier(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat ex, VGfloat ey)
Draw a quadratic bezier curve beginning at (sx, sy), using control points at (cx, cy), ending at (ex, ey).

	void QbezierOutline(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat ex, VGfloat ey)
Outlined version

	void Cbezier(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat px, VGfloat py, VGfloat ex, VGfloat ey)
Draw a cubic bezier curve beginning at (sx, sy), using control points at (cx, cy) and (px, py), ending at (ex, ey).

	void CbezierOutline(VGfloat sx, VGfloat sy, VGfloat cx, VGfloat cy, VGfloat px, VGfloat py, VGfloat ex, VGfloat ey) 
Outlined version

	void Arc(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat sa, VGfloat aext)
Draw an elliptical arc centered at (x, y), with width and height at (w, h).  Start angle (degrees) is sa, angle extent is aext.

	void ArcOutline(VGfloat x, VGfloat y, VGfloat w, VGfloat h, VGfloat sa, VGfloat aext)
Outlined version

	void Dot(VGfloat x, VGfloat y, bool smooth)
Draw a 1 unit dot with it's lower-left extent at (x,y). Only fill is used, no stroke.
If smooth == true then a circle is drawn with it's centre at (x+0.5,y+0.5),
if smooth == false then a rectangle is drawn from (x,y).

### Text and Images

	void Text(VGfloat x, VGfloat y, char* s, Fontinfo f, int pointsize)
Draw a the text srtring (s) at location (x,y), using pointsize.

	void TextMid(VGfloat x, VGfloat y, char* s, Fontinfo f, int pointsize)
Draw a the text srtring (s) at centered at location (x,y), using pointsize.

	void TextEnd(VGfloat x, VGfloat y, char* s, Fontinfo f, int pointsize)
Draw a the text srtring (s) at with its lend aligned to location (x,y), using pointsize

	VGfloat TextWidth(char *s, Fontinfo f, int pointsize)
Return the width of text

	VGfloat TextHeight(Fontinfo f, int pointsize)
Return a font's height

	TextDepth(Fontinfo f, int pointsize)
Return a font's distance beyond the baseline.

	TextLineHeight(Fontinfo f, int pointsize)
Return the font's recommended distance between lines.

	void Image(VGfloat x, VGfloat y, int w, int h, char * filename)
place a JPEG image with dimensions (w,h) at (x,y).

	
### Transformations

	void Translate(VGfloat x, VGfloat y)
Translate the coordinate system to (x,y).

	void Rotate(VGfloat r)
Rotate the coordinate system around angle r (degrees).

	void Scale(VGfloat x, VGfloat y)
Scale by x,y.

	void Shear(VGfloat x, VGfloat y)
Shear by the angles x,y.

## Clipping
	void ClipRect(VGint x, VGint y, VGint w, VGint h)
Limit drawing the drawing area to the specified rectangle, end with ClipEnd()

	void ClipEnd()
Ends clipping area

## Using fonts

### Supplimental: New font system
The old embedded font method is still supported, but new functions allow loading fonts directly.
The new system will try to load every glyph in the font. Some fonts may not load if they contain too many glyphs.


Fontinfo is now a pointer type rather than a structure.

	Fontinfo LoadTTFFile(const char *filename)
Loads a font using it's filename. The font can be either TrueType (.ttf) or Postscript Type 1 (.pfa, .pfb)
Other scalable formats that FreeType2 supports should also work.

	Fontinfo LoadTTF(const char *fontname)
Loads a font using the font's name and style. This uses fontconfig to search the installed fonts
for the nearest matching font and loads that (so if the requested font isn't found a similar one is used).
The name is the family name of the font e.g, DejaVuSans, Roboto, Courier and can also include
a style like Italic or Bold. If a style is wanted it is put directly after the family name and a colon
e.g. Roboto:Italic

	void unloadfont(Fontinfo font)
Unloads the font. This replaces the old unload function. It now takes just one parameter,
the Fontinfo that was returned from LoadTTF() or LoadTTFFile().

The default fonts (SansTypeface, SerifTypeface and MonoTypeface) can now be unloaded if your program
doesn't need them and requires more gpu memory. If you do this you MUST set the corresponding variable
to NULL otherwise the library will try to unload them itself when you call finish().

	FontKerning(Fontinfo font, int active)
Turns on or off font kerning for the font.
If active is 0 then it is turned off,
if active is any other value then it is turned on if the font supports it.

A font's kerning status can be tested by looking at font->Kerning
If it is 0 then kerning is off. This will be the case if you try turning it on when the font doesn't support it.
Any other value means kerning is on.

A font's family name can be queried by reading font->Name
A font's style can be queried by reading font->Style
**Do not edit these strings!**

###Original font loading

Also included is the font2openvg program, which turns font information into C source that 
you can embed in your program. The Makefile makes font code from files found in /usr/share/fonts/truetype/ttf-dejavu/. 
If you want to use other fonts, adjust the Makefile accordingly, or generate the font code on your own once the font2openvg program is built.

font2openvg takes three arguments: the TrueType font file, the output file to be included and the prefix for identifiers.
For example to use the DejaVu Sans font:

	./font2openvg /usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf DejaVuSans.inc DejaVuSans

and include the generated code in your program:

	#include "DejaVuSans.inc"
	Fontinfo DejaFont
	
The loadfont function creates OpenVG paths from the font data:
Note that there are two extra parameters now, these provide the descender and ascender
sizes of the font (how far below and above the baseline that the font goes).

	loadfont(DejaVuSans_glyphPoints, 
            DejaVuSans_glyphPointIndices, 
        	DejaVuSans_glyphInstructions,                
        	DejaVuSans_glyphInstructionIndices, 
            DejaVuSans_glyphInstructionCounts, 
            DejaVuSans_glyphAdvances,
            DejaVuSans_characterMap, 
        	DejaVuSans_glyphCount,
        	int DejaVuSans_descender_height,
        	int DejaVuSans_ascender_height);

The unloadfont function releases the path information:
Note that the Glyphs parameter is now the Fontinfo, and the count parameter has been removed.
	unloadfont(DejaVuSansFont);

Fonts loaded this way do not support kerning, or have their name & style set.
Also the TextLineHeight is set to be TextHeight + TextDepth (i.e. the approximate size of a character).


Note that the location of the font files may differ.  (The current location for Jessie is /usr/share/fonts/truetype/ttf-dejavu)
Use the FONTLIB makefile variable to adjust this location.

# Build and run

<i>Note that you will need at least 64 Mbytes of GPU RAM:</i>. You will also need the DejaVu fonts, and the jpeg and freetype and fontconfig libraries.
The indent tool is also useful for code formatting.  Install them via:

	pi@raspberrypi ~ $ sudo apt-get install libjpeg8-dev indent libfreetype6-dev ttf-dejavu-core libfontconfig1-dev

Next, build the library and test:

	pi@raspberrypi ~ $ git clone git://github.com/paeryn/openvg
	pi@raspberrypi ~ $ cd openvg
	pi@raspberrypi ~ $ git checkout windowsave
	pi@raspberrypi ~/openvg $ make
	g++ -I/usr/include/freetype2 fontutil/font2openvg.cpp -o font2openvg -lfreetype
	./font2openvg /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf DejaVuSans.inc DejaVuSans
	468 glyphs written
	./font2openvg /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf DejaVuSansMono.inc DejaVuSansMono
	468 glyphs written
	./font2openvg /usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf DejaVuSerif.inc DejaVuSerif
	468 glyphs written
	gcc -std=gnu89 -O2 -Wall -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -fPIC -c oglinit.c
	gcc -std=gnu89 -O2 -Wall -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -fPIC -c libshapes.c
	gcc -std=gnu89 -O2 -Wall -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -fPIC -I/usr/include/freetype2 -c fontsystem.c
	gcc -L/opt/vc/lib -lEGL -lGLESv2 -ljpeg -shared -o libshapes.so -Wl,-soname,libshapes.so.2.0.0 oglinit.o libshapes.o fontsystem.o
	pi@raspberrypi ~/openvg/client $ cd client
	pi@raspberrypi ~/openvg/client $ make test
	gcc -O2 -Wall -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -I.. -o shapedemo shapedemo.c ../libshapes.o ../oglinit.o ../fontsystem.o -Wl,-rpath=/opt/vc/lib -L/opt/vc/lib -lEGL -lGLESv2 -lbcm_host -pthread -ljpeg -lfreetype -lfontconfig
	./shapedemo demo 5


The program "shapedemo" exercises a high-level API built on OpenVG found in libshapes.c. 

	./shapedemo                      # show a reference card
	./shapedemo raspi                # show a self-portrait
	./shapedemo image                # show four test images
	./shapedemo astro                # the sun and the earth, to scale
	./shapedemo text                 # show blocks of text in serif, sans, and mono fonts
	./shapedemo rand 10              # show 10 random shapes
	./shapedemo rotate 10 a          # rotated and faded "a"
	./shapedemo test "hello, world"  # show a test pattern, with "hello, world" at mid-display in sans, serif, and mono.
	./shapedemo fontsize             # show a range of font sizes (per <https://speakerdeck.com/u/idangazit/p/better-products-through-typography>)
	./shapedemo demo 10              # run through the demo, pausing 10 seconds between each one; contemplate the awesome.
	

To install the shapes library as a system-wide shared library
	
	pi@raspberrypi ~/openvg $ make library
	pi@raspberrypi ~/openvg $ sudo make install

The openvg shapes library can now be used in C code by including shapes.h and fontinfo.h and linking with libshapes.so:

	#include <shapes.h>
	#include <fontinfo.h>

	pi@raspberrypi ~ $ gcc -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads anysource.c -o anysource -lshapes
	pi@raspberrypi ~ $ ./anysource

<a href="http://www.flickr.com/photos/ajstarks/7883988028/" title="The Raspberry Pi, drawn by the Raspberry Pi by ajstarks, on Flickr"><img src="http://farm9.staticflickr.com/8442/7883988028_21fd6533e0.jpg" width="500" height="281" alt="The Raspberry Pi, drawn by the Raspberry Pi"></a>

## Go wrapper

###The Go section hasn't been updated to work with the new system yet, consider it broken in this fork for now.

A Go programming language wrapper for the library is found in openvg.go. Sample clients are in the directory go-client.  The API closely follows the C API; here is the "hello, world" program in Go:

<a href="https://godoc.org/github.com/ajstarks/openvg">The Go API</a>

	package main

	import (
		"bufio"
		"github.com/ajstarks/openvg"
		"os"
	)

	func main() {
		width, height := openvg.Init() // OpenGL, etc initialization

		w2 := openvg.VGfloat(width / 2)
		h2 := openvg.VGfloat(height / 2)
		w := openvg.VGfloat(width)

		openvg.Start(width, height)                               // Start the picture
		openvg.BackgroundColor("black")                           // Black background
		openvg.FillRGB(44, 77, 232, 1)                            // Big blue marble
		openvg.Circle(w2, 0, w)                                   // The "world"
		openvg.FillColor("white")                                 // White text
		openvg.TextMid(w2, h2, "hello, world", "serif", width/10) // Greetings 
		openvg.End()                                              // End the picture
		bufio.NewReader(os.Stdin).ReadBytes('\n')                 // Pause until [RETURN]
		openvg.Finish()                                           // Graphics cleanup
	}

	
To build the wrapper: (make sure GOPATH is set)

	pi@raspberrypi ~/openvg $ go install .
	pi@raspberrypi ~/openvg $ cd go-client/hellovg
	pi@raspberrypi ~/openvg/go-client/hellovg $ go build .
	pi@raspberrypi ~/openvg/go-client/hellovg $ ./hellovg 

