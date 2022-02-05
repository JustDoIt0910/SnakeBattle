TARGET = Server

INCLUDE = /usr/local/src/libevent-2.1.12-stable/include
LIB = /usr/local/lib/
OBJS = main.o listener.o protocol.o user.o server.o snake.o error.o object.o
VPATH = server protocol user objects

$(TARGET): $(OBJS)
	g++ -o $(TARGET) $(OBJS) -I $(INCLUDE) -L $(LIB) -levent -lpthread

main.o: main.cc
listener.o: listener.h listener.cc protocol.h
protocol.o: protocol.h protocol.cc
server.o: server.h server.cc user.h object.h error.h
snake.o: object.h snake.h snake.cc
object.o: object.h object.cc
user.o: user.h user.cc
error.o: error.h error.cc

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)