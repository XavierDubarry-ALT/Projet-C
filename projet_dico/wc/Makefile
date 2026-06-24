CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -I../libdico/src
LDFLAGS = -L../libdico/lib -ldico
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
EXEC = bin/WordCount
all: $(EXEC)

$(EXEC): $(OBJ)
	@mkdir -p bin
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean