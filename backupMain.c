 /**********************************************************
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
 **********************************************************/
#include <stdio.h>
#include <stdlib.h> // For exit(0)
#include <pigpio.h> 
#include <time.h>
#include <signal.h> // For signal handler

//static int keepRunning = 1;

void sigHandler(int);
void sigHandler2(int);

int main()
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

  // Set up handler to catch Ctl+C
  gpioSetSignalFunc(SIGINT, sigHandler);  


  gpioSetMode(23, PI_INPUT);        // Set gpio 23 as input

  // For toggle   
  int openStatus = 0;

  // Get the time information for logging
  time_t startTime;
  struct tm *startTimeInfo;
  time(&startTime);
  startTimeInfo = localtime(&startTime);

  // Temporary copy of time info returned by asctime()
  char* tempStartTimeString = asctime(startTimeInfo);
  
  // BUFFER OVERFLOW POSSIBILITY
  char startTimeString[256];

  // Copy the start time info for logging purposes
  int i = 0;
  for(i; tempStartTimeString[i] != '\0'; i++)
  {
     startTimeString[i] = tempStartTimeString[i];
  }
  startTimeString[i] = '\0'; 
  
  time_t openTime;
  time_t closeTime;
  unsigned long long int totalOpenTime = 0;
  
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
      // To display current time when fridge opens
      struct tm *timeinfo;
      
      time(&openTime);
      timeinfo = localtime(&openTime);

      printf("The fridge opened at %s", asctime(timeinfo));

      
      // Only display the message once
      openStatus = 1;
    }
    else if (!gpioRead(23) && openStatus)
    {
      // To display current time when fridge closes
      struct tm *timeinfo;
      
      time(&closeTime);
      timeinfo = localtime(&closeTime);

      double seconds = difftime(closeTime, openTime);
      totalOpenTime += seconds;
      
      printf("The fridge closed at %s", asctime(timeinfo));
      printf("The fridge was open for %.f seconds.\n", seconds);
      printf("The fridge has been open for %llu seconds since %s", totalOpenTime, startTimeString);//asctime(startTimeInfo));
      printf("\n");
      
      // Only display the message once
      openStatus = 0;
    }	
  }

  gpioTerminate();
  return 0;
}

void sigHandler(int signum)
{
  // Create a handler for emergency exits
  // NOTE that by doing this Ctrl+C will just exit. This is not wanted...
  gpioSetSignalFunc(SIGINT, sigHandler2);  

   // Display the options
   printf("\nCaught signal %d...\n", signum);
   printf("What would you like to do?\n\n");
   printf("d - display the log and resume\n");
   printf("r - resume monitoring\n");
   printf("q - Cleanly exit the program\n\n");
   printf("> ");

   char option = 0;

   scanf("%c", &option);

   switch (option)
   {
      case 'd':
        // display the log and resume
        
	printf("Resume monitoring...\n");
        break;
      case 'r':
	// resume monitoring
	printf("Resume monitoring...\n");      
        break; 
      case 'q':
      default:
        // close the file and exit
	exit(0);
   }
   
   // Set the handler back to this function
   gpioSetSignalFunc(SIGINT, sigHandler);  
   return;   
}

void sigHandler2(int signum)
{
  printf("\nExiting...\n");
  exit(0);
}
