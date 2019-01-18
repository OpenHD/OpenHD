// planets: an exploration of scale
package main

import (
	"bufio"
	"github.com/ajstarks/openvg"
	"os"
)

// Body describes a body within the solar system:
// name, distance from the sun, size and color
type Body struct {
	name     string
	distance openvg.VGfloat
	radius   openvg.VGfloat
	color    openvg.RGB
}

var (
	sun     = Body{"Sun", 0, 695500, openvg.RGB{247, 115, 12}}
	mercury = Body{"Mercury", 0.34, 2439.7, openvg.RGB{250, 248, 242}}
	venus   = Body{"Venus", 0.72, 6051.8, openvg.RGB{255, 255, 242}}
	earth   = Body{"Earth", 1.0, 6371, openvg.RGB{11, 92, 227}}
	mars    = Body{"Mars", 1.54, 3396.2, openvg.RGB{240, 198, 29}}
	jupiter = Body{"Jupiter", 5.02, 69911, openvg.RGB{253, 199, 145}}
	saturn  = Body{"saturn", 9.46, 60268, openvg.RGB{224, 196, 34}}
	uranus  = Body{"uranus", 20.11, 25559, openvg.RGB{220, 241, 245}}
	neptune = Body{"neptune", 30.08, 24764, openvg.RGB{57, 182, 247}}

	solarSystem = []Body{sun, mercury, venus, earth, mars, jupiter, saturn, uranus, neptune}
)

func vmap(value, low1, high1, low2, high2 openvg.VGfloat) openvg.VGfloat {
	return low2 + (high2-low2)*(value-low1)/(high1-low1)
}

func light(x, y, r openvg.VGfloat, c openvg.RGB) {
	stops := []openvg.Offcolor{
		{0.0, c, 1},
		{0.50, openvg.RGB{c.Red / 2, c.Green / 2, c.Blue / 2}, 1},
	}
	openvg.FillRadialGradient(x, y, (x+r)*.90, y, r, stops)
}

func main() {

	width, height := openvg.Init()

	w := openvg.VGfloat(width)
	h := openvg.VGfloat(height)
	y := h / 2
	var (
		margin  openvg.VGfloat = 100.0
		minsize openvg.VGfloat = 7.0
		labeloc openvg.VGfloat = 100.0
	)
	bgcolor := "black"
	labelcolor := "white"
	maxsize := (h / 2) * 0.05

	origin := sun.distance
	mostDistant := neptune.distance
	firstSize := mercury.radius
	lastSize := neptune.radius

	openvg.Start(width, height)
	openvg.BackgroundColor(bgcolor)

	for _, p := range solarSystem {
		x := vmap(p.distance, origin, mostDistant, margin, w-margin)
		r := vmap(p.radius, firstSize, lastSize, minsize, maxsize)

		if p.name == "Sun" {
			openvg.FillRGB(p.color.Red, p.color.Green, p.color.Blue, 1)
			openvg.Circle(margin-(r/2), y, r)
		} else {
			light(x, y, r, p.color)
			openvg.Circle(x, y, r)
		}
		if p.name == "Earth" && len(os.Args) > 1 {
			openvg.StrokeColor(labelcolor)
			openvg.StrokeWidth(1)
			openvg.Line(x, y+(r/2), x, y+labeloc)
			openvg.StrokeWidth(0)
			openvg.FillColor(labelcolor)
			openvg.TextMid(x, y+labeloc+10, os.Args[1], "sans", 12)
		}
	}
	openvg.End()
	bufio.NewReader(os.Stdin).ReadByte()
	openvg.Finish()
}
