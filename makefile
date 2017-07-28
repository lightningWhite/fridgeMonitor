####################################################
# Program:
#     Fridge Monitor
# Author:
#     Daniel Hornberger
# Summary:
#     Monitors the opening and closing of the fridge
#     and keeps a record of when and how long the
#     fridge was open during each day.
####################################################


LFLAGS = -lpigpio, -lstdio, -lstdlib, -ltime, -lsignal


a.out: main.o
	gcc -Wall -pthread -o fridgeMon main.c -lpigpio -lrt -std=c11 

clean:
	rm a.out *.o
