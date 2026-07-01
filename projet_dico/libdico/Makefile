#PROJET JULIEN FISCHER ET XAVIER DUBARRY FIPA28 ENSTA
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -fPIC
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
EXEC = bin/bench_dico
LIB = lib/libdico.so

all: $(EXEC) $(LIB)

$(EXEC): $(OBJ)
	@mkdir -p bin
	$(CC) $(OBJ) -o $@

$(LIB): $(OBJ)
	@mkdir -p lib
	$(CC) -shared -o $@ $(OBJ)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC) $(LIB)

.PHONY: all clean