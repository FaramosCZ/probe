CC=g++ -std=c++11

all: cli

cli:
	$(CC) -Wall -Wextra -Werror -pedantic login/*.cpp -o login.exe

clean: 
	rm -rf login.exe