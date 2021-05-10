CC = gcc
TARGET = procInfoClient
LIBS = -lpthread

$(TARGET) : main.o inter.o network.o logger.o
	$(CC) -o $(TARGET) main.o inter.o network.o logger.o $(LIBS)

main.o : main.c

inter.o : inter.c

network.o : network.c

logger.o : logger.c

clean :
	rm $(TARGET) *.o
