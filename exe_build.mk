CXXOBJECTS = $(CXXFILES:.cpp=.o)

SOURCES = $(CXXFILES)
OBJECTS = $(CXXOBJECTS)

PCH = stdafx.h.gch

all: opt

opt: CXXFLAGS += $(CXXFLAGS_OPT)
opt: $(EXE)

debug: CXXFLAGS += $(CXXFLAGS_DEBUG)
debug: $(EXE)

coverage: CXXFLAGS += $(CXXFLAGS_COVERAGE)
coverage: LDFLAGS += -lgcov
coverage: $(EXE)

profile: CXXFLAGS += $(CXXFLAGS_PROFILE)
profile: LDFLAGS += -lgcov
profile: $(EXE)

profiled: CXXFLAGS += $(CXXFLAGS_PROFILED)
profiled: $(EXE)

$(PCH): stdafx.h
	$(CXX) $(CXXFLAGS) -x c++-header $<

$(EXE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXE) $(LDFLAGS)

%.o: %.cpp $(PCH)
	$(CXX) $(CXXFLAGS) $< -c -o $@

.PHONY: clean
clean:
	-rm -f $(EXE) $(OBJECTS) $(PCH) *.gcov *.gcda *.gcno
