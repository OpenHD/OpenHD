//vgplot -- plot data (a stream of x,y coordinates)
package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"math"
	"os"

	"github.com/ajstarks/openvg"
)

// rawdata defines data as openvg.VGfloat x,y coordinates
type rawdata struct {
	x openvg.VGfloat
	y openvg.VGfloat
}

type options map[string]bool
type attributes map[string]string
type measures map[string]openvg.VGfloat

// plotset defines plot metadata
type plotset struct {
	opt  options
	attr attributes
	size measures
}

var (
	plotopt                                                      = options{}
	plotattr                                                     = attributes{}
	plotnum                                                      = measures{}
	ps                                                           = plotset{plotopt, plotattr, plotnum}
	plotw, ploth, plotc, gwidth, gheight, gutter, beginx, beginy openvg.VGfloat
)

// init initializes command flags and sets default options
func init() {

	// boolean options 
	showx := flag.Bool("showx", false, "show the xaxis")
	showy := flag.Bool("showy", false, "show the yaxis")
	showbar := flag.Bool("showbar", false, "show data bars")
	area := flag.Bool("area", false, "area chart")
	connect := flag.Bool("connect", true, "connect data points")
	showdot := flag.Bool("showdot", false, "show dots")
	showbg := flag.Bool("showbg", true, "show the background color")
	showfile := flag.Bool("showfile", false, "show the filename")
	sameplot := flag.Bool("sameplot", false, "plot on the same frame")

	// attributes
	bgcolor := flag.String("bgcolor", "silver", "plot background color")
	barcolor := flag.String("barcolor", "gray", "bar color")
	dotcolor := flag.String("dotcolor", "black", "dot color")
	linecolor := flag.String("linecolor", "gray", "line color")
	areacolor := flag.String("areacolor", "gray", "area color")
	font := flag.String("font", "Calibri,sans", "font")
	labelcolor := flag.String("labelcolor", "black", "label color")
	plotlabel := flag.String("label", "", "plot label")

	// sizes
	dotsize := flag.Float64("dotsize", 2, "dot size")
	linesize := flag.Float64("linesize", 2, "line size")
	barsize := flag.Float64("barsize", 2, "bar size")
	fontsize := flag.Float64("fontsize", 11, "font size")
	xinterval := flag.Float64("xint", 10, "x axis interval")
	yinterval := flag.Float64("yint", 4, "y axis interval")

	// meta options
	
	gw := flag.Float64("width", 1024, "canvas width")
 	gh := flag.Float64("height", 768, "canvas height")
	pw := flag.Float64("pw", 500, "plot width")
	ph := flag.Float64("ph", 500, "plot height")
	gu := flag.Float64("gutter", *ph/10, "gutter")
	bx := flag.Float64("bx", 100, "initial x")
	by := flag.Float64("by", *gh-10, "initial y")
	pc := flag.Float64("pc", 2, "plot columns")
	flag.Parse()

	// fill in the plotset -- all options, attributes, and sizes
	plotopt["showx"] = *showx
	plotopt["showy"] = *showy
	plotopt["showbar"] = *showbar
	plotopt["area"] = *area
	plotopt["connect"] = *connect
	plotopt["showdot"] = *showdot
	plotopt["showbg"] = *showbg
	plotopt["showfile"] = *showfile
	plotopt["sameplot"] = *sameplot

	plotattr["bgcolor"] = *bgcolor
	plotattr["barcolor"] = *barcolor
	plotattr["linecolor"] = *linecolor
	plotattr["dotcolor"] = *dotcolor
	plotattr["areacolor"] = *areacolor
	plotattr["font"] = *font
	plotattr["label"] = *plotlabel
	plotattr["labelcolor"] = *labelcolor

	plotnum["dotsize"] = openvg.VGfloat(*dotsize)
	plotnum["linesize"] = openvg.VGfloat(*linesize)
	plotnum["fontsize"] = openvg.VGfloat(*fontsize)
	plotnum["xinterval"] = openvg.VGfloat(*xinterval)
	plotnum["yinterval"] = openvg.VGfloat(*yinterval)
	plotnum["barsize"] = openvg.VGfloat(*barsize)

	gutter = openvg.VGfloat(*gu)
	gwidth = openvg.VGfloat(*gw)
	gheight = openvg.VGfloat(*gh)
	plotw = openvg.VGfloat(*pw)
	ploth = openvg.VGfloat(*ph)
	beginx = openvg.VGfloat(*bx)
	beginy = openvg.VGfloat(*by)
	plotc = openvg.VGfloat(*pc)
}

// fmap maps world data to document coordinates
func fmap(value openvg.VGfloat, low1 openvg.VGfloat, high1 openvg.VGfloat, low2 openvg.VGfloat, high2 openvg.VGfloat) openvg.VGfloat {
	return low2 + (high2-low2)*(value-low1)/(high1-low1)
}

// doplot opens a file and makes a plot
func doplot(x, y openvg.VGfloat, location string) {
	var f *os.File
	var err error
	if len(location) > 0 {
		if plotopt["showfile"] {
			plotattr["label"] = location
		}
		f, err = os.Open(location)
	} else {
		f = os.Stdin
	}
	if err != nil {
		fmt.Fprintf(os.Stderr, "%v\n", err)
		return
	}
	nd, data := readxy(f)
	f.Close()

	if nd >= 2 {
		plot(x, y, plotw, ploth, ps, data)
	}
}

// plot places a plot at the specified location with the specified dimemsions
// using the specified settings, using the specified data
func plot(x, y, w, h openvg.VGfloat, settings plotset, d []rawdata) {
	nd := len(d)
	if nd < 2 {
		fmt.Fprintf(os.Stderr, "%d is not enough points to plot\n", len(d))
		return
	}
	// Compute the minima and maxima of the data
	maxx, minx := d[0].x, d[0].x
	maxy, miny := d[0].y, d[0].y
	for _, v := range d {

		if v.x > maxx {
			maxx = v.x
		}
		if v.y > maxy {
			maxy = v.y
		}
		if v.x < minx {
			minx = v.x
		}
		if v.y < miny {
			miny = v.y
		}
	}
	// Prepare for a area or line chart by allocating
	// polygon coordinates; for the horizon plot, you need two extra coordinates
	// for the extrema.
	needpoly := settings.opt["area"] || settings.opt["connect"]
	var xpoly, ypoly []openvg.VGfloat
	if needpoly {
		xpoly = make([]openvg.VGfloat, nd+2)
		ypoly = make([]openvg.VGfloat, nd+2)
		// preload the extrema of the polygon, 
		// the bottom left and bottom right of the plot's rectangle
		xpoly[0] = x
		ypoly[0] = y
		xpoly[nd+1] = x + w
		ypoly[nd+1] = y
	}
	// Draw the plot's bounding rectangle
	if settings.opt["showbg"] && !settings.opt["sameplot"] {
		openvg.FillColor(settings.attr["bgcolor"])
		openvg.Rect(x, y, w, h)
	}
	// Loop through the data, drawing items as specified
	spacer := openvg.VGfloat(10.0)
	for i, v := range d {
		xp := fmap(v.x, minx, maxx, x, x+w)
		yp := fmap(v.y, miny, maxy, y, y+h)
		if needpoly {
			xpoly[i+1] = xp
			ypoly[i+1] = yp
		}
		if settings.opt["showbar"] {
			openvg.StrokeColor(settings.attr["barcolor"])
			openvg.StrokeWidth(settings.size["barsize"])
			openvg.Line(xp, yp, xp, y)
		}
		if settings.opt["showdot"] {
			openvg.FillColor(settings.attr["dotcolor"])
			openvg.StrokeWidth(0)
			openvg.Circle(xp, yp, settings.size["dotsize"])
		}
		if settings.opt["showx"] {
			if i%int(settings.size["xinterval"]) == 0 {
				openvg.FillColor("black")
				openvg.TextMid(xp, y-(spacer*2), fmt.Sprintf("%d", int(v.x)), settings.attr["font"], int(settings.size["fontsize"]))
				openvg.StrokeColor("silver")
				openvg.StrokeWidth(1)
				openvg.Line(xp, y, xp, y-spacer)
			}
			openvg.StrokeWidth(0)
		}
	}
	// Done constructing the points for the area or line plots, display them in one shot
	if settings.opt["area"] {
		openvg.FillColor(settings.attr["areacolor"])
		openvg.Polygon(xpoly, ypoly)
	}

	if settings.opt["connect"] {
		openvg.StrokeColor(settings.attr["linecolor"])
		openvg.StrokeWidth(settings.size["linesize"])
		openvg.Polyline(xpoly[1:nd+1], ypoly[1:nd+1])
	}
	// Put on the y axis labels, if specified
	if settings.opt["showy"] {
		bot := openvg.VGfloat(math.Floor(float64(miny)))
		top := openvg.VGfloat(math.Ceil(float64(maxy)))
		yrange := top - bot
		interval := yrange / openvg.VGfloat(settings.size["yinterval"])
		for yax := bot; yax <= top; yax += interval {
			yaxp := fmap(yax, bot, top, openvg.VGfloat(y), openvg.VGfloat(y+h))
			openvg.FillColor("black")
			openvg.TextEnd(x-spacer, yaxp, fmt.Sprintf("%.1f", yax), settings.attr["font"], int(settings.size["fontsize"]))
			openvg.StrokeColor("silver")
			openvg.StrokeWidth(1)
			openvg.Line(x-spacer, yaxp, x, yaxp)
		}
		openvg.StrokeWidth(0)
	}
	// Finally, tack on the label, if specified
	if len(settings.attr["label"]) > 0 {
		openvg.FillColor(settings.attr["labelcolor"], 0.3)
		openvg.TextMid(x+(w/2), y+(h/2), settings.attr["label"], settings.attr["font"], int(w/8)) // int(settings.size["fontsize"]))
	}
	openvg.StrokeWidth(0)
}

// readxy reads coordinates (x,y openvg.VGfloat values) from a io.Reader
func readxy(f io.Reader) (int, []rawdata) {
	var (
		r     rawdata
		err   error
		n, nf int
	)
	data := make([]rawdata, 1)
	for ; err == nil; n++ {
		if n > 0 {
			data = append(data, r)
		}
		nf, err = fmt.Fscan(f, &data[n].x, &data[n].y)
		if nf != 2 {
			continue
		}
	}
	return n - 1, data[0 : n-1]
}

// plotgrid places plots on a grid, governed by a number of columns.
func plotgrid(x, y openvg.VGfloat, files []string) {
	px := x
	for i, f := range files {
		if i > 0 && i%int(plotc) == 0 && !plotopt["sameplot"] {
			px = x
			y -= (ploth + gutter)
		}
		doplot(px, y, f)
		if !plotopt["sameplot"] {
			px += (plotw + gutter)
		}
	}
}

// main plots data from specified files or standard input in a 
// grid where plotc specifies the number of columns.
func main() {
	w, h := openvg.Init()
	openvg.Start(w, h)
	openvg.FillColor("white")
	openvg.Rect(0, 0, gwidth, gheight)
	filenames := flag.Args()
	if len(filenames) == 0 {
		doplot(beginx, beginy, "")
	} else {
		plotgrid(beginx, beginy, filenames)
	}
	openvg.End()
	bufio.NewReader(os.Stdin).ReadByte()
	openvg.Finish()
}
