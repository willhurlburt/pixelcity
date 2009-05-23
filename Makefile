all: pixelcity

SRCS := $(wildcard *.cpp *.c)

DEPS_TMP := $(SRCS:.cpp=.d)
DEPS := $(DEPS_TMP:.c=.d)

OBJS := $(DEPS:.d=.o)

# automatic dependencies: include them all so GNU make will remake them,
-include $(DEPS)

# ...and provide rules to re-make them via gcc/g++
%.d: %.c
	@$(CC) -MM -MP -MF $@ $(CPPFLAGS) $<

%.d: %.cpp
	@$(CXX) -MM -MP -MF $@ $(CPPFLAGS) $<

pixelcity: $(OBJS)
	g++ -o $@ -g $(OBJS) $(LDFLAGS) -lm -lX11 -lGL -lGLU

CFLAGS=-Wall -pedantic -D_FILE_OFFSET_BITS=64
CXXFLAGS=-Wall -pedantic -D_FILE_OFFSET_BITS=64
CPPFLAGS=$(shell pkg-config --cflags fontconfig freetype2)
LDFLAGS=$(shell pkg-config --libs fontconfig freetype2)

ifeq ($(OPT), 0)
CFLAGS += -g -O0
CXXFLAGS += -g -O0
else
CFLAGS += -O3
CXXFLAGS += -O3
endif

clean:
	rm -f *.o pixelcity

realclean: clean
	rm -f *.d
