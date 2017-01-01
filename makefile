CC=gcc
CFLAGS=-Wall -g
PROJ=miniftpd
OBJ=main.o sysutil.o
$(PROJ):$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(PROJ) $(OBJ)