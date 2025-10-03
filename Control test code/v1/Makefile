CC = gcc-10
CFLAGS = -O0 -Wall
LDFLAGS = -lm 
OBJS = p_control_test.o p_func.o 
EXECUTABLE = p_control_test



all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(EXECUTABLE)

funcs.o: p_func.h p_func.c
	$(CC) $(CFLAGS) -c funcs.c

p_control_test.o: p_control_test.c p_func.h
	$(CC) $(CFLAGS) -c p_control_test.c $(LDFLAGS) 

clean:
	rm -f *.o  $(EXECUTABLE)