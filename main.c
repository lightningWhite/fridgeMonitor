/**************************************************************
 * FRIDGE MONITOR
 * This program monitors the number of times the fridge door
 * opens throughout the day and computes the accumulated
 * time the door is open throughout the day. A log is kept
 * of what times the door was opened and for how long it was
 * open.
 *
 * Using pigpio library for GPIO interface.
 * 
 * Pin 16 - Input GPIO 23
 * Pin 17 - Vcc (3.3V)
 * Pin 20 - GROUND
 **************************************************************/
#include <stdio.h>
#include <stdlib.h> // For exit(0)
#include <pigpio.h> 
#include <time.h>
#include <signal.h> // For signal handling
#include <string.h> // For strcat()

/************************************************************** 
 * SIGHANDLER
 * Signal Handler for Ctrl+C to cleanly close the file before
 * exiting the program.
 **************************************************************/
void sigHandler(int signum, void* fp)
{
  // Cleanly close the file before exiting
  printf("\nClosing file...\n");
  fclose(fp);
  
  printf("Exiting...\n");
  exit(0);   
}

/************************************************************** 
 * SETUPGPIO
 * Initiallize pigpio and configure the pins.
 **************************************************************/
int setupGPIO()
{
  // setup gpio pins
  if (gpioInitialise() < 0)
  {
    printf("pigpio initialization failed... Exiting.\n");
    return -1;
  }
  else
  {
    printf("Successful pigpio initialization...\n");
  }

  gpioSetMode(23, PI_INPUT);        // Set gpio 23 as input

  return 0;
}

/************************************************************** 
 * OPEN FILE
 * Open a file to log open/close time information.
 **************************************************************/
int openFile(FILE* fp)
{
  char filepath[256];
  strcpy(filepath, "/tmp/fridgeLogs/");
  time_t currentTime = time(NULL);

  char filenameStr[256];
  sprintf(filenameStr, "%ju", (uintmax_t)currentTime);
  strcat(filepath, filenameStr);
  
  fp = fopen(filepath, "a+");
  if (fp == NULL)
  {
    // If open failed
    return -1;
  }
  
  fprintf(fp, "This is a test...\n");

  // Set up handler to catch Ctl+C for closing file before exit
  gpioSetSignalFuncEx(SIGINT, sigHandler, fp);
  
  return 0;
}

/************************************************************** 
 * GET START TIME
 * Get the current time and return it as a string.
 **************************************************************/
int getStartTime(char* startTimeString)
{
  // Get current time as a string
  time_t startTime;
  struct tm *startTimeInfo;
  time(&startTime);
  startTimeInfo = localtime(&startTime);

  // Temporary copy of time info returned by asctime()
  char* tempStartTimeString = asctime(startTimeInfo);

  // Save the start time info for logging purposes
  int i;
  for(i = 0; tempStartTimeString[i] != '\0'; i++)
  {
     startTimeString[i] = tempStartTimeString[i];
  }
  startTimeString[i] = '\0';

  return 0;
}

/************************************************************** 
 * HANDLE OPENED STATE
 * Get the current time, print and log open message.
 **************************************************************/
void handleOpenedState(time_t* openTime, FILE* fp)
{
  // Get current time and print it
  struct tm *timeinfo;
  time(openTime); 
  timeinfo = localtime(openTime);
  fprintf(fp, "This is a test...\n");
  
  // Create log message
  char msg[256];
  strcpy(msg, "The fridge opened at ");
  strcat(msg, asctime(timeinfo));

  // SEG FAULT!!! fprintf(fp, "msg\n");
  
  // Display the log message
  printf("The fridge opened at %s", asctime(timeinfo));  
}

/************************************************************** 
 * HANDLE CLOSED STATE
 * Get the current time, print and log open message.
 **************************************************************/
void handleClosedState(time_t* closeTime, time_t* openTime,
		       unsigned long long int* totalOpenTime,
		       const unsigned int numOpenings,
		       char* startTimeString, FILE* fp)
{
  
  
  // To display current time when fridge closes
  struct tm *timeinfo;
  
  time(closeTime);
  timeinfo = localtime(closeTime);
  
  double seconds = difftime(*closeTime, *openTime);
  *totalOpenTime += seconds;

  printf("The fridge closed at %s", asctime(timeinfo));
  printf("The fridge was open for %.f seconds.\n", seconds);
  printf("The fridge has opened %u times and total open time is %llu seconds since %s\n", numOpenings, *totalOpenTime, startTimeString);
}


/************************************************************** 
 * MAIN
 * Monitor the GPIO pins and log openings/closings. Record 
 * the total time open. 
 **************************************************************/
int main()
{
  // Setup the GPIO pins
  if (setupGPIO() < 0)
    return -1;

  
  // BUFFER OVERFLOW POSSIBILITY
  char startTimeString[256];
  
  // Get the time information for logging
  getStartTime(startTimeString);

  FILE *fp =  NULL;

  if(openFile(fp) < -1)
  {
    printf("Failed to open log file... Exiting.\n");
    return -1;
  }
  
  time_t openTime = NULL;
  time_t closeTime = NULL;
  unsigned long long int totalOpenTime = 0;
  unsigned int numOpenings = 0;

  // For toggle   
  int openStatus = 0;
  
  // Polling loop to check fridge open/close status
  while (1)
  {
    // Debounce the input signal (inelegant way)
    int i = 0;
    while (i < 100000)
    {
      i++;
    }
    
    // Check if the fridge is open (0) or closed (1)
    if (gpioRead(23) && !openStatus)
    {
      // Log an opening
      handleOpenedState(&openTime, fp);
      
      // Increment numOpenings count
      numOpenings++;
       
      // Only display the message once
      openStatus = 1;
    }
    else if (!gpioRead(23) && openStatus)
    {
      // Log info pertaining to a closing
      handleClosedState(&closeTime, &openTime, &totalOpenTime, numOpenings, startTimeString, fp);
     
      // Only display the message once
      openStatus = 0;
    }	
  }

  gpioTerminate();
  return 0;
}
