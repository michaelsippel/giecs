INCLUDE = -I./include

ifeq (,$(TARGET))
TARGET = linux64
endif

ifeq ($(TARGET),linux32)
AS = $(CXX) -m32
CC = $(CXX) -m32
LD = $(CXX) -m32
BIN = vm_linux32
else ifeq ($(TARGET),linux64)
AS = $(CXX) -m64
CC = $(CXX) -m64
LD = $(CXX) -m64
BIN = vm_linux64
else ifeq ($(TARGET),win32)
AS = i686-w64-mingw32-g++
CC = i686-w64-mingw32-g++
LD = i686-w64-mingw32-g++
BIN = vm_win32.exe
else ifeq ($(TARGET),win64)
AS = x86_64-w64-mingw32-g++
CC = x86_64-w64-mingw32-g++
LD = x86_64-w64-mingw32-g++
BIN = vm_win64.exe
endif

CFLAGS = -std=c++11 $(INCLUDE)
ASMFLAGS =
LDFLAGS =

LIB_SRCS = $(shell find ./src -name '*.cpp')
TEST_SRCS = $(shell find ./test -name '*.cpp')
REPL_SRCS = $(shell find ./repl -name '*.cpp')
LIB_OBJS = $(addsuffix .$(TARGET).o,$(basename $(LIB_SRCS)))
TEST_OBJS = $(addsuffix .$(TARGET).o,$(basename $(TEST_SRCS)))
REPL_OBJS = $(addsuffix .$(TARGET).o,$(basename $(REPL_SRCS)))
C_SRCS = $(shell find -name '*.cpp')
C_HDRS = $(shell find -name '*.h')

LIB = giecs.a


$(BIN): $(REPL_OBJS) $(LIB)
	$(LD) -o $@ $^ $(LDFLAGS)

$(LIB): $(LIB_OBJS)
	ar -rcs $@ $^

unit_test: $(TEST_OBJS) $(LIB)
	$(LD) -o $@ $^ -lboost_unit_test_framework
	./$@
	rm $@

%.$(TARGET).o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

%.$(TARGET).o: %.S
	$(AS) $(ASMFLAGS) -c -o $@ $<

style: $(C_SRCS) $(C_HDRS)
	astyle --style=allman --indent-classes --indent-switches $^

test: unit_test $(BIN)
	cd examples && ./test.lisp

clean:
	rm $(LIB_OBJS)
	rm $(TEST_OBJS)
	rm $(REPL_OBJS)
	rm $(LIB)

