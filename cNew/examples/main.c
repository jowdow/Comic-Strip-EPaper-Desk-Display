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

#define DEBUGPRINT
enum State {COMIC = 0, MEME = 1, PICS = 2}; // JD -  These are the possible states for 
char comicsDirectory_c[11] = "/pic/comics";
char cartoon_c[MAX_CARTOONS][MAX_LINE_LENGTH]; // JD - Hard coding the cartoon
char weather_c[18][50]; // JD - I am storing this in a weird way. even though it is a 2d array the second dimension is just to store the chars ex. Day,Description,Temps,Day,Description,Temps,Day,Description,Temps,Day,Description,Temps,Day,Description,Temps,Day,Description,Temps

void  Handler(int signo)
{
    //System Exit
    printf("\r\nHandler:exit\r\n");
    DEV_Module_Exit();

    exit(0);
}

void initCartoonVar(void)
{
    char filename[] = "cartoons.txt";  // Specify the filename
    int lineCount = 0;

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
    }

    // Read lines from the file
    while (fgets(cartoon_c[lineCount], MAX_LINE_LENGTH, file) != NULL) {
        // Remove the newline character if present
        char* newlinePos = strchr(cartoon_c[lineCount], '\n');
        if (newlinePos != NULL) {
            *newlinePos = '\0';
        }

        lineCount++;

        if (lineCount >= MAX_CARTOONS) {
            printf("Maximum line count reached. Some lines may not be read.\n");
            break;
        }
    }
    // Closing the file
    fclose(file);
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
            weather_c[dataTypeIndex_u8][dataIndex_u8] = character_c;// JD - Actually copying the weather data to the local variable
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

    #ifdef DEBUGPRINT
    printf("e-Paper Init and Clear...\r\n");
    #endif

    EPD_5IN65F_Init();
    
    // JD - This is to get the time it takes to clear the image , most of the timing stuff is not needed however we do need to clear the screen
    EPD_5IN65F_Clear(EPD_5IN65F_WHITE);
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
    
    GUI_ReadBmp_RGB_7Color(comic, 0, 0);
    #ifdef DEBUGPRINT
    printf("Showing "); printf("%s",comic); printf("\r\n");
    #endif
	EPD_5IN65F_Display(comicImage);  // JD - Actually displaying the image
    DEV_Delay_ms(5000);
    
    #ifdef DEBUGPRINT
	printf("e-Paper Sleep...\r\n");
    #endif
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

void deletePreviousComics(char pathToComicsWithData_c[]){
    // JD - We need to know how many comics we have so this finds if data is present and adds to a running sum
    uint8_t num_rows_with_data_u8 = 0;
    for(uint8_t i=0;i<MAX_CARTOONS;i++){
        if (cartoon_c[i][0] != 0) {
                num_rows_with_data_u8++;
        }
        else{
            break;
        }
    }

    printf("%s\n",pathToComicsWithData_c);
    // JD - This deletes previous .bmp and 0gif files 
    char fullPath_c[300]="";
    for(int x = 0; x<=num_rows_with_data_u8;x++){
        
        memset(fullPath_c,0,sizeof(fullPath_c));
        sprintf(fullPath_c, "%s%s.bmp", pathToComicsWithData_c, cartoon_c[x]);
        if (remove(fullPath_c) != 0) {// JD - using the file path to try to delete the file
            #ifdef DEBUGPRINT
            printf("Error deleting file '%s' BMP\n", cartoon_c[x]);
            #endif
        } else {
            #ifdef DEBUGPRINT
            printf("Deleted file '%s' BMP\n", cartoon_c[x]);
            #endif
        }
        
        memset(fullPath_c,0,sizeof(fullPath_c));
        sprintf(fullPath_c, "%s%s.gif", pathToComicsWithData_c, cartoon_c[x]);
        if (remove(fullPath_c) != 0) { // JD - using the file path to try to delete the file
            #ifdef DEBUGPRINT
            printf("Error deleting file '%s' GIF\n", cartoon_c[x]);
            #endif
        } else {
            #ifdef DEBUGPRINT
            printf("Deleted file '%s' GIF\n", cartoon_c[x]);
            #endif
        }
    }
    printf("\n\n");
}

int isLeapYear(int year) {
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
        return 1;
    else
        return 0;
}

int getLastDayOfMonth(int year, int month) {
    int lastDay;

    switch (month) {
        case 1: // January
        case 3: // March
        case 5: // May
        case 7: // July
        case 8: // August
        case 10: // October
        case 12: // December
            lastDay = 31;
            break;
        case 4: // April
        case 6: // June
        case 9: // September
        case 11: // November
            lastDay = 30;
            break;
        case 2: // February
            lastDay = isLeapYear(year) ? 29 : 28;
            break;
        default:
            lastDay = -1; // Invalid month
            break;
    }

    return lastDay;
}

void getPreviousDay(char* date, char* previousDay) {
    int year, month, day;
    sscanf(date, "%d-%d-%d", &year, &month, &day);

    // Adjust the date
    if (day > 1) {
        day--;
    } else {
        if (month > 1) {
            month--;
            day = getLastDayOfMonth(year, month);
        } else {
            year--;
            month = 12;
            day = getLastDayOfMonth(year, month);
        }
    }

    sprintf(previousDay, "%04d-%d-%02d", year, month, day);
}


int main(void)
{
    // Exception handling:ctrl + c
    signal(SIGINT, Handler);
    
    UBYTE *BlackImage; // Memory for image
    displayInit(BlackImage);
    
    uint8_t isFirstRun = TRUE;
    uint8_t isNewMinute = FALSE;
    
    // JD - Time declaration 
    time_t t= time(NULL);
    struct tm tm = *localtime(&t);
    char year_c[4];
    char month_c[2];
    char day_c[2];
    char hour_c[2];
    char min_c[2];
    char currentComic_c[300];
    char date_c[30];
    char previousDayDate_c[30];
    

    char previousMinute_c = '0';
    uint8_t cartoonIndex_u8 = 0;
    uint8_t cartoonIndexMax_u8 = 0;
    
    enum State displayState = COMIC;
    
    // JD - Time intialization
    memset(date_c,0,sizeof(date_c));
    t= time(NULL);
    tm = *localtime(&t);

    // JD - Below is getting the current directory and then formatting the command_c to run the python script
    char command_c[200]="";
    char directory_c[100]; 
    if( getcwd( directory_c , sizeof(directory_c) ) != NULL){ // If current directory is not NULL
        printf("Current Directory: ");printf(directory_c);
        printf("\n");
        snprintf(command_c, 200, "python %s/main.py", directory_c);
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

    // JD - This could be put in a better place however cartoonIndexMax is a local variable so that makes it a little harder
    // JD - This finds how many cartoons we have and then saves that to max so we loop through them all
    for(uint8_t i=0;i<MAX_CARTOONS;i++){
        if (cartoon_c[i][0] != 0) {
                cartoonIndexMax_u8++;
        }
        else{
            break;
        }
    }

    sprintf(date_c, "%s-%s-%s", year_c, month_c, day_c);
    getPreviousDay(date_c, previousDayDate_c);

    char pathToComicsWithData_c[300]="";
    sprintf(pathToComicsWithData_c, "%s%s/%s", directory_c, comicsDirectory_c, previousDayDate_c);
    deletePreviousComics(pathToComicsWithData_c);
    


    while(TRUE){
        
        // JD - Grabbing time and memset
        memset(date_c,0,sizeof(date_c));
        t= time(NULL);
        tm = *localtime(&t);
        // JD - Initalizing the time just received 
        sprintf( hour_c,"%d", tm.tm_hour );
        sprintf( min_c,"%d", tm.tm_min );
        sprintf( year_c,"%d",( tm.tm_year + 1900 ) );
        
        // JD - this is just to print when there is a new minute so we know that the program is running
        if(isFirstRun || (min_c[1] != NULL && min_c[1] != previousMinute_c) || (min_c[1] == NULL && min_c[0] != previousMinute_c)){ // JD - If it is a singal digit in the minute (:01 - :09) the min_c variable stores a singal digit so we check if the min_c[1] is null to see if it is singal digits
            if(min_c[1] == NULL){
                previousMinute_c = min_c[0];
                isNewMinute = TRUE;
                printf("Current Time: %c%c:%c%c \n \n",hour_c[0],hour_c[1],'0',min_c[0]);
            }else{
                previousMinute_c = min_c[1];
                isNewMinute = TRUE;
                printf("Current Time: %c%c:%c%c \n \n",hour_c[0],hour_c[1],min_c[0],min_c[1]);
            }
        }
        else{
            isNewMinute = FALSE;
        }

        // JD - Update weather and comics if (((time is 7:48) or (time is 12:55) or isFirstRun) and isNewMinute). One could argue that this should go in the below if statment that actually displays the image but I want a custom time to run the updating and you can't do to the minute custom data gathering in the below if statment
        if((((char)hour_c[0]=='7' && (char)min_c[0]=='4' && (char)min_c[1]=='8') || ((char)hour_c[0]=='1' && (char)hour_c[1]=='2' && (char)min_c[0]=='4' && (char)min_c[1]=='5') || isFirstRun ) && isNewMinute){ 
            sprintf( month_c,"%d",( tm.tm_mon + 1 ) ); // JD - We update this here because I only want it to update the month when we know there is a new comic
            sprintf( day_c,"%d", tm.tm_mday );// JD - We update here because I only want it to update the day when we know there is a new comic

            if((char)hour_c[0]=='1' && (char)hour_c[1]=='2' && (char)min_c[0]=='4' && (char)min_c[1]=='5'){
                // JD - Creating the date
                memset(date_c,0,sizeof(date_c));
                sprintf(date_c, "%s-%s-%s", year_c, month_c, day_c);

                memset(pathToComicsWithData_c,0,sizeof(pathToComicsWithData_c));
                sprintf(pathToComicsWithData_c, "%s%s/%s", directory_c, comicsDirectory_c,date_c);
                deletePreviousComics(pathToComicsWithData_c); // JD - Deleting today's comics so that the weather data is updated
            }

            system(command_c);
            initCartoonVar();
            initWeatherVar();
        }
        
        // JD - Time to show new picture
        if( ( (( (char)min_c[1]== '0' || ((char)min_c[1]== NULL && (char)min_c[0]== "0") ) && isNewMinute) && (tm.tm_wday > 0 && tm.tm_wday < 6)) || isFirstRun ){ // JD - If (((it is 00,10,20,30,40,50,60  and it is a new minute ) and it is during the weekday,tm.tm_wday 1-5 is monday-friday) or it is our first time running)

            if(isFirstRun){
                isFirstRun = FALSE;
            }

            switch(displayState){
                case COMIC:
                    if(cartoonIndex_u8 > cartoonIndexMax_u8){ // JD - Resetting index so we don't call out of range
                        cartoonIndex_u8 = 0;
                    }
                    // JD - Creating the date
                    sprintf(date_c, "%s-%s-%s", year_c, month_c, day_c);

                    // JD - Printing time and current comic
                    memset(currentComic_c,0,sizeof(currentComic_c));
                    printf("Current Time: %c%c:%c%c \n",hour_c[0],hour_c[1],min_c[0],min_c[1]);
                    printf("Date: ");printf(date_c);printf("\n");
                    printf("Cartoon: ");printf(cartoon_c[cartoonIndex_u8]);printf("\n");
                    
                    // JD - Joining string to create name of file
                    sprintf(currentComic_c, "%s%s/%s%s.bmp", directory_c, comicsDirectory_c, date_c, cartoon_c[cartoonIndex_u8]);
                    
                    #ifdef DEBUGPRINT
                    printf("Trying to open: ");printf(currentComic_c);printf("\n");
                    #endif

                    // JD - Checking if file is there
                    if(access(currentComic_c,F_OK)==0){
                        printf("Displaying Comic");printf("\n");printf("\n");
                        displayComic(currentComic_c,date_c,hour_c,min_c);
                    }
                    else{
                        printf("File not present displaying Error message\n");
                        //EPD_5IN65F_Show7Block();
                    }
                    
                    isFirstRun = FALSE;
                    cartoonIndex_u8++;
                    break;
                case MEME:
                    // JD - TODO
                    break;
                    // JD - TODO
                case PICS:
                    break;
            }
        }
        // JD - Delays for 0.2 Seconds or 200ms. This is just to do less processing
        DEV_Delay_ms(200);
    }
    
    return 0;
}