CC=gcc
CFLAGS=-Wall -g
PROJ=miniftpd
OBJ=main.o session.o sysutil.o ftpproto.o privparent.o
$(PROJ):$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(PROJ) $(OBJ)
