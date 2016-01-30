C_SRCS = $(shell find -name '*.cpp')
A_SRCS = $(shell find -name '*.S')
C_HDRS = $(shell find -name '*.h')
SRCS = $(C_SRCS) $(A_SRCS)
OBJS = $(addsuffix .o,$(basename $(SRCS)))

INCLUDE = -I./include

AS = g++
CC = g++
LD = g++
CFLAGS = -std=c++11 $(INCLUDE)
ASMFLAGS =
LDFLAGS =

BIN = vm

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(AS) $(ASMFLAGS) -c -o $@ $<

style: $(C_SRCS) $(C_HDRS)
	astyle --style=allman --indent-classes --indent-switches $^

clean:
	rm $(OBJS)
	rm $(BIN)

