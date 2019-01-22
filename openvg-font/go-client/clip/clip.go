//
// clip: test rectangular clipping
//
package main

import (
	"bufio"
	"github.com/ajstarks/openvg"
	"os"
)

func main() {
	var cx, cy, cw, ch, midy int
	message := "Now is the time for all good men to come to the aid of the party"

	w, h := openvg.Init()
	var speed openvg.VGfloat = 15.0
	var x openvg.VGfloat = 0
	midy = (h / 2)
	fontsize := w / 50
	cx = 0
	ch = fontsize * 2
	cw = w
	cy = midy - (ch / 2)

	rx, ry, rw, rh := openvg.VGfloat(cx), openvg.VGfloat(cy), openvg.VGfloat(cw), openvg.VGfloat(ch)
	// scroll the text, only in the clipping rectangle
	for x = 0; x < rw+speed; x += speed {
		openvg.Start(w, h)
		openvg.Background(255, 255, 255)
		openvg.FillRGB(0, 0, 0, .2)
		openvg.Rect(rx, ry, rw, rh)
		openvg.ClipRect(cx, cy, cw, ch)
		openvg.Translate(x, ry+openvg.VGfloat(fontsize/2))
		openvg.FillRGB(0, 0, 0, 1)
		openvg.Text(0, 0, message, "sans", fontsize)
		openvg.ClipEnd()
		openvg.End()
	}
	bufio.NewReader(os.Stdin).ReadBytes('\n')
	openvg.Finish()
	os.Exit(0)
}
