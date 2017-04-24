CFLAGS = -Wall -Werror

CC = mpicc
PROGRAM = FIT3143_A2
DEPS =
OBJ = main.o

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS)

$(PROGRAM): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $PROGRAM) *.o