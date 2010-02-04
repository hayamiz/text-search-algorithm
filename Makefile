
TARGET = text-search
OBJS = text-search.o

CC ?= gcc
CFLAGS ?= -g -Wall
CFLAGS += $(shell pkg-config --cflags glib-2.0)
LDFLAGS += $(shell pkg-config --libs glib-2.0)

.PHONY: clean all

all: $(TARGET)

clean:
	rm -f *.o $(TARGET)

*.o: *.h

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

