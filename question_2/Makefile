SRC := question_2.c
TARGET = question_2

CC=gcc
LDFLAGS= -pthread  -lpthread

all: question_2

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	-rm -f *.o $(TARGET) *.elf *.ma