CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LIBS = -lncurses
SRC = src/bank_system.c
OUT = bank_system

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(OUT) *.o accounts.dat trans_*.txt

.PHONY: all clean
