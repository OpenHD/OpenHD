// first OpenVG program, with gradients
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

	stops := []openvg.Offcolor{
		{0.0, openvg.RGB{44, 100, 232}, 1.0}, // blue-ish
		{0.5, openvg.RGB{22, 50, 151}, 1.0},  // darker blue
		{1.0, openvg.RGB{88, 200, 255}, 1.0}, // lighter blue
	}

	openvg.Start(width, height)                               // Start the picture
	openvg.BackgroundColor("black")                           // Black background
	openvg.FillRadialGradient(w2, 0, w2, w2, w*.5, stops)     // Big blue marble
	openvg.Circle(w2, 0, w)                                   // The "world"
	openvg.FillColor("white")                                 // White text
	openvg.TextMid(w2, h2, "hello, world", "serif", width/10) // Greetings
	openvg.SaveEnd("hvg.raw")                                 // End the picture
	bufio.NewReader(os.Stdin).ReadBytes('\n')                 // Pause until [RETURN]
	openvg.Finish()                                           // Graphics cleanup
}
