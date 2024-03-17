##g++ -g -Wall -std=c++17 progFiles.cpp -o progName ###where progName will have .exe after it

# the complier: gcc for C g++ for C++
CC = g++ #gcc

# compiler flags:
#  -g			- this flag adds debugging information to the executable file
#  -Wall		- this flag is used to turn on most compiler warnings
#  -Werror  	- this makes a ll warnings into errors
#  -pedantic	- issues all warnings demanded by strict ISO C++ rules if you want to be extra safe
#  -Wextra		- enables extra warnings not in '-Wall', including warnings for bad pointer to int, zero comparisons, base class not init in copy constructor of derived class, etc

CFLAGS = -g -Wall

# The build target
PROG1 = HelloWorld

all: $(PROG1)

$(PROG1): $(PROG1).c 
			$(CC) $(CFLAGS) -o $(PROG1) $(PROG1).c

clean:
			rm $(PROG1)