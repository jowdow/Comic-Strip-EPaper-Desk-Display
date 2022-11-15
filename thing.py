#!/usr/bin/python
# -*- coding:utf-8 -*-
import sys
import os

devOnRPI = False
exadir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'examples')
picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)

import logging

if devOnRPI:
    True
    # from waveshare_epd import epd5in65f
import time
from PIL import Image, ImageDraw, ImageFont
import traceback
import datetime
import PIL
import requests
from bs4 import BeautifulSoup
import urllib.request
import numpy as np
import os

logging.basicConfig(level=logging.DEBUG)

# List of available color schemes
defaultschemes = {
    # If doindendent == True, treat the RGB values as independent, matching each independently.
    # In this case, color scheme uses a list of numbers from 0 - 255 called "shades" that represent
    # the shade of each color
    # If doindependent == False, match against actual RGB colors. In this case, the color scheme is a
    # list of RGB colors
    "rgb": {  #
        "doindependent": False,
        "colors": np.array(
            [(0, 0, 0), (0, 0, 255), (255, 0, 0), (0, 255, 0), (255, 128, 0), (255, 255, 0), (255, 255, 255)])
        # [(0, 0, 0), (255, 255, 255)])
    }
}


def getdata(url):
    r = requests.get(url)
    return r.text


# src="https://assets.amuniversal.com/

def floyd_steinberg(image):
    # image: np.array of shape (height, width), dtype=float, 0.0-1.0
    # works in-place!
    h, w = image.shape
    for y in range(h):
        for x in range(w):
            old = image[y, x]
            new = np.round(old)
            image[y, x] = new
            error = old - new
            # precomputing the constants helps
            if x + 1 < w:
                image[y, x + 1] += error * 0.4375  # right, 7 / 16
            if (y + 1 < h) and (x + 1 < w):
                image[y + 1, x + 1] += error * 0.0625  # right, down, 1 / 16
            if y + 1 < h:
                image[y + 1, x] += error * 0.3125  # down, 5 / 16
            if (x - 1 >= 0) and (y + 1 < h):
                image[y + 1, x - 1] += error * 0.1875  # left, down, 3 / 16
    return image


def file_error(inputpath):
    print("Error: File", inputpath, "does not exist.")
    exit(1)


# returns whether an item is in a list (because the "in" keyword was giving me difficulty)
# works specifically for a list of numpy elements
def isin(item, list):
    for listitem in list:
        if (np.array_equal(item, listitem)):
            return True
    return False


# returns the closest point in a given color scheme
# this function is the bottleneck of the whole program
def getclosest(color, colorscheme):
    if colorscheme["doindependent"]:
        bestcolor = np.array([0, 0, 0], dtype=np.uint8)
        for i in range(3):
            bestdist = float("inf")
            bestshade = None
            for newshade in colorscheme["shades"]:
                dist = abs(newshade - color[i])
                if (dist < bestdist):
                    bestdist = dist
                    bestshade = newshade
            bestcolor[i] = bestshade
        return bestcolor
    else:
        bestdist = float("inf")
        bestcolor = None
        for newcolor in colorscheme["colors"]:
            dist = np.sum(np.square(newcolor - color))
            if (dist < bestdist):
                bestdist = dist
                bestcolor = newcolor
        return bestcolor


# exactly like getclosest, but also handles excluded colors
def getclosestwithexclusion(color, colorscheme, excludedcolors):
    bestdist = float('inf')
    bestcolor = None
    for newcolor in colorscheme["colors"]:
        if not isin(newcolor, excludedcolors):
            dist = np.sum(np.square(newcolor - color))
            if (dist < bestdist):
                bestdist = dist
                bestcolor = newcolor
    return bestcolor


# takes a colorscheme specified by shades, and returns a colorscheme specified by colors
def makecolors(colorscheme):
    if colorscheme["doindependent"]:
        mycolors = []
        for shadeR in colorscheme["shades"]:
            for shadeG in colorscheme["shades"]:
                for shadeB in colorscheme["shades"]:
                    mycolors.append(np.array([shadeR, shadeG, shadeB]))
        return {
            "doindependent": False,
            "colors": mycolors
        }
    else:
        return colorscheme


def find_file(file_name):
    # When I want to find a file in a secific place replace os.getcwd() with the path in string form
    for path, subdirs, files in os.walk(os.getcwd()):
        for name in files:
            if (file_name == name):
                return True
    return False


def dither(title):
    # actual dithering operations
    try:
        with Image.open(title + ".gif") as im:
            fixed_width = 600
            width_percent = (fixed_width / float(im.size[0]))
            height_size = int((float(im.size[1]) * float(width_percent)))
            im = im.resize((fixed_width, height_size), PIL.Image.NEAREST)
            bmp = np.array(im)  # from PIL Image to Numpy Array, signature (width, height, 3)
            try:
                (width, height, _) = bmp.shape
            except ValueError:
                (width, height) = bmp.shape
            if height > 601 or width > 449:  # sometimes the image is too big after resizing and this grabs and error image to fix that
                with Image.open("/pic/error.bmp") as newIm:
                    Image.fromarray(np.array(newIm)).save(title + ".bmp")
                    return
            print("W " + str(height) + " H " + str(width))
            newbmp = np.empty((width, height, 3), dtype=np.uint8)
            error = np.zeros((width, height, 3))
            for j in range(height):
                for i in range(width):
                    color = bmp[i, j] + error[i, j]
                    newcolor = getclosest(color, defaultschemes["rgb"])
                    newbmp[i, j] = newcolor
                    extracolor = color - newcolor
                    if i + 1 < width:
                        error[i + 1, j] += extracolor * 7 / 16
                    if i - 1 >= 0 and j - 1 >= 0:
                        error[i - 1, j - 1] += extracolor * 3 / 16
                    if j - 1 >= 0:
                        error[i, j - 1] += extracolor * 5 / 16
                    if i + 1 < width and j - 1 >= 0:
                        error[i + 1, j - 1] += extracolor * 1 / 16
        Image.fromarray(newbmp).save(title + ".bmp")

    except FileNotFoundError:
        file_error(title + ".gif")


def findAndDitherCartoons():
    print("Looking for BMP")

    with open('cartoons.txt') as f:  # Opening and reading all contents of the cartoons file
        cartoon = f.readlines()

    current_time = datetime.datetime.now()  # Time is used for the file name storage
    dateArr = [str(current_time.year), str(current_time.month), str(current_time.day)]
    print(dateArr[0] + "-" + dateArr[1] + "-" + dateArr[2])

    inputPath = str(dateArr[0] + "-" + dateArr[1] + "-" + dateArr[2])
    for title in cartoon:
        title = title.replace('\n', '')  # Getting rid of new lines
        if not find_file((inputPath + "_" + title + ".bmp")):  # If the file is not present
            print("Going to: " + "https://www.gocomics.com/" + title + "/" + dateArr[0] + "/" + dateArr[1] + "/" +
                  dateArr[2])
            try:
                htmldata = getdata(
                    "https://www.gocomics.com/" + title + "/" + dateArr[0] + "/" + dateArr[1] + "/" + dateArr[2])
                soup = BeautifulSoup(htmldata, 'html.parser')
                imgContainer = soup.find_all("picture",
                                             class_='item-comic-image')  # IDK why I have this i forget but imma keep it for now
                imgPlace = soup.findAll('img')
                soup.select('src')  # This grabs all of the objects with images ;)
                results = []
                for a in soup.find_all(attrs={'class': 'item-comic-image'}):
                    name = a.find("img")
                    results.append(name.get("src"))
                print("Found image link: " + results[0])
                try:
                    urllib.request.urlretrieve(results[0], (inputPath + "_" + title + ".gif"))
                except IndexError:
                    print("Image isn't present. Likely comic hasn't been uploaded yet")

                print("Downloaded Image")
                print("Dithering Image")

                dither((inputPath + "_" + title))
            except:
                print("Error" + "https://www.gocomics.com/" + title + "/" + dateArr[0] + "/" + dateArr[1] + "/" +
                      dateArr[2])
            # This will do color dither but sucks balls

            # OLD WAY
            #         img = Image.open(inputPath + ".gif")
            #
            #         fixed_width = 600
            #         width_percent = (fixed_width / float(img.size[0]))
            #         height_size = int((float(img.size[1]) * float(width_percent)))
            #         img = img.resize((fixed_width, height_size), PIL.Image.NEAREST)
            #         numpydata = np.array(img)
            #
            #         floyd_steinberg(numpydata)
            #         im = Image.fromarray(numpydata)
            #
            #         im.save(str(dateArr[0] + "-" + dateArr[1] + "-" + dateArr[2]) + ".bmp")
            print("Done with image")

        else:
            print(inputPath + " " + title + " already exists" + "      ")

    print("\n")


def initCartoonVars(currentTime):
    numberOfCartoons = 0
    cartoonFileName = []
    with open("cartoons.txt") as f:  # Opening and reading all contents of the cartoons file
        cartoon = f.readlines()
        for x in cartoon:
            # adding the date + getting rid of new lines + the file type
            cartoonFileName.append(
                str(currentTime.year) + "-" + str(currentTime.month) + "-" + str(currentTime.day) + "_" + x.replace(
                    '\n', '') + ".bmp")
        numberOfCartoons = len(cartoonFileName) - 1

    return numberOfCartoons, cartoonFileName


def main():
    # This is the main loop
    try:
        #epd = epd5in65f.EPD()
        #epd.init()
        #epd.Clear()

        font18 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 18)
        font24 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 24)
        font30 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 40)
        font70 = ImageFont.truetype(os.path.join(picdir, 'Font.ttc'), 70)

        currentTime = datetime.datetime.now()  # Keeping track of da time
        isFirstRun = True
        previousDay = currentTime.day
        startTime = currentTime
        states = ["cartoon", "image", "text"]  # Used to tell which type of item to display
        displayState = "cartoon"
        cartoonIndex = 0
        numberOfCartoons, cartoonFileName = initCartoonVars(currentTime)
        isErrorPresent = True
        while True:

            # If it is a new day and the time is after 3 AM
            if (currentTime.day != previousDay and currentTime.hour > 3):
                previousDay = currentTime.day
                findAndDitherCartoons()
            elif (isFirstRun):
                findAndDitherCartoons()

            if not (currentTime.minute % 2) or isFirstRun:
                if isFirstRun:
                    isFirstRun = False

                #epd.Clear()

                if isErrorPresent:
                    True
                    # ErrorDisplay(epd,font18,font24,font30,font70,["URL Error",str(currentTime.minute)])
                    #updateScreen(epd, font18, font24, font30, font70)

                else:
                    img = Image.open(exadir + "/2022-9-14_bc.bmp")
                    # img.show()
                    # img = Image.open(os.path.join(exadir, '.png'))
                    print("Displaying:" + cartoonFileName[cartoonIndex])
                    #epd.display(epd.getbuffer(img))

                    cartoonIndex = cartoonIndex + 1
                    if numberOfCartoons == cartoonIndex:
                        cartoonIndex = 0

                nextMin = currentTime.minute + 1
                while currentTime.minute != nextMin:  # This while loop is used to hold back the loop until we get to the next minute because it will still be the same minute when we get back up
                    currentTime = datetime.datetime.now()
                isFirstRun = False

            currentTime = datetime.datetime.now()
    except KeyboardInterrupt:
        logging.info("ctrl + c:")
        epd5in65f.sleep()
        exit()


def updateScreen(epd, font18, font24, font30, font70):
    Himage = Image.new('RGB', (epd.width, epd.height), 0xffffff)  # 255: clear the frame
    draw = ImageDraw.Draw(Himage)
    Y = 20
    X = 20
    draw.line([(30 + X, 28 + Y), (30 + X, 5 + Y)], fill=epd.BLACK, width=3)  # Outside
    draw.line([(40 + X, 28 + Y), (40 + X, 5 + Y)], fill=epd.BLACK, width=3)  # Outside
    draw.arc([(25 + X, 25 + Y), (45 + Y, 45 + Y)], -50, 235, fill=epd.BLACK, width=3)  # Outside halfish circle
    draw.arc([(28 + X, 0 + Y), (43 + Y, 21 + Y)], -160, -40, fill=epd.BLACK, width=3)  # Outside top arc

    draw.line([(30 + X, 28 + Y), (30 + X, 5 + Y)], fill=epd.BLACK, width=3)  # Outside

    draw.line([(35 + X, 33 + Y), (35 + X, 6 + Y)], fill=epd.RED, width=3)  # Outside
    draw.arc([(31 + X, 29 + Y), (39 + Y, 40 + Y)], -200, 235, fill=epd.RED, width=8)  # Outside halfish circle

    draw.text((43, 70), '25', font=font18, fill=epd.BLACK)
    draw.text((63, 70), 'C', font=font18, fill=epd.BLACK)
    draw.text((0, 0), 'Air Temp(0x0D)', font=font18, fill=epd.BLACK)
    epd.display(epd.getbuffer(Himage))


def ErrorDisplay(epd, font18, font24, font30, font70, message):
    Himage = Image.new('RGB', (epd.width, epd.height), 0xffffff)  # 255: clear the frame
    draw = ImageDraw.Draw(Himage)
    draw.rectangle((0, 0, 600, 80), fill=epd.RED)
    draw.text((200, 0), 'Error', font=font70, fill=epd.WHITE, outline=1)
    draw.text((10, 90), 'Type:', font=font30, fill=epd.RED)
    draw.text((220, 90), message[0], font=font30, fill=epd.RED)
    draw.text((10, 140), 'Information:', font=font24, fill=epd.RED)
    draw.text((10, 170), message[1], font=font24, fill=epd.RED)
    draw.text((10, 370), u'ä½ çŠ¯äº†ä¸€å€‹åš´é‡çš„éŒ¯èª¤', font=font30, fill=epd.RED)
    epd.display(epd.getbuffer(Himage))


if __name__ == '__main__':
    main()
