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

//char cartoon_c[6][20] = {{"garfield"},{"pickles"},{"bc"},{"peanuts"},{"dilbert-classics"}}; // JD - Hard coding the cartoon
char cartoon_c[MAX_CARTOONS][MAX_LINE_LENGTH]; // JD - Hard coding the cartoon
char weather[18][50]; // JD - I am storing this in a weird way. even though it is a 2d array the second dimension is just to store the chars ex. Day,Description,Temps,Day,Description,Temps,Day,Description,Temps,Day,Description,Temps,Day,Description,Temps,Day,Description,Temps

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
            cartoon_c[cartoonNumber_u8][cartoonCharNumber_u16] = character_c;
            cartoonCharNumber_u16++;
        }
        
    }
    // Closing the file
    fclose(file_ptr);
}

void initWeatherVar(void)
{    
    FILE* file_ptr;
    char character_c;
    uint8_t dataIndex_u8 = 0;
    uint8_t dataTypeIndex_u8 = 0;
    
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
    UDOUBLE Imagesize = ((EPD_5IN65F_WIDTH % 2 == 0)? (EPD_5IN65F_WIDTH / 2 ): (EPD_5IN65F_WIDTH / 2 + 1)) * EPD_5IN65F_HEIGHT;
    if((comicImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for blank memory...\r\n");
        return -1;
    }
    Paint_NewImage(comicImage, EPD_5IN65F_WIDTH, EPD_5IN65F_HEIGHT, 0, EPD_5IN65F_WHITE);
    Paint_SetScale(7);
    
    // JD - Clearing Display
    Paint_Clear(EPD_5IN65F_WHITE);
    
    //strcat(comic,".bmp");
    GUI_ReadBmp_RGB_7Color(comic, 0, 0);
    printf("Showing "); printf("%s",comic); printf("\r\n");
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
        return;
    }
    else{
        EPD_5IN65F_Init();
        DEV_Delay_ms(100);

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
    char year_c[4];
    char month_c[2];
    char day_c[2];
    char hour_c[2];
    char min_c[2];
    char currentComic_c[30];
    char date_c[30];
    

    char previousMinute = '0';
    char bufferedDay[2]; // JD - This is used to buffer the day because they \
                                don't update the comics at midnight so we buffer the day until it is updated
    int cartoonIndex = 0;
    int cartoonIndexMax = 4;
    
    enum State displayState = COMIC;
    
    // JD - Time intialization
    memset(date_c,0,sizeof(date_c));
    t= time(NULL);
    tm = *localtime(&t);

    // JD - Below is a very janky workaround because 
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
    sprintf( hour_c,"%d", tm.tm_hour );
    sprintf( min_c,"%d", tm.tm_min );
    sprintf( year_c,"%d",( tm.tm_year + 1900 ) );
    sprintf( month_c,"%d",( tm.tm_mon + 1 ) );
    sprintf( day_c,"%d", tm.tm_mday );
    
    initCartoonVar(); // JD - This is needed to init the variable used to call the images which are saved by name
    initWeatherVar();
    
    while(TRUE){
        
        // JD - Grabbing time and memset
        memset(date_c,0,sizeof(date_c));
        t= time(NULL);
        tm = *localtime(&t);
        // JD - Initalizing the time just received 
        sprintf( hour_c,"%d", tm.tm_hour );
        sprintf( min_c,"%d", tm.tm_min );
        sprintf( year_c,"%d",( tm.tm_year + 1900 ) );
        
        if(isFirstRun || min_c[1] != previousMinute){ // JD - We need to know when it is the next minute or the first run
            previousMinute = min_c[1];
            printf("Current Time: %c%c:%c%c \n \n",hour_c[0],hour_c[1],min_c[0],min_c[1]);
            isNewMinute = TRUE;
        }
        else{
            isNewMinute = FALSE;
        }

        if((((char)hour_c[0]=='0' && (char)hour_c[1]=='7' && (char)min_c[0]=='5' && (char)min_c[1]=='5') || isFirstRun ) && isNewMinute){ // Update weather and comics if (((time is 7:55) or isFirstRun) and isNewMinute)
                sprintf( month_c,"%d",( tm.tm_mon + 1 ) ); // JD - We update this here because I only want it to update the month when we know there is a new comic
                sprintf( day_c,"%d", tm.tm_mday );// JD - We update here because I only want it to update the day when we know there is a new comic
                
                system(command);
                initCartoonVar();
                initWeatherVar();
        }
        
        if((((char)min_c[1]=='0') && isNewMinute) || isFirstRun){ // JD - If it is 10,20,30,40,50,60  and it is a new minute or it is our first time running

            if(isFirstRun){
                isFirstRun = FALSE;
            }

            if(displayState == COMIC){
                if(cartoonIndex > cartoonIndexMax){ // JD - Resetting index 
                    cartoonIndex = 0;
                }
                // JD - Creating the date
				strcat(date_c, year_c);
				strcat(date_c, "-");
				strcat(date_c, month_c); 
				strcat(date_c, "-");
				strcat(date_c, day_c); 

                // JD - Printing time and current comic
                memset(currentComic_c,0,sizeof(currentComic_c));
                printf("Current Time: %c%c:%c%c \n",hour_c[0],hour_c[1],min_c[0],min_c[1]);
                printf("Date: ");printf(date_c);printf("\n");
                printf("Cartoon: ");printf(cartoon_c[cartoonIndex]);printf("\n");
				
                // JD - Joining string to create name of file
                strcat(currentComic_c, date_c);
                strcat(currentComic_c, cartoon_c[cartoonIndex]);
                strcat(currentComic_c, ".bmp");
                
                printf("Trying to open: "); printf(currentComic_c);printf("\n");

                // JD - Checking if file is there
                if(access(currentComic_c,F_OK)==0){
                    printf("Displaying Comic");printf("\n");printf("\n");
                    displayComic(currentComic_c,date_c,hour_c,min_c);
                }
                else{
                    // JD - There should be a font that is 70 and 30 so i need to do that still
                    printf("File not present displaying Error message\n");
                    //EPD_5IN65F_Show7Block();
                    //Paint_DrawRectangle(0, 0, 600, 80, EPD_5IN65F_RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
                    //Paint_DrawString_EN(200, 0, "Error", &Font20, EPD_5IN65F_BLACK, EPD_5IN65F_WHITE);
                    //Paint_DrawString_EN(0, 90, "Type:", &Font20, EPD_5IN65F_RED,EPD_5IN65F_BLACK);
                    //Paint_DrawString_EN(220, 90, "File Error", &Font20, EPD_5IN65F_RED,EPD_5IN65F_BLACK);
                    //Paint_DrawString_EN(10, 140, "Information:", &Font24, EPD_5IN65F_RED,EPD_5IN65F_BLACK);
                    //Paint_DrawString_EN(10, 170, currentComic_c, &Font24, EPD_5IN65F_RED,EPD_5IN65F_BLACK);
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
        // JD - Delays for 0.2 Seconds or 200ms. This is just to do less processing
        DEV_Delay_ms(200);
    }
    
    return 0;
}
