### The main goal of this project is to create a display for my desk that shows that current day's comic. As of right now this is based on the [5.65inch ACeP 7-Color E-Paper from waveshare](https://www.waveshare.com/product/displays/e-paper/epaper-1/5.65inch-e-paper-module-f.htm) and an Raspberry Pi Zero 2 W

The main idea is that the RPI will web scrape for the comic strip image and then convert it from GIF to BMP format. Then, with the newly formated image it will be displayed. Thank you Robert W. Floyd and Louis Steinberg for creating the first image [dithering algorithm](https://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering)


## Setting up the [Raspbeery Pi Zero 2 W](https://www.raspberrypi.com/products/raspberry-pi-zero-2-w/)

I am still figuring out which steps I would recommend to start with first but this is an alternative to doing all of this via command line. This is a [link](https://youtu.be/yVW8LftDSwg?t=209) to a youtube video titled "C Programming on a Headless Raspberry Pi Part 1: VSCode Server Setup (Pi Zero 2 W)" by "Learn Embedded Systems" and they go through a great way to connect the to RPI and still have a gui. KEEP IN MIND WE DO NOT CARE ABOUT COMPILING OR RUNNING ANY C PROGRAMS THAT ARE DISCUSSED IN THE VIDEO. ONLY DO THE VSCODE RPI INTEGRATION. After you are done your VSCode should look a little something like the photo below. Import areas are folder directory on the left and the terminal at the bottom.

![VSCode.jpg](Docs/VSCodeMainScreen.jpg)

Another thing to note is how you plan on getting the software on the RPI. I used github to download and do my verison control. Below is the command to install git on the RPI.
```sh
sudo apt install git
```
This will download the repo to your RPI.
```sh
git clone https://github.com/jowdow/DeliveryArmCapstone.git
```

Wave Share gave an amazing tutorial on how to set up the RPI. Here is the [link](https://www.waveshare.com/wiki/5.65inch_e-Paper_Module_(F)_Manual#Working_With_Raspberry_Pi), If you follow it (minus one instruction) you should be able to get up and going pretty fast. The only issue I ran into was getting an error after running the command "sudo apt-get install wiringpi". Just skip the command and follow the instructions in the pink box in the image below.

![WiringPi.jpg](Docs/WiringPi.jpg)


Below are the command and an example of how to commit and push your changes to your fork of the repo.
```sh
git commit -a
```
```sh
git push
```
![commandsforcommiting.jpg](Docs/commandsforcommiting.jpg)

## Current Bugs / Notes
This is very much so incomplete code and NOT up to my standard. I will be updating it as I get the time. I currently have a bug in the code however it works mostly well.

![CurrentBug.jpg](Docs/CurrentBugMinuteZeroToMinuteTen.jpg)

## Case
I have two cases I made. One is for display on wall on my desk for others to see as well and one is for me and will sit beneath my monitor. Below is the wall mounted version and what the internals kind of look like. I WILL BE REMOVING THE TAPE IT IS TEMPORARY.
![Back.jpg](Docs/Back.jpg)
![FrontImage.jpg](Docs/FrontImage.jpg)
![BackImage.jpg](Docs/BackImage.jpg)
