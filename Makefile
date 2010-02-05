
TARGET = text-search
OBJS = text-search.o SFMT.o sauto.o poor.o

CC ?= gcc
CFLAGS ?= -g -Wall
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += -lm $(shell pkg-config --libs glib-2.0)
SFMT_CFLAGS = $(CFLAGS)
SFMT_CFLAGS += -msse2 -fno-strict-aliasing -DHAVE_SSE2=1 -DMEXP=19937

.PHONY: clean all

all: $(TARGET)

clean:
	rm -f *.o $(TARGET) $(OBJS)

*.o: *.h

SFMT.o: SFMT/SFMT.c
	$(CC) $(SFMT_CFLAGS) -c $^

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

