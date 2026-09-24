CC = gcc

CFLAGS = -Wall -Wextra -g -Isrc
LDFLAGS = -pthread

SRC = \
	src/main.c \
	src/server/server.c \
	src/http/http.c \
	src/http/parser.c \
	src/http/request.c \
	src/http/response.c \
	src/router/router.c \
	src/handler/handler.c \
	src/file/file.c \
	src/error/error.c \
	src/form/form.c \
	src/logger/logger.c \
	src/mime/mime.c \
	src/utils/utils.c

OUT = build/http-server


all:
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)


run: all
	./$(OUT)


clean:
	rm -rf build