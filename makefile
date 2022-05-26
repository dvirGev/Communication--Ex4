.PHONY: all run clean
CC = gcc
FLAGS= 
HEADERS = 
all: sender measure 

sender: sender.o
	$(CC) $< -o sender 
measure: measure.o
	$(CC) $< -o measure -lpthread
# main: main.o
# 	$(CC) $< -o main
%.o: %.c 
	$(CC) -c $< -o $@

clean:
	rm -f *.o sender measure output.txt
