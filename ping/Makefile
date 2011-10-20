CFLAGS += -std=c99
CFLAGS += -g
CFLAGS += -D_GNU_SOURCE

EXEC := ping

OBJS := 
OBJS += main.o
OBJS += ping.o

all:	$(OBJS)
	gcc -o $(EXEC) $(OBJS)

clean:
	rm -f $(EXEC) $(OBJS)
