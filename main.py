import serial
import serial.rs485
from PIL import Image, ImageDraw, ImageFont
import PIL
import os

BLACK = 0x000000
WHITE = 0xFFFFFF
GREEN = 0x00FF00
BLUE = 0xFF0000
RED = 0x0000FF
YELLOW = 0x00FFFF
ORANGE = 0x0080FF


def main():
    font18 = ImageFont.truetype("Font.ttc", 18)
    font24 = ImageFont.truetype("Font.ttc", 24)
    font40 = ImageFont.truetype("Font.ttc", 40)
    font70 = ImageFont.truetype("Font.ttc", 70)

    Himage = Image.new('RGB', (600, 448), WHITE)
    draw = ImageDraw.Draw(Himage)
    # draw.rectangle((0, 0, 600, 80), fill=RED)
    # draw.pieslice([(0, 0), (30, 30)], 0, 90, fill=None, outline=BLACK, width=4)
    Y = 0
    X = 300

    drawTemp(draw, 0, 0, "", "C", str(10), font18)
    drawTemp(draw, 50, 0, "", "A", str(25), font18)
    drawMotor(draw, 160, 0, "", "26", str(1250), font18)
    drawTemp(draw, 230, 0, "", "14", str(30), font18)
    drawMotor(draw, 320, 0, "", "29", str(9999), font18)


    Himage.show()


def drawMotor(draw, X, Y, name, address, value, font18):
    draw.ellipse([(30 + X, 30 + Y), (50 + X, 50 + Y)], fill=GREEN)  # Inside green
    draw.ellipse([(27 + X, 27 + Y), (53 + X, 53 + Y)], outline=BLACK, width=4)  # Outside blackring
    draw.ellipse([(39 + X, 39 + Y), (41 + X, 41 + Y)], fill=BLACK)  # Black dot in middle

    draw.ellipse([(27 + X, 28 + Y), (31 + X, 32 + Y)], fill=BLACK)  # Circle Idk which one
    draw.ellipse([(27 + X, 49 + Y), (31 + X, 53 + Y)], fill=BLACK)  # Circle Idk which one
    draw.ellipse([(49 + X, 28 + Y), (53 + X, 32 + Y)], fill=BLACK)  # Circle Idk which one
    draw.ellipse([(49 + X, 49 + Y), (53 + X, 53 + Y)], fill=BLACK)  # Circle Idk which one

    draw.text([16 + X, 0 + Y], name + "(0x" + address + ")", font=font18, fill=BLACK)  # Name
    draw.text([10 + X, 70 + Y], value + "RPM", font=font18, fill=BLACK)  # Value


def drawTemp(draw, X, Y, name, address, tempValue, font18):
    draw.line([(55 + X, 53 + Y), (55 + X, 30 + Y)], fill=BLACK, width=3)  # Outside
    draw.line([(65 + X, 53 + Y), (65 + X, 30 + Y)], fill=BLACK, width=3)  # Outside
    draw.arc([(50 + X, 50 + Y), (70 + X, 70 + Y)], -50, 235, fill=BLACK, width=3)  # Outside over halfish circle
    draw.arc([(53 + X, 25 + Y), (68 + X, 46 + Y)], -160, -40, fill=BLACK, width=3)  # Outside top arc

    draw.line([(60 + X, 58 + Y), (60 + X, 31 + Y)], fill=RED, width=3)  # Inside red line
    draw.arc([(56 + X, 54 + Y), (64 + X, 65 + Y)], -200, 235, fill=RED, width=8)  # Inside red circle

    draw.text([40 + X, 0 + Y], name + "(0x" + address + ")", font=font18, fill=BLACK)  # Name
    draw.text([45 + X, 70 + Y], tempValue + "C", font=font18, fill=BLACK)  # Value
    return draw


if __name__ == '__main__':
    main()
