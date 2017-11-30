CXXOBJECTS = $(CXXFILES:.cpp=.o)

SOURCES = $(CXXFILES)
OBJECTS = $(CXXOBJECTS)

PCH = stdafx.h.gch

all: opt

opt: CXXFLAGS += $(CXXFLAGS_OPT)
opt: $(LIB)

debug: CXXFLAGS += $(CXXFLAGS_DEBUG)
debug: $(LIB)

coverage: CXXFLAGS += $(CXXFLAGS_COVERAGE)
coverage: $(LIB)

profile: CXXFLAGS += $(CXXFLAGS_PROFILE)
profile: $(LIB)

profiled: CXXFLAGS += $(CXXFLAGS_PROFILED)
profiled: $(LIB)

$(PCH): stdafx.h
	$(CXX) $(CXXFLAGS) -x c++-header $<

$(LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIB) $(OBJECTS)

%.o: %.cpp $(PCH)
	$(CXX) $(CXXFLAGS) $< -c -o $@

.PHONY: clean
clean:
	-rm -f $(LIB) $(OBJECTS) $(PCH)
