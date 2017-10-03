LIB = liblr35902.a

CXXFLAGS = -Wall -std=c++11 -pipe -I ../inc -I ../../inc

CXXFILES = CharacterDefinition.cpp Disassembler.cpp Display.cpp GameBoyBus.cpp LR35902.cpp Profiler.cpp

CXXOBJECTS = $(CXXFILES:.cpp=.o)

SOURCES = $(CXXFILES)
OBJECTS = $(CXXOBJECTS)

PCH = stdafx.h.gch 

all: opt

opt: CXXFLAGS += -DNDEBUG -march=native -O2
opt: $(LIB)

debug: CXXFLAGS += -g -D_DEBUG
debug: $(LIB)

coverage: CXXFLAGS += -g -D_DEBUG -fprofile-arcs -ftest-coverage
coverage: $(LIB)

$(PCH): stdafx.h
	$(CXX) $(CXXFLAGS) -x c++-header $<

$(LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIB) $(OBJECTS)

%.o: %.cpp $(PCH)
	$(CXX) $(CXXFLAGS) $< -c -o $@

.PHONY: clean
clean:
	-rm -f $(LIB) $(OBJECTS) $(PCH)
