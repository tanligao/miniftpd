CC=gcc
CFLAGS=-Wall -g
PROJ=hash
OBJ=main.o hash.o
$(PROJ):$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(PROJ) $(OBJ)
