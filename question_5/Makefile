SRC := question_5.c
TARGET = question_5

CC=gcc
LDFLAGS= -pthread  -lpthread

all: question_5

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	-rm -f *.o $(TARGET) *.elf *.ma