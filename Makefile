LIB = 
OBJ = o
CFLAGS = -g -O2 -Wall
PREFIX = /usr/local
OBJS = server.$(OBJ) commander.$(OBJ)

all: $(OBJS)
	$(CC) $(CFLAGS) -o server $(OBJS) $(LIB)

server.$(OBJ): commander.h include.h
commander.$(OBJ): commander.h include.h


.PHONY: install
install: server
	install -m 0755 server $(PREFIX)/bin


.PHONY: clean
clean:
	$(RM) server $(OBJS)