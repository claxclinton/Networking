CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -std=c99
CFLAGS += -g
CFLAGS += -D_GNU_SOURCE

EXEC := pingserver

OBJS := 
OBJS += main.o
OBJS += pingserver.o

all:	$(OBJS)
	gcc -o $(EXEC) $(OBJS)

clean:
	rm -f $(EXEC) $(OBJS)
