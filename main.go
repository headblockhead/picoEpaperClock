package main

import (
	"image"
	"machine"

	"image/color"

	"golang.org/x/image/font"
	"golang.org/x/image/font/basicfont"
	"golang.org/x/image/math/fixed"
	"tinygo.org/x/drivers/waveshare-epd/epd2in13x"
)

var display epd2in13x.Device

func main() {
	machine.SPI1.Configure(machine.SPIConfig{
		Frequency: 8000000,
		Mode:      0,
	})

	display = epd2in13x.New(machine.SPI1, machine.GPIO9, machine.GPIO8, machine.GPIO12, machine.GPIO13)
	display.Configure(epd2in13x.Config{Width: 104, Height: 212, NumColors: 3})

	display.ClearBuffer()
	display.ClearDisplay()

	colored := color.RGBA{255, 0, 0, 255}
	black := color.RGBA{1, 1, 1, 255}

	// Show the time on the display using text
	img := image.NewRGBA(image.Rect(0, 0, 104, 212))
	drawText(img, 0, 13, "TinyGo Clock", black)
	drawHLineCol(img, 0, 15, img.Rect.Dx(), colored)

	drawImage(&display, img)
	display.Display()
	display.WaitUntilIdle()
	println("You could remove power now")
}

// drawText will write text in a 7x13 pixel font at a location.
func drawText(img *image.RGBA, x, y int, text string, col color.Color) {
	point := fixed.Point26_6{X: fixed.I(x), Y: fixed.I(y)}

	d := &font.Drawer{
		Dst:  img,
		Src:  image.NewUniform(col),
		Face: basicfont.Face7x13,
		Dot:  point,
	}
	d.DrawString(text)
}

func drawImage(display *epd2in13x.Device, img *image.RGBA) {
	for x := int16(0); x < int16(img.Rect.Dx()); x++ {
		for y := int16(0); y < int16(img.Rect.Dy()); y++ {
			display.SetPixel(x, y, img.RGBAAt(int(x), int(y)))
		}
	}
}

// drawHLineCol draws a horizontal line in a color of your choice from one X location to another. x2 has to be greater than x1.
func drawHLineCol(img *image.RGBA, x1 int, y int, x2 int, col color.RGBA) {
	for ; x1 <= x2; x1++ {
		img.Set(x1, y, col)
	}
}
