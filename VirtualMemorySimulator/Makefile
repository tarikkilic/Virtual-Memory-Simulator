
FLAGS= -Wall -ansi -pedantic -errors  -std=c++11

all: sortArrays

sortArrays: sortArrays.o 
				 g++ sortArrays.o  -o sortArrays -pthread -lm
sortArrays.o : sortArrays.cpp
				 g++  -c ${FLAGS} sortArrays.cpp
clean:
	rm *.o sortArrays  
