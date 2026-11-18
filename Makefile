CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra

calculator: calculator.c
	$(CC) $(CFLAGS) -o calculator calculator.c

clean:
	rm -f calculator calculator.exe

.PHONY: clean
