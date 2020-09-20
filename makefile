CC = g++

INCLUDES =
CPPFLAGS = -Wall -O3 -std=c++2a -flto $(INCLUDES)
LDLIBS =

SOURCES_CPP = $(wildcard *.cpp)
SOURCES_C = buse/buse.c

OBJECTS = $(SOURCES_CPP:%.cpp=%.o) $(SOURCES_C:%.c=%.o)

APP_NAME = chunkbd
APP = $(APP_NAME)

all: $(APP)
	strip $(APP)

debug: CPPFLAGS = -Wall -g3 -std=c++2a $(INCLUDES)
debug: $(OBJECTS)
	$(LINK.o) $^ $(LDLIBS) -o $(APP)

clean:
	rm -f $(APP) $(OBJECTS)

$(APP): $(OBJECTS)
	$(LINK.o) -flto $^ $(LDLIBS) -o $(APP)
