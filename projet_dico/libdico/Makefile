CC = gcc
CFLAGS = -Wall -Wextra -Werror -g
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
EXEC = bin/bench_dico
all: $(EXEC)

$(EXEC): $(OBJ)
	@mkdir -p bin
	$(CC) $(OBJ) -o $@

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean