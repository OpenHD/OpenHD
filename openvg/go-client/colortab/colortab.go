// colortab -- make a color/code placemat
package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"strings"

	"github.com/ajstarks/openvg"
)

func main() {
	var (
		filename     = flag.String("f", "svgcolors.txt", "input file")
		fontname     = flag.String("font", "sans", "fontname")
		neg          = flag.Bool("neg", false, "negative")
		showrgb      = flag.Bool("rgb", false, "show RGB")
		showcode     = flag.Bool("showcode", true, "show colors and codes")
		circsw       = flag.Bool("circle", true, "circle swatch")
		outline      = flag.Bool("outline", false, "outline swatch")
		fontsize     = flag.Int("fs", 12, "fontsize")
		rowsize      = flag.Int("r", 32, "rowsize")
		colw         = flag.Float64("c", 340, "column size")
		swatch       = flag.Float64("s", 16, "swatch size")
		gutter       = flag.Float64("g", 12, "gutter")
		err          error
		tcolor, line string
	)

	flag.Parse()
	f, oerr := os.Open(*filename)
	if oerr != nil {
		fmt.Fprintf(os.Stderr, "%v\n", oerr)
		return
	}
	width, height := openvg.Init()

	openvg.Start(width, height)
	fw := openvg.VGfloat(width)
	fh := openvg.VGfloat(height)
	if *neg {
		openvg.FillColor("black")
		openvg.Rect(0, 0, fw, fh)
		tcolor = "white"
	} else {
		openvg.FillColor("white")
		openvg.Rect(0, 0, fw, fh)
		tcolor = "black"
	}
	top := fh - 32.0
	left := openvg.VGfloat(32.0)
	cw := openvg.VGfloat(*colw)
	sw := openvg.VGfloat(*swatch)
	g := openvg.VGfloat(*gutter)
	in := bufio.NewReader(f)

	for x, y, nr := left, top, 0; err == nil; nr++ {
		line, err = in.ReadString('\n')
		fields := strings.Split(strings.TrimSpace(line), "\t")
		if nr%*rowsize == 0 && nr > 0 {
			x += cw 
			y = top
		}
		if len(fields) == 3 {
			var red, green, blue uint8
			fmt.Sscanf(fields[2], "%d,%d,%d", &red, &green, &blue)
			openvg.FillRGB(red, green, blue, 1)
			if *outline {
				openvg.StrokeColor("black")
				openvg.StrokeWidth(1)
			}
			if *circsw {
				openvg.Circle(x+sw/2, y+sw/2, sw)
			} else {
				openvg.Rect(x, y, sw, sw)
			}
			openvg.StrokeWidth(0)
			openvg.FillColor(tcolor)
			openvg.Text(x+sw+openvg.VGfloat(*fontsize/2), y, fields[0], *fontname, *fontsize)
			var label string
			if *showcode {
				if *showrgb {
					label = fields[1]
				} else {
					label = fields[2]
				}
				openvg.FillColor("gray")
				openvg.TextEnd(x+cw-(sw+g), y, label, *fontname, *fontsize)
			}
		}
		y -= (sw + g)
	}
	openvg.End()
	bufio.NewReader(os.Stdin).ReadBytes('\n')
	openvg.Finish()
}
