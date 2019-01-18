// twh: time, weather, headlines
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"math"
	"math/rand"
	"net/http"
	"os"
	"os/signal"
	"strings"
	"time"

	"github.com/ajstarks/openvg"
)

// Forecast is weather information from forecast.io
type Forecast struct {
	Lat       float64 `json:"latitude"`
	Long      float64 `json:"longitude"`
	Currently current `json:"currently"`
}

type current struct {
	Summary     string  `json:"summary"`
	Icon        string  `json:"icon"`
	PrecipProb  float64 `json:"precipProbability"`
	Temperature float64 `json:"temperature"`
	FeelsLike   float64 `json:"apparentTemperature"`
}

// NYTHeadlines is the headline info from the New York Times
type NYTHeadlines struct {
	Status     string   `json:"status"`
	Copyright  string   `json:"copyright"`
	NumResults int      `json:"num_results"`
	Results    []result `json:"results"`
}

type result struct {
	Section    string `json:"section"`
	Subsection string `json:"subsection"`
	Title      string `json:"title"`
	Abstract   string `json:"abstract"`
}

// HNtop and HNitem are Hacker News top stories list and items
type HNTop []int

type HNitem struct {
	By    string `json:"by"`
	Title string `json:"title"`
}

// display is the dispaly context
type display struct {
	width, height      openvg.VGfloat
	bgcolor, textcolor string
}

// dimen define a rectangular region
type dimen struct {
	x, y, width, height openvg.VGfloat
}

const (
	weatherfmt    = "https://api.forecast.io/forecast/%s/%s/?exclude=hourly,daily,minutely,flags"
	NYTfmt        = "http://api.nytimes.com/svc/news/v3/content/all/%s/.json?api-key=%s&limit=5"
	HNTopURL      = "https://hacker-news.firebaseio.com/v0/topstories.json"
	HNItemfmt     = "https://hacker-news.firebaseio.com/v0/item/%d.json"
	weatherAPIkey = "-api-key-"
	NYTAPIkey     = "-api-key-"
)

var fromHTML = strings.NewReplacer(
	"‘", "'",
	"’", "'",
	"—", "--",
	"&#8216;", "'",
	"&#8217;", "'",
	"&#8220;", `"`,
	"&#8221;", `"`,
	"&lsquo;", "'",
	"&rsquo;", "'",
	"&ndash;", "-",
	"&mdash;", "--",
	"&#8211;", "-",
	"&#8212;", "--",
	"&#8230;", "...",
	"&amp;", "&")

// show the current time, weather and headlines
func main() {
	var (
		section    = flag.String("h", "u.s.", "headline type (arts, health, sports, science, technology, u.s., world, hn)")
		location   = flag.String("loc", "40.6213,-74.4395", "lat,long for weather")
		bgcolor    = flag.String("bg", "slateblue", "background color")
		textcolor  = flag.String("tc", "white", "text color")
		width      = flag.Int("width", 0, "screen width")
		height     = flag.Int("height", 0, "screen height")
		smartcolor = flag.Bool("sc", false, "smart colors")
	)
	flag.Parse()

	// initial display
	dw, dh := openvg.Init()
	if *width > 0 && *height > 0 {
		dw, dh = *width, *height
	}
	openvg.Start(dw, dh)
	canvas := display{
		width:     openvg.VGfloat(dw),
		height:    openvg.VGfloat(dh),
		bgcolor:   *bgcolor,
		textcolor: *textcolor,
	}
	canvas.countdown()
	openvg.End()
	canvas.clock(*smartcolor)
	canvas.weather(*location)
	canvas.headlines(*section)

	// update on specific intervals, shutdown on interrupt
	dateticker := time.NewTicker(1 * time.Minute)
	weatherticker := time.NewTicker(5 * time.Minute)
	headticker := time.NewTicker(10 * time.Minute)
	sigint := make(chan os.Signal, 1)
	signal.Notify(sigint, os.Interrupt)

	for {
		select {
		case <-dateticker.C:
			canvas.clock(*smartcolor)
		case <-weatherticker.C:
			canvas.weather(*location)
		case <-headticker.C:
			canvas.headlines(*section)
		case <-sigint:
			openvg.Finish()
			os.Exit(0)
		}
	}
}

// clock displays the current time
func (d *display) clock(smartcolor bool) {
	if smartcolor {
		d.bgcolor = daycolor()
	}
	cdim := dimen{x: d.width / 2, y: d.height / 2, width: d.width / 2, height: d.height / 2}
	cdim.regionFill(d.bgcolor, d.textcolor)
	clocksize := d.width / 20
	cs := int(clocksize)
	x := d.width * 0.95
	y := d.height * 0.70

	now := time.Now()
	openvg.TextEnd(x, y, now.Format("3:04 pm"), "sans", cs)
	openvg.TextEnd(x, y+(clocksize*2), now.Format("Monday January _2"), "sans", cs/2)
	openvg.End()
}

// weather retrieves data from the forecast.io API, decodes and displays it.
func (d *display) weather(latlong string) {
	wdim := dimen{x: 0, y: d.height / 2, width: d.width / 2, height: d.height / 2}
	r, err := netread(fmt.Sprintf(weatherfmt, weatherAPIkey, latlong))
	if err != nil {
		fmt.Fprintf(os.Stderr, "Weather read error: %v\n", err)
		wdim.gerror(d.bgcolor, d.textcolor, "no weather")
		return
	}
	defer r.Close()
	var data Forecast
	err = json.NewDecoder(r).Decode(&data)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%v\n", err)
		wdim.gerror(d.bgcolor, d.textcolor, "no weather")
		return
	}
	x := d.width * 0.05
	y := d.height * 0.70
	wsize := d.width / 20
	spacing := wsize * 2.0
	w1 := int(wsize)
	w2 := w1 / 2
	w3 := w1 / 4
	c := data.Currently
	temp := fmt.Sprintf("%0.f°", c.Temperature)
	tw := openvg.TextWidth(temp, "sans", w1)
	wdim.regionFill(d.bgcolor, d.textcolor)
	openvg.Text(x, y, temp, "sans", w1)
	if c.Temperature-c.FeelsLike > 1 {
		openvg.Text(x, y-(spacing/3),
			fmt.Sprintf("(feels like %0.f°)", c.FeelsLike), "sans", w3)
	}
	openvg.Text(x, y+spacing, c.Summary, "sans", w2)
	if c.PrecipProb > 0 {
		openvg.Text(x, y-(spacing*.6),
			fmt.Sprintf("%0.f%% Chance of precipitation", c.PrecipProb*100), "sans", w3)
	}

	ic := dimen{
		x:      x + tw + d.width*0.01,
		y:      d.height * 0.67,
		width:  d.width / 10,
		height: d.width / 10,
	}

	switch c.Icon {
	case "clear-day":
		ic.sun("orange")
	case "clear-night":
		ic.moon(d.bgcolor, d.textcolor)
	case "rain":
		ic.rain("skyblue")
	case "snow":
		ic.snow(d.textcolor)
	case "wind":
		ic.wind(d.bgcolor, d.textcolor)
	case "fog":
		ic.fog(d.textcolor)
	case "cloudy":
		ic.cloud(d.textcolor)
	case "partly-cloudy-day":
		ic.pcloud(d.textcolor)
	case "partly-cloudy-night":
		ic.npcloud("darkgray", d.textcolor)
	}
	openvg.End()
}

// headlines shows hacker news or NYT headlines
func (d *display) headlines(headlinetype string) {
	if headlinetype == "hn" {
		d.hackernews(5)
	} else {
		d.nytheadlines(headlinetype)
	}
}

// hackernews shows the top n articles from Hackernews
func (d *display) hackernews(n int) {
	hdim := dimen{x: 0, y: 0, width: d.width, height: d.height / 2}
	r, err := netread(HNTopURL)
	if err != nil {
		fmt.Fprintf(os.Stderr, "headline read error: %v\n", err)
		hdim.gerror(d.bgcolor, d.textcolor, "no headlines")
		return
	}
	var hnid HNTop
	err = json.NewDecoder(r).Decode(&hnid)
	if err != nil {
		fmt.Fprintf(os.Stderr, "decode: %v\n", err)
		hdim.gerror(d.bgcolor, d.textcolor, "no headlines")
		r.Close()
		return
	}
	r.Close()

	var item HNitem
	x := d.width / 2
	y := d.height * 0.10
	headsize := d.width / 100
	spacing := headsize * 2.0
	hdim.regionFill(d.bgcolor, d.textcolor)
	for i := n - 1; i >= 0; i-- {
		hnr, err := netread(fmt.Sprintf(HNItemfmt, hnid[i]))
		if err != nil {
			fmt.Fprintf(os.Stderr, "%v: getting id %d\n", err, hnid[i])
			hnr.Close()
			continue
		}
		err = json.NewDecoder(hnr).Decode(&item)
		if err != nil {
			fmt.Fprintf(os.Stderr, "%v: decoding id %d\n", err, hnid[i])
			hnr.Close()
			continue
		}
		openvg.TextMid(x, y, item.Title, "sans", int(headsize))
		y += spacing
		hnr.Close()
	}
	openvg.Image(d.width*0.05, 15, 32, 32, "hn.png")
	openvg.End()
}

// headlines retrieves data from the New York Times API, decodes and displays it.
func (d *display) nytheadlines(section string) {
	hdim := dimen{x: 0, y: 0, width: d.width, height: d.height / 2}
	r, err := netread(fmt.Sprintf(NYTfmt, section, NYTAPIkey))
	if err != nil {
		fmt.Fprintf(os.Stderr, "headline read error: %v\n", err)
		hdim.gerror(d.bgcolor, d.textcolor, "no headlines")
		return
	}
	defer r.Close()
	var data NYTHeadlines
	err = json.NewDecoder(r).Decode(&data)
	if err != nil {
		fmt.Fprintf(os.Stderr, "decode: %v\n", err)
		hdim.gerror(d.bgcolor, d.textcolor, "no headlines")
		return
	}
	x := d.width / 2
	y := d.height * 0.10
	hdim.regionFill(d.bgcolor, d.textcolor)
	headsize := d.width / 100
	spacing := headsize * 2.0
	for i := len(data.Results) - 1; i >= 0; i-- {
		openvg.TextMid(x, y, fromHTML.Replace(data.Results[i].Title), "sans", int(headsize))
		y = y + spacing
	}
	openvg.Image(d.width*0.05, 15, 30, 30, "poweredby_nytimes_30a.png")
	openvg.End()
}

// netread derefernces a URL, returning the Reader, with an error
func netread(url string) (io.ReadCloser, error) {
	client := &http.Client{Timeout: 30 * time.Second}
	resp, err := client.Get(url)
	if err != nil {
		return nil, err
	}
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("unable to get network data for %s (%s)", url, resp.Status)
	}
	return resp.Body, nil
}

// countdown shows a countdown display to the top of minute
func (d *display) countdown() {
	tick := time.NewTicker(1 * time.Second)
	ty := d.height / 2
	th := d.height / 20
	size := d.width / 70
	for delay := 60 - time.Now().Second(); delay > 0; delay-- {
		select {
		case <-tick.C:
			tx := d.width * (openvg.VGfloat(60-delay) / 60)
			openvg.BackgroundColor(d.bgcolor)
			openvg.FillColor("black")
			openvg.Rect(0, ty, d.width, th)
			openvg.FillColor("white")
			openvg.TextEnd(tx, ty+(th/4), fmt.Sprintf("start in %d ", delay), "sans", int(size))
			openvg.Rect(tx, ty, d.width-tx, th)
			openvg.End()
		}
	}
	openvg.BackgroundColor(d.bgcolor)
}

// regionFill colors a rectangular region, and sets the fill color for subsequent text
func (d *dimen) regionFill(bgcolor, textcolor string) {
	openvg.FillColor(bgcolor)
	openvg.Rect(d.x, d.y, d.width, d.height)
	openvg.FillColor(textcolor)
}

// gerror makes a graphical error message
func (d *dimen) gerror(bgcolor, textcolor, message string) {
	d.regionFill(bgcolor, textcolor)
	openvg.TextMid(d.x+d.width/2, d.y+d.height/2, message, "sans", int(d.width/20))
	openvg.End()
}

// fog shows the fog icon
func (d *dimen) fog(color string) {
	x, y, w, h := d.x, d.y, d.width, d.height
	radius := d.width / 3
	r2 := radius * 1.8
	openvg.FillColor(color, 0.5)
	openvg.Circle(x+w*0.25, y+h*0.25, radius)
	openvg.Circle(x+w*0.30, y+h*0.45, radius)
	openvg.Circle(x+w*0.60, y+h*0.40, r2)
}

// cloud shows the cloudy icon
func (d *dimen) cloud(color string) {
	x, y, w, h := d.x, d.y, d.width, d.height
	radius := d.width / 3
	r2 := radius * 1.8
	openvg.FillColor(color)
	openvg.Circle(x+w*0.25, y+h*0.25, radius)
	openvg.Circle(x+w*0.30, y+h*0.45, radius)
	openvg.Circle(x+w*0.60, y+h*0.40, r2)
}

// drop shows the raindrop icon
func (d *dimen) drop(color string) {
	x, y, w, h := d.x, d.y, d.width, d.height
	openvg.FillColor(color)
	openvg.Ellipse(x+(w/2), y+(h*0.40), w*0.52, h*0.65)
	xp := []openvg.VGfloat{x + (w / 2), x + (w * 0.25), x + (w * 0.75)}
	yp := []openvg.VGfloat{y + h, y + (h / 2), y + (h / 2)}
	openvg.Polygon(xp, yp)
}

// rain shows raindrops
func (d *dimen) rain(color string) {
	dd := dimen{x: 0, y: 0, width: d.width / 6, height: d.height / 6}
	for i := 0; i < 20; i++ {
		dd.x = d.x + d.width*openvg.VGfloat(rand.Float64())
		dd.y = d.y + d.height*openvg.VGfloat(rand.Float64())
		dd.drop(color)
	}
}

// flake shows the snowflake icon
func (d *dimen) flake(color string) {
	x, y, w, h := d.x, d.y, d.width, d.height
	cx := x + (w / 2)
	cy := y + (h / 2)
	r := w * 0.30
	openvg.StrokeColor(color)
	openvg.StrokeWidth(w / 20)
	for t := 0.0; t < 2*math.Pi; t += math.Pi / 4 {
		c := openvg.VGfloat(math.Cos(t))
		s := openvg.VGfloat(math.Sin(t))
		x1 := (r * c) + cx
		y1 := (r * s) + cy
		openvg.Line(cx, cy, x1, y1)
	}
	openvg.StrokeWidth(0)
}

// snow shows the snow icon
func (d *dimen) snow(color string) {
	df := dimen{x: 0, y: 0, width: d.width / 6, height: d.height / 6}
	for i := 0; i < 20; i++ {
		df.x = d.x + d.width*openvg.VGfloat(rand.Float64())
		df.y = d.y + d.height*openvg.VGfloat(rand.Float64())
		df.flake(color)
	}
}

// sun shows the icon for clear weather
func (d *dimen) sun(color string) {
	x, y, w, h := d.x, d.y, d.width, d.height
	cx := x + (w / 2)
	cy := y + (h / 2)
	r0 := w * 0.50
	r1 := w * 0.45
	r2 := w * 0.30
	openvg.FillColor(color)
	openvg.Circle(cx, cy, r0)
	openvg.StrokeColor(color)
	openvg.StrokeWidth(w / 30)
	for t := 0.0; t < 2*math.Pi; t += math.Pi / 6 {
		c := openvg.VGfloat(math.Cos(t))
		s := openvg.VGfloat(math.Sin(t))
		x1 := (r1 * c) + cx
		y1 := (r1 * s) + cy
		x2 := (r2 * c) + cx
		y2 := (r2 * s) + cy
		openvg.Line(x1, y1, x2, y2)
	}
	openvg.StrokeWidth(0)
}

// moon shows the icon for clear weather at night
func (d *dimen) moon(bg, fg string) {
	x, y, w, h := d.x, d.y, d.width, d.height
	cx := x + w/2
	cy := y + h/2
	w2 := w / 2
	openvg.FillColor(fg)
	openvg.Circle(cx, cy, w2)
	openvg.FillColor(bg)
	openvg.Circle(x+w*0.65, cy, w2)
}

// pcloud shows the icon for partly cloudy
func (d *dimen) pcloud(color string) {
	sd := dimen{x: d.x + d.width*.2, y: d.y + d.height*.33, width: d.width * .7, height: d.height * .7}
	sd.sun("orange")
	d.cloud(color)
}

// npcloud shows the partly cloudy icon at night
func (d *dimen) npcloud(ccolor, mcolor string) {
	d.cloud(ccolor)
	md := dimen{x: d.x + d.width*.2, y: d.y + d.height*0.05, width: d.width * .7, height: d.height * .7}
	md.moon(ccolor, mcolor)
}

// wind shows the windy icon
func (d *dimen) wind(bg, color string) {
	x, y, w, h := d.x, d.y, d.width, d.height
	openvg.FillColor(bg, 0)
	openvg.StrokeWidth(w / 25)
	openvg.StrokeColor(color)
	openvg.Qbezier(x+w*0.10, y+h*0.8, x+w*0.50, y+h*0.60, x+w*0.9, y+h*0.85)
	openvg.Qbezier(x+w*0.10, y+h*0.5, x+w*0.55, y+h*0.30, x+w*0.9, y+h*0.55)
	openvg.Qbezier(x+w*0.10, y+h*0.2, x+w*0.60, y+h*0.10, x+w*0.9, y+h*0.35)
	openvg.StrokeWidth(0)
}

// daycolor returns a color appropriate for the hour of the day
func daycolor() string {
	hour := time.Now().Hour()
	switch {
	case hour <= 11 && hour >= 6:
		return "steelblue"
	case hour <= 17 && hour >= 12:
		return "blue"
	case hour <= 19 && hour >= 18:
		return "slategray"
	default:
		return "midnightblue"
	}
}
