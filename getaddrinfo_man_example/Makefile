CFLAGS += -g
CFLAGS += -D_GNU_SOURCE

EXEC_SERVER := server
EXEC_CLIENT := client

OBJS := 
OBJS += server.o
OBJS += client.o

all:	$(OBJS)
	gcc -o $(EXEC_SERVER) server.o
	gcc -o $(EXEC_CLIENT) client.o

clean:
	rm -f $(EXEC_SERVER) $(EXEC_CLIENT) $(OBJS)
