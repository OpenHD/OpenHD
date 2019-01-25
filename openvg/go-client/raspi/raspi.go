// raspberry pi self-portrait
package main

import (
	"bufio"
	"github.com/ajstarks/openvg"
	"os"
)

// makepi draws the Raspberry Pi
func makepi(x, y, w, h openvg.VGfloat) {
	// dimensions
	socw := h / 5
	compw := h / 5
	cjw := h / 10
	cjh := h / 8
	audw := h / 5
	aujw := h / 10
	aujh := cjh / 2
	hdw := w / 6
	hdh := w / 10
	gpw := w / 3
	gph := h / 8
	pw := h / 10
	usw := w / 5
	ush := h / 5
	etw := w / 5
	eth := h / 5
	sdw := w / 6
	sdh := w / 4
	offset := (w / 2) / 10
	w34 := (w * 3) / 4
	w2 := w / 2
	h2 := h / 2
	h40 := (h * 2) / 5

	openvg.FillRGB(0, 128, 0, 1)
	openvg.Rect(x, y, w, h) // board

	openvg.FillRGB(255, 255, 0, 1)
	openvg.Rect(x+w2, (y+h)-compw, compw, compw) // composite
	openvg.FillRGB(192, 192, 192, 1)
	openvg.Rect(x+w2+(cjw/2), y+h, cjw, cjh) // composite jack

	openvg.FillRGB(0, 0, 0, 1)
	openvg.Rect(x+w34, y+h-audw, audw, audw)     // audio
	openvg.Rect(x+w34+(aujw/2), y+h, aujw, aujh) // audio jack

	openvg.FillRGB(192, 192, 192, 1)
	openvg.Rect(x+w2, y, hdw, hdh)                 // HDMI
	openvg.Rect((x+w)-etw, y, etw, eth)            // Ethernet
	openvg.Rect((x+w+offset)-usw, y+h40, usw, ush) // USB
	openvg.Rect(x, y, pw, pw)                      // Power

	openvg.FillRGB(0, 0, 0, 1)
	openvg.Rect(x+(w*2)/5, y+h40, socw, socw) // SoC
	openvg.Rect(x, (y+h)-gph, gpw, gph)       // GPIO
	openvg.FillRGB(0, 0, 255, 1)
	openvg.Rect(x-sdw, (y+h2)-(sdh/2), sdw, sdh) // SD card
}

// WaitEnd paused for user input, then terminates
func WaitEnd() {
	openvg.End()
	bufio.NewReader(os.Stdin).ReadBytes('\n')
	openvg.Finish()
}

// raspberry pi, scaled to the screen dimensions
func main() {
	w, h := openvg.Init()
	midx := openvg.VGfloat(w) / 2
	midy := openvg.VGfloat(h) / 2
	rw := midx
	rh := (rw * 2) / 3
	fontsize := w / 25
	openvg.Start(w, h)
	openvg.Background(255, 255, 255)
	makepi(midx-(rw/2), midy-(rh/2), rw, rh)
	makepi(200, 100, 75, 50)
	openvg.FillRGB(128, 0, 0, 1)
	openvg.TextMid(midx, midy-(rh/2)-openvg.VGfloat(fontsize*2), "The Raspberry Pi", "sans", fontsize)
	WaitEnd()
}
