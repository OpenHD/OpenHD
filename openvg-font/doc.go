/*
Package openvg is a wrapper to a C library of high-level 2D graphics operations built on OpenVG 1.1

The typical "hello world" program looks like this: 

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
		openvg.FillRGB(44, 100, 232, 1)                           // Big blue marble
		openvg.Circle(w2, 0, w)                                   // The "world"
		openvg.FillColor("rgb(100,255,123)")                      // White text
		openvg.TextMid(w2, h2, "hello, world", "serif", width/10) // Greetings 
		openvg.End()                                              // End the picture
		bufio.NewReader(os.Stdin).ReadBytes('\n')                 // Pause until [RETURN]
		openvg.Finish()                                           // Graphics cleanup
	}

Functions

The Init function provides the necessary graphics subsystem initialization and the dimensions of the whole canvas.
The Init() call must be paired with a corresponding Finish() call, which performs an orderly shutdown.

Typically a "drawing" begins with the Start() call, and ends with End(). A program can have an arbitrary set
of Start()/End() pairs.

The coordinate system uses float64 coordinates, with the origin at the lower left, with x increasing to the right,
and y increasing upwards.

Currently, the library provides no mouse or keyboard events, other than those provided by the base operating system.
It is typical to pause for user input between drawings by reading standard input.

The library's functionally includes shapes, attributes, transformations, text, images, and convenince functions.
Shape functions include Polygon, Polyline, Cbezier, Qbezier, Rect, Roundrect, Line, Elipse, Circle, and Arc.
Transformation functions are: Translate, Rotate, Shear, and Scale.
For displaying and measuring text: Text, TextMid, TextEnd, and TextWidth.
The attribute functions are StrokeColor, StrokeRGB, StrokeWidth, and FillRGB, FillColor, FillLinearGradient, and FillRadialGradient. 
Colors are specfied with RGB triples (0-255) with alpha values (0.0-1.0), or named colors as specified by the SVG standard.

Convenience functions are used to set the Background color, start the drawing with a background color, and save the raster to a file.
The input terminal may be set/restored to/from raw and cooked mode.

*/
package openvg
