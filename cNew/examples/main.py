import datetime

import PIL
import requests
from bs4 import BeautifulSoup
import urllib.request
from PIL import Image
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
            #[(0, 0, 0), (255, 255, 255)])
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
            if height > 601 or width > 449: # sometimes the image is too big after resizing and this grabs and error image to fix that
                with Image.open("/pic/error.bmp") as newIm:
                    Image.fromarray(np.array(newIm)).save(title+ ".bmp")
                    return
            print("W "+str(height)+" H "+ str(width))
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
        Image.fromarray(newbmp).save("/home/pi/Desktop/cNew/"+title+ ".bmp")

    except FileNotFoundError:
        file_error(title + ".gif")

def getAndSaveToFileWeatherData():
    htmldata = getdata("https://forecast.weather.gov/MapClick.php?x=304&y=258&site=iwx&zmx=&zmy=&map_x=304&map_y=258")
    soup = BeautifulSoup(htmldata, 'html.parser')
    
    
    weatherData = []
    weatherBox = soup.find_all('div', class_="tombstone-container")
    count = 0
    for dayInfo in weatherBox:
        tempArray = []
        if dayInfo.find(class_="period-name").get_text().find("NOW") != -1: # Sometimes there are warnings and I don't care about them so this skips those
            continue
    
        if count % 2 == 0:
            temp = dayInfo.find(class_="period-name").get_text()
            temp = temp.replace("ThisAfternoon", "Today")
            temp = temp.replace("Night", "")
            temp = temp.replace("Day", "")
            tempArray.append(temp)
    
            temp = dayInfo.find(class_="short-desc").get_text()
            temp = temp.replace("Chance", "")
            tempArray.append(temp)
    
            temp = dayInfo.find(class_="temp").get_text()
            temp=temp.replace("High: ", "")
            temp=temp.replace("Low: ", "")
            temp=temp.replace(" °F", "")
            tempArray.append(temp)
    
            weatherData.append(tempArray)
        else:
            temp = dayInfo.find(class_="temp").get_text()
            temp=temp.replace("High: ","")
            temp=temp.replace("Low: ", "")
            temp=temp.replace(" °F", "")
            #print(temps)
            weatherData[len(weatherData)-1][2] += "|" + temp
        # temp.replace("This","")
        # temp.replace("Night", "")
        # print(dayInfo)
        if count == 0 and tempArray[0] == 'Tonight':
            weatherData[0][2] = "?|"+weatherData[0][2]
            count = 0
        else:
            count += 1
    
    
    
    file = open('/home/pi/Desktop/cNew/weather.txt','w')
    
    # JD - This makes sure that there is a space before each capital letter
    # count = 0
    # countMessage = 0
    # tempMessage = ""
    # isNoSpacePresentBefore = 0
    # for x in range(2):
    #     tempMessage = ""
    #     for char in weatherData[x][2]:
    #         if char.isupper() and count != 0 and isNoSpacePresentBefore:
    #             tempMessage += " " + char
    #         else:
    #             if char == ' ':
    #                 isNoSpacePresentBefore = 0
    #             else:
    #                 isNoSpacePresentBefore = 1
    #
    #             tempMessage += char
    #         count += 1
    #     weatherData[x][2] = tempMessage
    #     countMessage += 1
    for x in range(len(weatherData)):
        file.write(",".join(weatherData[x]))
        file.write('\n')
    for x in weatherData:
        print(x)
    print("")
    
    file.close()


def main():
    print("Starting weather parsing")
    getAndSaveToFileWeatherData()
    print("Done with weather")
    print("\n")
    print("Starting Comic Downloads")
    
    with open('/home/pi/Desktop/cNew/cartoons.txt') as f:
        cartoon = f.readlines()
    # Getting current time
    current_time = datetime.datetime.now()
    dateArr = [str(current_time.year), str(current_time.month), str(current_time.day)]
    print(dateArr[0]+"-"+dateArr[1]+"-"+dateArr[2])
    
    inputPath = str(dateArr[0] + "-" + dateArr[1] + "-" + dateArr[2])
    for title in cartoon:
        title = title.replace('\n','')
        if not find_file((inputPath+title + ".bmp")):
            print("Going to: "+"https://www.gocomics.com/"+title+"/" + dateArr[0] + "/" + dateArr[1] + "/" + dateArr[2])
            htmldata = getdata("https://www.gocomics.com/"+title+"/" + dateArr[0] + "/" + dateArr[1] + "/" + dateArr[2])
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
                print("Found image link: "+results[0])
                try:
                    urllib.request.urlretrieve(results[0], (inputPath + title + ".gif"))
                except IndexError:
                    print("Image isn't present. Likely comic hasn't been uploaded yet")
                    
                print("Downloaded Image")
                print("Dithering Image")

                dither(inputPath + title)

        else:
            print(inputPath+" "+title+" already exists"+"      " )
        
    print("\n")
    

if __name__ == '__main__':
    main()
