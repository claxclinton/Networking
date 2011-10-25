CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -std=c99
CFLAGS += -g
CFLAGS += -D_GNU_SOURCE

EXEC := pingclient

OBJS := 
OBJS += main.o
OBJS += pingclient.o

all:	$(OBJS)
	gcc -o $(EXEC) $(OBJS)

clean:
	rm -f $(EXEC) $(OBJS)
