CC := g++
CFLAGS := -Wall -g -std=c++17
LDFLAGS := -pthread
TARGET := a.out
FLAGS := ""

SRCS := $(wildcard src/*.cpp)
OBJS := $(patsubst src/%.cpp,obj/%.o,$(SRCS))

all: $(TARGET)

bench: CFLAGS += -O3
bench: clean
bench: all

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

obj/%.o: src/%.cpp src/*.hpp
	mkdir ./obj -p
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) obj/*.o
	
.PHONY: all clean
