VERSION = 0.02
CFLAGS  = -Wall -O2 # -g -D_REENTRANT -DVERSION=\"$(VERSION)\"
LDFLAGS = -lm
OUT = $(shell uname -m)

CC      = /usr/bin/g++




OBJ = tdt.o parser.o 

all: $(OBJ)
	$(CC) $(CFLAGS) -o tdt.$(OUT) $(OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o *.$(OUT)
