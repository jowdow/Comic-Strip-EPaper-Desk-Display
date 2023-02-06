#include <stdlib.h>     //exit()
#include <signal.h>     //signal()
#include "EPD_Test.h"   //Examples
#include "EPD_5in65f.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define RUN 1
#define NO 0
#define LOOP_DELAY 1000
#define MAX_LINE_LENGTH 600
#define MAX_CARTOONS 50
#define TRUE 1
#define FALSE 0
enum State {COMIC = 0, MEME = 1, PICS = 2};

//char cartoon[6][20] = {{"garfield"},{"pickles"},{"bc"},{"peanuts"},{"dilbert-classics"}}; // JD - Hard coding the cartoon
char cartoon[MAX_CARTOONS][MAX_LINE_LENGTH]; // JD - Hard coding the cartoon
char weather[18][50]; // JD - Hard coding the weather data. i am storing this in a weird way. even though it is a 2d array the second dimension is just to store the chars ex. Day,Description,Temps,Day,Description,Temps,Day,Description,Temps,Day,Description,Temps,Day,Description,Temps,Day,Description,Temps

void  Handler(int signo)
{
    //System Exit
    printf("\r\nHandler:exit\r\n");
    DEV_Module_Exit();

    exit(0);
}

void initCartoonVar(void)
{
    FILE* file_ptr;
    char cartoonName_c[40];
    char previousChar_c;
    char character_c;
    uint8_t cartoonNumber_u8 = 0;
    uint16_t cartoonCharNumber_u16 = 0;
    
    // Opening file in reading mode
    file_ptr = fopen("cartoons.txt", "r"); // JD - Opening the file
    if (NULL == file_ptr) {
        printf("cartoons.txt Can't be opened \n");
    }
    while(!feof(file_ptr)){  // JD - While not the end of the file
        character_c = fgetc(file_ptr); // JD - Getting the next char from the file
        if(character_c == 10){  // JD - 10 is the number for new line. we don't want the new line. I HATE THE NEW LINES BECAUSE C SUCKS ASS TO GET RID OF NEW LINE. ALL MY HOMIES HATE NEW LINES
            cartoonNumber_u8++; // JD - Now that we are at the new line this is a new comic so we need to start recording on the next array
            cartoonCharNumber_u16=0;
        }
        else{
            cartoon[cartoonNumber_u8][cartoonCharNumber_u16] = character_c;
            cartoonCharNumber_u16++;
        }
        
    }
    // Closing the file
    fclose(file_ptr);
}

void initWeatherVar(void)
{    
    FILE* file_ptr;
    char data_c[40];
    char previousChar_c;
    char character_c;
    uint8_t dataIndex_u8 = 0;
    uint8_t dataTypeIndex_u8 = 0;
    uint8_t count_u8 = 0;
    
    // Opening file in reading mode
    file_ptr = fopen("weather.txt", "r"); // JD - Opening the file
    if (NULL == file_ptr) {
        printf("weather.txt Can't be opened \n");
    }
    while(!feof(file_ptr)){  // JD - While not the end of the file
        character_c = fgetc(file_ptr); // JD - Getting the next char from the file
        if(character_c == 10 || character_c == 44){  // JD - 10 is the number for new line. we don't want the new line. I HATE THE NEW LINES BECAUSE C SUCKS ASS TO GET RID OF NEW LINE. ALL MY HOMIES HATE NEW LINES and 44 is equal to ',' in ascii
            dataTypeIndex_u8++; // JD - Now that we are at the new line this is a new comic so we need to start recording on the next array
            dataIndex_u8=0;
        }
        else{
            weather[dataTypeIndex_u8][dataIndex_u8] = character_c;
            dataIndex_u8++;
        }
        
    }
    // Closing the file
    fclose(file_ptr);
        
}

int displayComic(char *comic[], char date[], char hour[],char min[])
{
    if(DEV_Module_Init()!=0){
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
    
    EPD_5IN65F_Init();
    
    // JD - This is to get the time it takes to clear the image , most of the timing stuff is not needed however we do need to clear the screen
	struct timespec start={0,0}, finish={0,0}; 
    clock_gettime(CLOCK_REALTIME,&start);
    EPD_5IN65F_Clear(EPD_5IN65F_WHITE);
	clock_gettime(CLOCK_REALTIME,&finish);
    printf("%ld S\r\n",finish.tv_sec-start.tv_sec);
    DEV_Delay_ms(100);


    // JD - Initilzing memory used to display images and setting up those variables used
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
    
    // JD - Actually dislaying image
    Paint_Clear(EPD_5IN65F_WHITE);
    
    // TIME START
    Paint_DrawString_EN(410, 410, "Time ", &Font16, EPD_5IN65F_WHITE, EPD_5IN65F_RED);
    Paint_DrawString_EN(465, 410, hour, &Font16, EPD_5IN65F_WHITE, EPD_5IN65F_RED);
    Paint_DrawString_EN(485, 410, ":", &Font16, EPD_5IN65F_WHITE, EPD_5IN65F_RED);
    Paint_DrawString_EN(495, 410, min, &Font16, EPD_5IN65F_WHITE, EPD_5IN65F_RED);
    // TIME END
    
    // DATE START
    Paint_DrawString_EN(410, 432, "Date ", &Font16, EPD_5IN65F_WHITE, EPD_5IN65F_RED);
    Paint_DrawString_EN(465, 432, date, &Font16, EPD_5IN65F_WHITE, EPD_5IN65F_RED);
    // DATE END
    
    Paint_DrawRectangle(1, 200, 150, 448, EPD_5IN65F_RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    
    // DAY 1 START
    Paint_DrawString_EN(10, 210, weather[0], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    Paint_DrawString_EN(10, 230, weather[1], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    Paint_DrawString_EN(75, 210, weather[2], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    // DAY 1 END
    
    Paint_DrawLine(3, 250, 148, 250, EPD_5IN65F_BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    
    // DAY 2 START
    Paint_DrawString_EN(10, 260, weather[3], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    Paint_DrawString_EN(10, 280, weather[4], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    Paint_DrawString_EN(75, 260, weather[5], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    // DAY 2 END
    
    Paint_DrawLine(3, 300, 148, 300, EPD_5IN65F_BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    
    // DAY 3 START
    Paint_DrawString_EN(10, 310, weather[6], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    Paint_DrawString_EN(10, 330, weather[7], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    Paint_DrawString_EN(75, 310, weather[8], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    // DAY 3 END
    
    Paint_DrawLine(3, 350, 148, 350, EPD_5IN65F_BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    
    // DAY 4 START
    Paint_DrawString_EN(10, 360, weather[9], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    Paint_DrawString_EN(10, 380, weather[10], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    Paint_DrawString_EN(75, 360, weather[11], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    // DAY 4 END
    
    Paint_DrawLine(3, 400, 148, 400, EPD_5IN65F_BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    
    // DAY 5 START
    Paint_DrawString_EN(10, 410, weather[12], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    Paint_DrawString_EN(10, 430, weather[13], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    Paint_DrawString_EN(75, 410, weather[14], &Font12, EPD_5IN65F_RED, EPD_5IN65F_WHITE);
    // DAY 5 END
    
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
    if(DEV_Module_Init()==0){
        return -1;
    }
    else{
        EPD_5IN65F_Init();
        DEV_Delay_ms(100);

        // JD - Idk why this is comment is here as we are not using an STM32 ...
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
    uint8_t isDataUpdated = FALSE;
    
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
    char bufferedDay[2]; // JD - This is used to buffer the day because they \
                                don't update the comics at midnight so we buffer the day until it is updated
    int cartoonIndex = 0;
    int cartoonIndexMax = 4;
    
    enum State displayState = COMIC;
    
    // JD - Time intialization
    memset(date,0,sizeof(date));
    t= time(NULL);
    tm = *localtime(&t);

    char py[7] = "python";
    char command[100]="";
    char directory[80]; 
    if( getcwd( directory , sizeof(directory) ) != NULL){
        printf("Current Directory: ");printf(directory);
        printf("\n");
        strcat(command,py);
        strcat(command," ");
        strcat(command,directory);
        strcat(command,"/examples/main.py");
    }
    else{
        printf("getcwd() error");
    }

    
    // JD - Initalizing the time just received 
    sprintf( hour,"%d", tm.tm_hour );
    sprintf( min,"%d", tm.tm_min );
    sprintf( year,"%d",( tm.tm_year + 1900 ) );
    sprintf( month,"%d",( tm.tm_mon + 1 ) );
    sprintf( day,"%d", tm.tm_mday );
    
    initCartoonVar(); // JD - This is needed to init the variable used to call the images which are saved by name
    initWeatherVar();
    
    while(TRUE){
        
        // JD - Grabbing time and memset
        memset(date,0,sizeof(date));
        t= time(NULL);
        tm = *localtime(&t);
        // JD - Initalizing the time just received 
        sprintf( hour,"%d", tm.tm_hour );
        sprintf( min,"%d", tm.tm_min );
        sprintf( year,"%d",( tm.tm_year + 1900 ) );

        
        if(min[1] != previousMinute){ // JD - We need to know when it is the next minute
            previousMinute = min[1];
            printf("Min: ");printf(min);printf("\n");
            isNewMinute = TRUE;
        }
        else{
            isNewMinute = FALSE;
        }
        
        if((isFirstRun || (char)hour[1]=='6') && !isDataUpdated){ // If it is the first run or the hour is 6 am or 4 pm and data has not been updated
            sprintf( month,"%d",( tm.tm_mon + 1 ) ); // JD - We update this here because I only want it to update the month when we know there is a new comic
            sprintf( day,"%d", tm.tm_mday );// JD - We update here because I only want it to update the day when we know there is a new comic
            
	        system(command);
            initCartoonVar();
            initWeatherVar();
            
            isDataUpdated = TRUE;
        }
        else{
            isDataUpdated = FALSE;    
        }
        
        if( ( (  (char)min[1]=='0' ) && isNewMinute) || isFirstRun){ // JD - If it is 10,20,30,40,50,60  and it is a new minute or it is our first time running
            if(isFirstRun){
                isFirstRun = FALSE;
            }
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
                
                printf("Trying to open: "); printf(currentComic);

                // JD - Checking if file is there
                if(access(currentComic,F_OK)==0){
                    printf("Displaying Comic");printf("\n");printf("\n");
                    displayComic(currentComic,date,hour,min);
                }
                else{
                    // JD - There should be a font that is 70 and 30 so i need to do that still
                    printf("File not present displaying Error message\n");
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
