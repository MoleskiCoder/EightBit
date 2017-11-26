CXXFLAGS += -g -Wall -std=c++14 -pipe

CXXOBJECTS = $(CXXFILES:.cpp=.o)

SOURCES = $(CXXFILES)
OBJECTS = $(CXXOBJECTS)

PCH = stdafx.h.gch

.PHONY: all
all: opt

.PHONY: opt
opt: CXXFLAGS += -DNDEBUG -march=native -O3 -flto
opt: $(LIB)

.PHONY: debug
debug: CXXFLAGS += -D_DEBUG
debug: $(LIB)

.PHONY: coverage
coverage: CXXFLAGS += -D_DEBUG -fprofile-arcs -ftest-coverage
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
