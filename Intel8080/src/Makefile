LIB = libintel8080.a

CXXFLAGS = -Wall -std=c++14 -pipe -I ../inc -I ../../inc

CXXFILES = Disassembler.cpp Intel8080.cpp Profiler.cpp

CXXOBJECTS = $(CXXFILES:.cpp=.o)

SOURCES = $(CXXFILES)
OBJECTS = $(CXXOBJECTS)

PCH = stdafx.h.gch 

all: opt

opt: CXXFLAGS += -DNDEBUG -march=native -O3
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
