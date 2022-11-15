#include <stdlib.h>     //exit()
#include <signal.h>     //signal()
#include "EPD_Test.h"   //Examples
#include "EPD_5in65f.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define RUN 1
#define NO 0
#define LOOP_DELAY 1000
#define TRUE 1
#define FALSE 0
enum State {COMIC = 0, MEME = 1, PICS = 2};


void  Handler(int signo)
{
    //System Exit
    printf("\r\nHandler:exit\r\n");
    DEV_Module_Exit();

    exit(0);
}

int displayComic(char *comic[])
{
    if(DEV_Module_Init()!=0){
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
    
    EPD_5IN65F_Init();
    
    // JD - This is to get the time it takes to clear the image
	struct timespec start={0,0}, finish={0,0}; 
    clock_gettime(CLOCK_REALTIME,&start);
    EPD_5IN65F_Clear(EPD_5IN65F_WHITE);
	clock_gettime(CLOCK_REALTIME,&finish);
    printf("%ld S\r\n",finish.tv_sec-start.tv_sec);
    DEV_Delay_ms(100);


    UBYTE *comicImage;  // JD - This is the actual image data 
    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */  // JD - IDK what this comment is about but I think they are just reusing code
    UDOUBLE Imagesize = ((EPD_5IN65F_WIDTH % 2 == 0)? (EPD_5IN65F_WIDTH / 2 ): (EPD_5IN65F_WIDTH / 2 + 1)) * EPD_5IN65F_HEIGHT;
    if((comicImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for blank memory...\r\n");
        return -1;
    }
    Paint_NewImage(comicImage, EPD_5IN65F_WIDTH, EPD_5IN65F_HEIGHT, 0, EPD_5IN65F_WHITE);
    Paint_SetScale(7);
    
    printf("Showing "); printf("%s",comic); printf("\r\n");
    Paint_Clear(EPD_5IN65F_WHITE);
    //strcat(comic,".bmp");
    GUI_ReadBmp_RGB_7Color(comic, 0, 0);
	EPD_5IN65F_Display(comicImage);  // JD - Actually displaying the image
    DEV_Delay_ms(5000);
    
	printf("e-Paper Sleep...\r\n");
    EPD_5IN65F_Sleep();

    free(comicImage); // JD - Free up memory 
    comicImage = NULL;

    DEV_Module_Exit();
    return 0;
    
}

void displayInit(UBYTE *BlackImage){
    if(DEV_Module_Init()!=0){
        return -1;
    }
    else{
        EPD_5IN65F_Init();
        DEV_Delay_ms(100);

        /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
        UDOUBLE Imagesize = ((EPD_5IN65F_WIDTH % 2 == 0)? (EPD_5IN65F_WIDTH / 2 ): (EPD_5IN65F_WIDTH / 2 + 1)) * EPD_5IN65F_HEIGHT;
        if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
            printf("Failed to apply for blank memory...\r\n");
        }
        else{
            Paint_NewImage(BlackImage, EPD_5IN65F_WIDTH, EPD_5IN65F_HEIGHT, 0, EPD_5IN65F_WHITE);
            Paint_SetScale(7);
        }
    }
}

void delayMilli(int milliseconds){
    clock_t startTime = clock();
    while(clock() < startTime + milliseconds);
}

void delaySec(int seconds){
    int milliseconds = 1000 * seconds;
    clock_t startTime = clock();
    while(clock() < startTime + milliseconds);
}




int main(void)
{
    
    // Exception handling:ctrl + c
    signal(SIGINT, Handler);
    
    UBYTE *BlackImage; // Memory for image
    displayInit(BlackImage);
    
    uint8_t isFirstRun = TRUE;
    uint8_t isNewMinute = FALSE;
    
    char cartoon[6][20] = {{"garfield"},{"pickles"},{"bc"},{"peanuts"},{"dilbert-classics"}}; // JD - Hard coding the cartoon
    
    // JD - Time declaration 
    time_t t= time(NULL);
    struct tm tm = *localtime(&t);
    char year[4];
    char month[2];
    char day[2];
    char hour[2];
    char min[2];
    char currentComic[30];
    char date[30];

    char previousMinute = '0';

    
    int cartoonIndex = 0;
    int cartoonIndexMax = 4;
    
    enum State displayState = COMIC;
    
    // JD - Time intialization
    memset(date,0,sizeof(date));
    t= time(NULL);
    tm = *localtime(&t);
    
    // JD - Initalizing the time just received 
    sprintf( hour,"%d", tm.tm_hour );
    sprintf( min,"%d", tm.tm_min );
    sprintf( year,"%d",( tm.tm_year + 1900 ) );
    sprintf( month,"%d",( tm.tm_mon + 1 ) );
    sprintf( day,"%d", tm.tm_mday );
    
    while(TRUE){
        
        // JD - Grabbing time and memset
        memset(date,0,sizeof(date));
        t= time(NULL);
        tm = *localtime(&t);
        
        // JD - Initalizing the time just received 
        sprintf( hour,"%d", tm.tm_hour );
        sprintf( min,"%d", tm.tm_min );
        sprintf( year,"%d",( tm.tm_year + 1900 ) );
        sprintf( month,"%d",( tm.tm_mon + 1 ) );
        sprintf( day,"%d", tm.tm_mday );
        
        if(min[1] != previousMinute){ // JD - We need to know when it is the next minute
            previousMinute = min[1];
            isNewMinute = TRUE;
        }
        else{
            isNewMinute = FALSE;
        }
        
        if(isFirstRun || (char)hour[1]=='6'){ // JD - Run at start or at 6 am or 4pm(24hour time)
            isFirstRun = FALSE;
            system("python /home/pi/Desktop/cNew/examples/main.py");
        }
        if((char)min[1]=='0' && isNewMinute){ // JD - We display a new image every 10 mins 
            
            if(displayState == COMIC){
                if(cartoonIndex > cartoonIndexMax){ // JD - Resetting index 
                    cartoonIndex = 0;
                }
                
                // JD - Creating the date
				strcat(date, year);
				strcat(date, "-");
				strcat(date, month);
				strcat(date, "-");
				strcat(date, day);
                
                // JD - Printing time and current comic
                memset(currentComic,0,sizeof(currentComic));
                printf("Hour: ");printf(hour);printf("\n");
                printf("Min: ");printf(min);printf("\n");
                printf("date: ");printf(date);printf("\n");
                printf("cartoon: ");printf(cartoon[cartoonIndex]);printf("\n");
				
                // JD - Joining string to create name of file
                strcat(currentComic, date);
                strcat(currentComic, cartoon[cartoonIndex]);
                strcat(currentComic, ".bmp");
                
                printf("Trying to open "); printf(currentComic); printf("\n");

                // JD - Checking if file is there
                if(access(currentComic,F_OK)==0){
                    printf("Displaying Comic"); printf("\n");
                    displayComic(currentComic);
                }
                else{
                    // JD - There should be a font that is 70 and 30 so i need to do that still
                    printf("File not present displaying Error"); printf("\n");
					EPD_5IN65F_Show7Block();
                    //Paint_DrawRectangle(0, 0, 600, 80, EPD_5IN65F_RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
                    //Paint_DrawString_EN(200, 0, "Error", &Font20, EPD_5IN65F_BLACK, EPD_5IN65F_WHITE);
                    //Paint_DrawString_EN(0, 90, "Type:", &Font20, EPD_5IN65F_RED,EPD_5IN65F_BLACK);
                    //Paint_DrawString_EN(220, 90, "File Error", &Font20, EPD_5IN65F_RED,EPD_5IN65F_BLACK);
                    //Paint_DrawString_EN(10, 140, "Information:", &Font24, EPD_5IN65F_RED,EPD_5IN65F_BLACK);
                    //Paint_DrawString_EN(10, 170, currentComic, &Font24, EPD_5IN65F_RED,EPD_5IN65F_BLACK);
                    //Paint_DrawString_EN(10, 370, u'你犯了一個嚴重的錯誤', &Font20, EPD_5IN65F_RED,EPD_5IN65F_BLACK);
                }
                
                isFirstRun = FALSE;
                cartoonIndex++;
                
                // JD - Delay is not needed 
                DEV_Delay_ms(1000);
            }
            else if(displayState == MEME){
                // JD - TODO
            }
            else if(displayState == PICS){
                // JD - TODO
            }
        }
        // JD - Delays for 0.2 Seconds or 200ms
        DEV_Delay_ms(200);
    }
    
    return 0;
}
