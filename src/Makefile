CXXFLAGS = -Wall -std=c++11 -pipe -I ../inc

CXXFILES = EventArgs.cpp InputOutput.cpp IntelProcessor.cpp Memory.cpp Processor.cpp

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

