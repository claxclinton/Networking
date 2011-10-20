CFLAGS += -std=c99
CFLAGS += -g
CFLAGS += -D_GNU_SOURCE

EXEC := printaddrinfo

OBJS := 
OBJS += main.o
OBJS += printaddrinfo.o

all:	$(OBJS)
	gcc -o $(EXEC) $(OBJS)

clean:
	rm -f $(EXEC) $(OBJS)
