CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = src/main.c src/server/server.c 
OUT = http-server

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run:
	./$(OUT)

clean:
	rm -f $(OUT)