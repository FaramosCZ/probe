CC=g++ -std=c++11

all: cli

cli:
	$(CC) -Wall -Wextra -Werror -pedantic -o4 -o login.exe login/*.cpp

clean: 
	rm -rf login.exe
