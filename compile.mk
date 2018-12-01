CXXFLAGS += -g -Wall -std=c++17 -pipe

CXXFLAGS_OPT = -DNDEBUG -march=native -Ofast -flto
CXXFLAGS_DEBUG = -D_DEBUG
CXXFLAGS_COVERAGE = $(CXXFLAGS_DEBUG) -fprofile-arcs -ftest-coverage

CXXFLAGS_PROFILE = $(CXXFLAGS_OPT) -fprofile-generate
CXXFLAGS_PROFILED = $(CXXFLAGS_OPT) -fprofile-use

CXXOBJECTS = $(CXXFILES:.cpp=.o)

SOURCES = $(CXXFILES)
OBJECTS = $(CXXOBJECTS)

PCH = stdafx.h.gch

all: opt

opt: CXXFLAGS += $(CXXFLAGS_OPT)
debug: CXXFLAGS += $(CXXFLAGS_DEBUG)
coverage: CXXFLAGS += $(CXXFLAGS_COVERAGE)
profile: CXXFLAGS += $(CXXFLAGS_PROFILE)
profiled: CXXFLAGS += $(CXXFLAGS_PROFILED)

$(PCH): stdafx.h
	$(CXX) $(CXXFLAGS) -x c++-header $<

%.o: %.cpp $(PCH)
	$(CXX) $(CXXFLAGS) $< -c -o $@
