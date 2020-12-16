    CC     = gcc -std=c11 -g
    CFLAGS = -Wall
    LFLAGS =

      PROG = ftp
      OBJS = rawsocket.o \
            utils.o \
            servidor.o \
            cliente.o \
            crc.o \
            comandos.o \
             $(PROG).o

.PHONY: limpa faxina clean distclean purge all

%.o: %.c %.h rawsocket.h
	$(CC) -c $(CFLAGS) $<

$(PROG):  $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

clean:
	@rm -f *~ *.bak

purge:   clean
	@rm -f *.o core a.out
	@rm -f $(PROG)
