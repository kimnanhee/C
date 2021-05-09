CC = gcc
TARGET = 20
LIBS = -lpthread

$(TARGET) : main.o inter.o network.o
	$(CC) -o $(TARGET) main.o inter.o network.o $(LIBS)
	rm *.o

main.o : main.c

inter.o : inter.c

network.o : network.c

clean :
	rm $(TARGET) *.o
