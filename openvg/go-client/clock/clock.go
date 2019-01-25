// clock
package main

import (
	"flag"
	"math"
	"os"
	"os/signal"
	"time"

	"github.com/ajstarks/openvg"
)

const (
	deg2rad = math.Pi / 180
	edge    = 1.15
)

var handstyle, framecolor, dotcolor, digitcolor, facecolor, hrcolor, mincolor, secolor, centercolor string
var secline, showdots, showdigits bool

var hourangles = [12]float64{
	90, 60, 30, // 12, 1, 2
	0, 330, 300, // 3, 4, 5
	270, 240, 210, // 6, 7, 8
	180, 150, 120, // 9, 10, 11
}

var minangles = [60]float64{
	90, 84, 78, 72, 66, // 0-4
	60, 54, 48, 42, 36, // 5-9
	30, 24, 18, 12, 6, // 10-14
	0, 354, 348, 342, 336, // 15-19
	330, 324, 318, 312, 306, // 20-24
	300, 294, 288, 282, 276, // 25-29
	270, 264, 258, 252, 246, // 30-34
	240, 234, 228, 222, 216, // 35-39
	210, 204, 198, 192, 186, // 40-44
	180, 174, 168, 162, 156, // 45-49
	150, 144, 138, 132, 126, // 50-54
	120, 114, 108, 102, 96, // 55-59
}

func timecoord(x, y, r openvg.VGfloat, hour, min, sec int) (hx, hy, mx, my, sx, sy openvg.VGfloat) {
	radius := float64(r)
	hradius := radius * 0.6  // hour hand is 60% to the edge of the face
	mradius := radius * 0.9  // minute hand is 90% to the edge
	sradius := radius * edge // second hand is at the edge

	t := hourangles[hour%12]
	t = minadjust(t, min) * deg2rad
	hx = x + openvg.VGfloat(hradius*math.Cos(t))
	hy = y + openvg.VGfloat(hradius*math.Sin(t))

	t = minangles[min] * deg2rad
	mx = x + openvg.VGfloat(mradius*math.Cos(t))
	my = y + openvg.VGfloat(mradius*math.Sin(t))

	t = minangles[sec] * deg2rad
	sx = x + openvg.VGfloat(sradius*math.Cos(t))
	sy = y + openvg.VGfloat(sradius*math.Sin(t))
	return
}

var hourdigits = [12]string{"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"}

func frame(cx, cy, framesize, facesize, textsize openvg.VGfloat, framecolor, facecolor string) {
	openvg.FillColor(framecolor)
	openvg.Roundrect(cx-framesize/2, cy-framesize/2, framesize, framesize, textsize, textsize)
	openvg.FillColor(facecolor)
	openvg.Ellipse(cx, cy, facesize*2.4, facesize*2.4)
}

func face(x, y, r openvg.VGfloat, ts int) {
	var fx, fy, va openvg.VGfloat
	va = openvg.VGfloat(ts) / 2.0
	secsize := openvg.VGfloat(ts) / 3
	radius := float64(r)
	ir := radius * 1.2
	// hour display
	openvg.FillColor(digitcolor)
	openvg.StrokeColor(digitcolor)
	openvg.StrokeWidth(5)
	for h := 12; h > 0; h-- {
		t := hourangles[h%12] * deg2rad
		fx = x + openvg.VGfloat(radius*math.Cos(t))
		fy = y + openvg.VGfloat(radius*math.Sin(t))
		ix := x + openvg.VGfloat(ir*math.Cos(t))
		iy := y + openvg.VGfloat(ir*math.Sin(t))
		if showdigits {
			openvg.TextMid(fx, fy-va, hourdigits[h%12], "sans", ts)
		} else {
			openvg.Line(fx, fy, ix, iy)
		}
	}
	// second display
	openvg.FillColor(dotcolor)
	openvg.StrokeColor(dotcolor)
	openvg.StrokeWidth(2)
	re := radius * edge
	for a := 0.0; a < 360; a += 6.0 {
		t := a * deg2rad
		sx := x + openvg.VGfloat(re*math.Cos(t))
		sy := y + openvg.VGfloat(re*math.Sin(t))
		if showdots {
			openvg.Ellipse(sx, sy, secsize, secsize)
		} else {
			ix := x + openvg.VGfloat(ir*math.Cos(t))
			iy := y + openvg.VGfloat(ir*math.Sin(t))
			openvg.Line(sx, sy, ix, iy)
		}
	}
	openvg.StrokeWidth(0)

}

// if the hour is > half-elapsed, adjust the hour angle to
// reflect the fraction between the current and subsequent hour
func minadjust(t float64, value int) float64 {
	if value > 30 {
		return t - (30.0 * (float64(value) / 60))
	}
	return t
}

func arrowhand(cx, cy, px, py, r openvg.VGfloat, t float64, value int, color string) {
	ax := []openvg.VGfloat{cx, 0, px, 0, cx}
	ay := []openvg.VGfloat{cy, 0, py, 0, cy}
	t = minadjust(t, value) * deg2rad
	rf := float64(r * 0.9)
	tf := math.Pi / 45.0
	ax[1] = cx + openvg.VGfloat(rf*math.Cos(t-tf))
	ay[1] = cy + openvg.VGfloat(rf*math.Sin(t-tf))
	ax[3] = cx + openvg.VGfloat(rf*math.Cos(t+tf))
	ay[3] = cy + openvg.VGfloat(rf*math.Sin(t+tf))
	openvg.FillColor(color)
	openvg.Polygon(ax, ay)
}

func combohand(cx, cy, px, py, r, stroke openvg.VGfloat, t float64, value int, color string) {
	thinr := float64(r * 0.25)
	t = minadjust(t, value) * deg2rad
	tx := cx + openvg.VGfloat(thinr*math.Cos(t))
	ty := cy + openvg.VGfloat(thinr*math.Sin(t))
	openvg.FillColor(color)
	openvg.Ellipse(px, py, stroke*2, stroke*2)
	openvg.Ellipse(tx, ty, stroke*2, stroke*2)
	openvg.StrokeWidth(stroke)
	openvg.StrokeColor(color)
	openvg.Line(cx, cy, tx, ty)
	openvg.StrokeWidth(stroke * 2)
	openvg.Line(tx, ty, px, py)
	openvg.StrokeWidth(0)
}

func roundhand(cx, cy, px, py, stroke openvg.VGfloat, color string) {
	openvg.StrokeWidth(stroke)
	openvg.StrokeColor(color)
	openvg.Line(cx, cy, px, py)
	openvg.StrokeWidth(0)
	openvg.FillColor(color)
	openvg.Ellipse(px, py, stroke, stroke)
}

func secondhand(cx, cy, sx, sy, textsize openvg.VGfloat) {
	openvg.FillColor(secolor, 0.4)
	openvg.Ellipse(sx, sy, textsize, textsize)
	if secline {
		openvg.StrokeWidth(textsize / 6)
		openvg.StrokeColor(secolor)
		openvg.Line(cx, cy, sx, sy)
		openvg.StrokeWidth(0)
	}
}

func centerdot(cx, cy, size openvg.VGfloat) {
	openvg.FillColor(centercolor)
	openvg.Ellipse(cx, cy, size, size)
}

func main() {
	flag.StringVar(&mincolor, "mincolor", "maroon", "minute color")
	flag.StringVar(&facecolor, "facecolor", "white", "face color")
	flag.StringVar(&hrcolor, "hourcolor", "gray", "hour color")
	flag.StringVar(&secolor, "secolor", "maroon", "second color")
	flag.StringVar(&digitcolor, "digitcolor", "black", "digit color")
	flag.StringVar(&dotcolor, "dotcolor", "silver", "dotcolor")
	flag.StringVar(&framecolor, "framecolor", "gray", "frame color")
	flag.StringVar(&centercolor, "centercolor", "black", "center color")
	flag.StringVar(&handstyle, "handstyle", "combo", "handstyle (combo, arrow, or round)")
	flag.BoolVar(&showdigits, "showdigits", true, "show digits")
	flag.BoolVar(&showdots, "showdots", true, "show dots")
	flag.BoolVar(&secline, "secline", false, "show second hand")
	flag.Parse()

	width, height := openvg.Init()
	cx := openvg.VGfloat(width / 2)
	cy := openvg.VGfloat(height / 2)
	facesize := openvg.VGfloat(cy * 0.5)
	textsize := facesize / 10.0
	framesize := facesize * 2.5
	hourstroke := textsize
	minstroke := hourstroke * .6

	// set up the ticker and signal handler
	ticker := time.NewTicker(1 * time.Second)
	sigint := make(chan os.Signal, 1)
	signal.Notify(sigint, os.Interrupt)

	// main loop: for each second, draw the clock components
	openvg.Start(width, height)
	for {
		select {
		case <-ticker.C:
			// get the current time
			hr, min, sec := time.Now().Clock()

			// compute element coordinates
			hx, hy, mx, my, sx, sy := timecoord(cx, cy, facesize, hr, min, sec)

			// frame and clock face
			frame(cx, cy, framesize, facesize, textsize, framecolor, facecolor)
			face(cx, cy, facesize, int(textsize*1.5))

			// hour and minute hands
			switch handstyle {
			case "round", "r":
				roundhand(cx, cy, mx, my, minstroke, mincolor)
				roundhand(cx, cy, hx, hy, hourstroke, hrcolor)
			case "arrow", "a":
				arrowhand(cx, cy, mx, my, facesize*0.9, minangles[min], 0, mincolor)
				arrowhand(cx, cy, hx, hy, facesize*0.6, hourangles[hr%12], min, hrcolor)
			case "combo", "c":
				combohand(cx, cy, mx, my, facesize*0.9, minstroke/2, minangles[min], 0, mincolor)
				combohand(cx, cy, hx, hy, facesize*0.6, minstroke/2, hourangles[hr%12], min, hrcolor)
			default:
				combohand(cx, cy, mx, my, facesize*0.9, minstroke/2, minangles[min], 0, mincolor)
				combohand(cx, cy, hx, hy, facesize*0.6, minstroke/2, hourangles[hr%12], min, hrcolor)
			}

			// second indicator
			secondhand(cx, cy, sx, sy, textsize)

			// center dot
			centerdot(cx, cy, textsize)

			// make the picture
			openvg.End()
		case <-sigint:
			openvg.Finish()
			return
		}
	}
}
