import datetime

import PIL
import requests
from bs4 import BeautifulSoup
import urllib.request
from PIL import Image, ImageDraw, ImageFont
import numpy as np
import os


def getdata(url):
    try:
        r = requests.get(url)
        return r.text
    except:
        return "error"


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


# some handy functions

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
        print("Done Dithering")
        Image.fromarray(newbmp).save(os.getcwd() + "/" + title + ".bmp")

    except FileNotFoundError:
        file_error(title + ".gif")


def getAndSaveToFileWeatherData():
    API_KEY = ''
    CITY = 'New York'
    NUM_DAYS = 7

    # Make the API request to get the weather data
    response = requests.get(f'https://api.openweathermap.org/data/2.5/forecast?q={CITY}&appid={API_KEY}&units=imperial')
    data = response.json()

    # Get the start date and time for the forecast
    start_date = datetime.datetime.strptime(data['list'][0]['dt_txt'], '%Y-%m-%d %H:%M:%S')

    #
    date = ""
    lowTemp = 0.0
    highTemp = 0.0
    description = ""
    count = 0
    file = open(os.getcwd() + "/weather.txt", 'w')

    # Initialize a dictionary to store high, low, and description for each day
    daily_temperatures = {}

    # Loop over the forecast data and extract the high, low, and description for each day
    for item in data['list']:
        # Get the date and time for this forecast item
        date_time = datetime.datetime.strptime(item['dt_txt'], '%Y-%m-%d %H:%M:%S')

        # If this item is for a new day, add a new entry to the daily_temperatures dictionary
        if date_time.date() not in daily_temperatures:
            daily_temperatures[date_time.date()] = {
                'high': item['main']['temp_max'],
                'low': item['main']['temp_min'],
                'des': item['weather'][0]['description']
            }
        # Otherwise, update the high, low, and description if necessary
        else:
            if item['main']['temp_max'] > daily_temperatures[date_time.date()]['high']:
                daily_temperatures[date_time.date()]['high'] = item['main']['temp_max']
            if item['main']['temp_min'] < daily_temperatures[date_time.date()]['low']:
                daily_temperatures[date_time.date()]['low'] = item['main']['temp_min']
            if item['weather'][0]['description'] != daily_temperatures[date_time.date()]['des']:
                daily_temperatures[date_time.date()]['des'] = item['weather'][0]['description']

        # Stop looping if we have enough days of data
        if len(daily_temperatures) >= NUM_DAYS:
            break

    # Print and save the high, low, and description for each day
    for date, temps in daily_temperatures.items():
        file.write(date.strftime("%A"))
        file.write(",")
        file.write(temps["des"])
        file.write(",")
        file.write(str(round(temps["low"])))
        file.write("|")
        file.write(str(round(temps["high"])))
        file.write('\n')
        print(f'{date.strftime("%A")}: High {temps["high"]:.1f}°F, Low {temps["low"]:.1f}°F , {temps["des"]}')
    file.close()


def drawIcon(x, y, description,bmp_image):
    if "Showers" or "Rain" in description:
        img2 = Image.open('rain_icon.bmp')
        bmp_image.paste(img2, (x, y))
    elif "Flurries" in description:
        img2 = Image.open('snowflake_icon.bmp')
        bmp_image.paste(img2, (x, y))
    elif "Clouds" in description:
        img2 = Image.open('cloudy_icon.bmp')
        bmp_image.paste(img2, (x, y))
    elif "Clear" in description:
        img2 = Image.open('sun_icon.bmp')
        bmp_image.paste(img2, (x, y))
    elif "Thunderstorms" or "T-storms" or "Storms" in description:
        img2 = Image.open('thunder_icon.bmp')
        bmp_image.paste(img2, (x, y))


def saveWeatherDataToImage(title):
    # Create a new BMP image with the size of 600 by 448
    bmp_image = Image.new('RGB', (600, 448), (255, 255, 255))

    # Open the first image you want to insert
    img1 = Image.open(title)
    comicWidth, comicHeight = img1.size
    
    # Paste the first image at the top left corner of the BMP image
    bmp_image.paste(img1, (0, 0))

    lines = []
    # open the file
    with open('weather.txt', 'r') as f:
        # read all the lines in the file
        lines = f.readlines()

    # create an empty list to store the data
    data = []

    # loop over each line and split it by comma
    for line in lines:
        # remove the newline character from the end of the line
        line = line.strip()
        # split the line by comma
        parts = line.split(',')
        data.append(parts)
    adjustment = 0
    if comicHeight <350: # If the comic is small enough to also have the weather 
        if comicHeight > 190: # If the comic is larger than where we default show weather they need to be moved some
            adjustment = comicHeight-190
        draw = ImageDraw.Draw(bmp_image)  # init draw

        # This is here when i found out 
        #font_path = os.getcwd() +"/arial.ttf"
        #font_size = 20
        #font = ImageFont.truetype(font_path, font_size)

        # Time/Date START
        now = datetime.datetime.now()
        '''draw.text((410, 410), "Time: ", font= font ,fill="black")
        draw.text((445, 410), now.strftime("%H:%M:%S"), font= font ,fill="black")
        draw.text((410, 432), "Date:",font= font , fill="black")
        draw.text((445, 432), now.strftime("%Y-%m-%d"),font= font , fill="black")'''
        draw.text((410, 410), "Time: " ,fill="black")
        draw.text((445, 410), now.strftime("%H:%M:%S") ,fill="black")
        draw.text((410, 432), "Date:" , fill="black")
        draw.text((445, 432), now.strftime("%Y-%m-%d") , fill="black")
        # Time/Date END

        # DAY 1 START
        draw.text((10, 240+adjustment), data[0][0], fill="black")
        draw.text((10, 260+adjustment), data[0][1], fill="black")
        draw.text((75, 240+adjustment), data[0][2], fill="black")
        drawIcon(30, 200+adjustment, data[0][1],bmp_image)
        # DAY 1 END

        # DAY 2 START
        draw.text((125, 240+adjustment), data[1][0], fill="black")
        draw.text((125, 260+adjustment), data[1][1], fill="black")
        draw.text((190, 240+adjustment), data[1][2], fill="black")
        drawIcon(145, 200+adjustment,data[1][1],bmp_image)
        # DAY 2 END

        # DAY 3 START
        draw.text((240, 240+adjustment), data[2][0], fill="black")
        draw.text((240, 260+adjustment), data[2][1], fill="black")
        draw.text((305, 240+adjustment), data[2][2], fill="black")
        drawIcon(260, 200+adjustment, data[2][1],bmp_image)
        # DAY 3 END

        # DAY 4 START
        draw.text((355, 240+adjustment), data[3][0], fill="black")
        draw.text((355, 260+adjustment), data[3][1], fill="black")
        draw.text((430, 240+adjustment), data[3][2], fill="black")
        drawIcon(375, 200+adjustment, data[3][1],bmp_image)
        # DAY 4 END

        # DAY 5 START
        draw.text((480, 240+adjustment), data[4][0], fill="black")
        draw.text((480, 260+adjustment), data[4][1], fill="black")
        draw.text((545, 240+adjustment), data[4][2], fill="black")
        drawIcon(500, 200+adjustment, data[4][1],bmp_image)
        # DAY 5 END

    # The below code is correcting the image so it only has black or white pixels. It can not do color, I would need to add the "getclosest" function
    width, height = bmp_image.size
    array = [(0, 0, 0), (0, 0, 255), (255, 0, 0), (0, 255, 0), (255, 128, 0), (255, 255, 0), (255, 255, 255)]
    # Loop through each pixel in the image
    x=0
    y=240
    for x in range(width):
        for y in range(height):
            # Get the color value of the pixel at (x, y)
            color = bmp_image.getpixel((x, y))
            # print(f"Pixel at ({x}, {y}): {color}")
            if color not in array:
                print(f"Pixel at ({x}, {y}): {color}")
                if color[0] < 174:
                    bmp_image.putpixel((x, y), (0,0,0))
                else:
                    bmp_image.putpixel((x, y), (255,255,255))
            else:
                bmp_image.putpixel((x, y), color)


    # Save the modified BMP image
    bmp_image.save(os.getcwd() + "/" + title)


def main():
    print("Starting weather parsing")
    getAndSaveToFileWeatherData()
    print("Done with weather")
    print("\n")
    print("Starting Comic Downloads")

    with open(os.getcwd() + "/cartoons.txt") as f:
        cartoon = f.readlines()
    # Getting current time
    current_time = datetime.datetime.now()
    dateArr = [str(current_time.year), str(current_time.month), str(current_time.day)]
    print(dateArr[0] + "-" + dateArr[1] + "-" + dateArr[2])

    inputPath = str(dateArr[0] + "-" + dateArr[1] + "-" + dateArr[2])
    for title in cartoon:
        title = title.replace('\n', '')
        if not find_file((inputPath + title + ".bmp")):
            print("Going to: " + "https://www.gocomics.com/" + title + "/" + dateArr[0] + "/" + dateArr[1] + "/" +
                  dateArr[2])
            htmldata = getdata(
                "https://www.gocomics.com/" + title + "/" + dateArr[0] + "/" + dateArr[1] + "/" + dateArr[2])
            if htmldata == "error":
                print("Error likely do to no connection")
                continue
            else:
                # htmldata = getdata("https://www.gocomics.com/garfield/2022/07/20")
                soup = BeautifulSoup(htmldata, 'html.parser')
                imgContainer = soup.find_all("picture", class_='item-comic-image')
                imgPlace = soup.findAll('img')
                soup.select('src')
                results = []
                for a in soup.find_all(attrs={'class': 'item-comic-image'}):
                    name = a.find("img")
                    results.append(name.get("src"))
                try:
                    print("Found image link: " + results[0])
                    urllib.request.urlretrieve(results[0], (inputPath + title + ".gif"))
                    print("Downloaded Image")
                    print("Dithering Image")

                    dither(inputPath + title)
                    saveWeatherDataToImage(inputPath + title + ".bmp")
                except IndexError:
                    print("Image isn't present. Likely comic hasn't been uploaded yet or the comic can't be found")


        else:
            print(inputPath + " " + title + " already exists" + "      ")

    print("\n")


if __name__ == '__main__':
    main()
