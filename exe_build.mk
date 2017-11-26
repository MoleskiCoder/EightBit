CXXFLAGS += -g -Wall -std=c++14 -pipe

CXXOBJECTS = $(CXXFILES:.cpp=.o)

SOURCES = $(CXXFILES)
OBJECTS = $(CXXOBJECTS)

PCH = stdafx.h.gch

.PHONY: all
all: opt

.PHONY: opt
opt: CXXFLAGS += -DNDEBUG -march=native -O3 -flto
opt: $(EXE)

.PHONY: debug
debug: CXXFLAGS += -D_DEBUG
debug: $(EXE)

.PHONY: coverage
coverage: CXXFLAGS += -D_DEBUG -fprofile-arcs -ftest-coverage
coverage: LDFLAGS += -lgcov
coverage: $(EXE)

$(PCH): stdafx.h
	$(CXX) $(CXXFLAGS) -x c++-header $<

$(EXE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXE) $(LDFLAGS)

%.o: %.cpp $(PCH)
	$(CXX) $(CXXFLAGS) $< -c -o $@

.PHONY: clean
clean:
	-rm -f $(EXE) $(OBJECTS) $(PCH) *.gcov *.gcda *.gcno
