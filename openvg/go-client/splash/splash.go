// splash: show a splash screen image
// suggested by Mike Kazantsev
package main

import (
	"flag"
	"fmt"
	"image"
	"os"
	"os/signal"
	"syscall"

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
	var resize = flag.Bool("resize", false, "Resize image to fit the screen.")
	var bgcolor = flag.String("bg", "black", "Background color (named color or rgb(r,g,b)).")
	var fgcolor = flag.String("fg", "white", "text color (named color or rgb(r,g,b)).")
	var title = flag.String("t", "", "text annotation")
	var fontsize = flag.Float64("fp", 2.0, "fontsize %")
	var px = flag.Float64("px", 50, "x position %")
	var py = flag.Float64("py", 50, "y position %")
	var texty = flag.Float64("ty", 0, "text y %")
	var exit_code int
	defer func() {
		os.Exit(exit_code)
	}()
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `usage: %s [ flags ] image-path

Set specified image as an overlay screen via OpenVG lib.
Default is to put this image to the center.

`, os.Args[0])
		flag.PrintDefaults()
		exit_code = 1
		return
	}

	flag.Parse()
	if flag.NArg() != 1 {
		fmt.Fprintf(os.Stderr, "Exactly one image-path argument must be specified.\n")
		flag.Usage()
		exit_code = 2
		return
	}

	openvg.SaveTerm()
	w, h := openvg.Init()
	openvg.RawTerm()
	defer openvg.Finish()
	defer openvg.RestoreTerm()

	img, err := getimage(flag.Args()[0], w, h, *resize)
	if err != nil {
		exit_code = 3
		return
	}
	ib := img.Bounds()
	imw, imh := ib.Max.X-ib.Min.X, ib.Max.Y-ib.Min.Y
	sig_chan := make(chan os.Signal, 1)
	signal.Notify(sig_chan, os.Interrupt, os.Kill, syscall.SIGHUP, syscall.SIGTERM, syscall.SIGALRM)

	openvg.Start(w, h)
	openvg.BackgroundColor(*bgcolor)
	var x, y openvg.VGfloat
	if *px < 0 || *px > 100 {
		x = openvg.VGfloat(w)/2 - openvg.VGfloat(imw)/2
	} else {
		x = openvg.VGfloat(w)*openvg.VGfloat(*px/100) - openvg.VGfloat(imw)/2
	}

	if *py < 0 || *py > 100 {
		y = openvg.VGfloat(h)/2 - openvg.VGfloat(imh)/2
	} else {
		y = openvg.VGfloat(h)*openvg.VGfloat(*py/100) - openvg.VGfloat(imh)/2
	}
	openvg.Img(x, y, img)
	if len(*title) > 0 {
		var typ openvg.VGfloat
		fs := openvg.VGfloat(*fontsize/100.0) * openvg.VGfloat(w)
		if *texty == 0 {
			typ = y - fs*1.5
		} else {
			typ = openvg.VGfloat(h) * openvg.VGfloat(*texty/100)
		}
		openvg.FillColor(*fgcolor)
		openvg.TextMid(x+openvg.VGfloat(imw)/2, typ, *title, "sans", int(fs))
	}
	openvg.End()

	_ = <-sig_chan
}
