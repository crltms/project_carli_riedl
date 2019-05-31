TARGET	= plotter
CC	= gcc
RM	= rm -rf
CFLAGS	= `pkg-config --cflags --libs gtk+-3.0` --pedantic -Wall -g
CFLAGS  += -std=c99
CFLAGS  += -std=gnu99
INCPATH	= -I./inc/
SRC	= ./src/*.c
OBJS	= $(SRC:.c=.o)
LIBS	=-L/usr/lib/x86_64-linux-gnu  -lm
LIBS    += -lusb-1.0


all: $(OBJS) $(TARGET)

%.o: %.c
	$(CC) $(SRC) $(LIBS) $(INCPATH) $(CFLAGS) -o $(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET) core *~
