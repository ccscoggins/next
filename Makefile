CC = gcc
LIBS = -lncursesw
DEPS = lcui.h
OBJ = lcui.o next.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

next: $(OBJ)
	$(CC) -o $@ $^ $(CLAGS) $(LIBS)
