CC = gcc
LIBS = -lncursesw
DEPS = lcui.h
OBJ = lcui.o next.o
CFLAGS = -g

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

next: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm $(OBJ) next
