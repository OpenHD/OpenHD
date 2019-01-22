// picshow: show pictures
package main

import (
	"flag"
	"fmt"
	"image"
	"os"
	"time"

	"github.com/ajstarks/openvg"
	"github.com/disintegration/gift"
)

func getimage(image_path string, w, h int, resized bool) (image.Image, error) {
	f, err := os.Open(image_path)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%v\n", err)
		return nil, err
	}
	defer f.Close()
	img, _, err := image.Decode(f)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%v\n", err)
		return nil, err
	}
	if resized {
		g := gift.New(gift.ResizeToFit(w, h, gift.BoxResampling))
		res := image.NewRGBA(g.Bounds(img.Bounds()))
		g.Draw(res, img)
		return res, nil
	}
	return img, nil
}

func main() {
	var loop = flag.Bool("loop", false, "Loop the show")
	var resize = flag.Bool("resize", false, `Resize image to fit the screen.`)
	var bgcolor = flag.String("bg", "black", `Background color (named color or rgb(r,g,b)).`)
	var delay = flag.Duration("delay", 2*time.Second, "Delay between pictures")
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "usage: %s [ flags ] images...\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(1)
	}

	flag.Parse()
	w, h := openvg.Init()
	images := []image.Image{}
	imgcount := 0
	for _, imgfile := range flag.Args() {
		fmt.Fprintf(os.Stderr, "loading %q ", imgfile)
		img, err := getimage(imgfile, w, h, *resize)
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			continue
		}
		images = append(images, img)
		imgcount++
		fmt.Fprintln(os.Stderr, "ok")
	}
	for {
		for _, img := range images {
			ib := img.Bounds()
			imw, imh := ib.Max.X-ib.Min.X, ib.Max.Y-ib.Min.Y
			openvg.Start(w, h)
			openvg.BackgroundColor(*bgcolor)
			x, y := openvg.VGfloat(w)/2-openvg.VGfloat(imw)/2, openvg.VGfloat(h)/2-openvg.VGfloat(imh)/2
			openvg.Img(x, y, img)
			openvg.End()
			time.Sleep(*delay)
		}
		if !*loop {
			break
		}
	}
}
