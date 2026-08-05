CC = gcc
CFLAGS = -Wall -Wextra -g -Isrc
SRC = src/main.c src/server/server.c src/http/http.c src/http/request.c src/http/response.c src/router/router.c src/file/file.c src/mime/mime.c src/logger/logger.c src/handler/handler.c
OUT = http-server

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run:
	./$(OUT)

clean:
	rm -f $(OUT)