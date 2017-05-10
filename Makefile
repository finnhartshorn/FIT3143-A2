DEVCFLAGS = -Wall -Werror -std=c99
CFLAGS = -std=c99
LDFLAGS = -L pcg-c-0.94/src/
LDLIBS = -lpcg_random -lm

CC = mpicc
PROGRAM = FIT3143_A2
DEPS = main.h send_receive.h pcg-c-0.94/extras/pcg_spinlock.h pcg-c-0.94/extras/entropy.h
OBJ = main.o send_receive.o pcg-c-0.94/extras/entropy.o
INCLUDE = -I pcg-c-0.94/include -I pcg-c-0.94/extras

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS) $(INCLUDE)

$(PROGRAM): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(PROGRAM) *.o