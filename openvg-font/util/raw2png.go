//raw2png - convert RGBA bytes to PNG
package main

import (
	"bufio"
	"image"
	"image/png"
	"io"
	"log"
	"os"
	"strconv"
)

func main() {

	var (
		width  = 1920
		height = 1080
	)
	if len(os.Args) > 2 {
		width, _ = strconv.Atoi(os.Args[1])
		height, _ = strconv.Atoi(os.Args[2])
	}
	r := bufio.NewReader(os.Stdin)
	m := image.NewNRGBA(image.Rect(0, 0, width, height))
	for y := height - 1; y >= 0; y-- { // OpenVG has origin at lower left, y increasing up
		i := m.PixOffset(0, y)
		if _, err := io.ReadFull(r, m.Pix[i:i+4*width]); err != nil {
			log.Fatal(err)
		}
	}
	png.Encode(os.Stdout, m)
}
