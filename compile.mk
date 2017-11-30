CXXFLAGS += -g -Wall -std=c++14 -pipe

#CXXFLAGS_OPT = -DNDEBUG -march=native -Ofast -flto
CXXFLAGS_OPT = -DNDEBUG -march=native -Ofast
CXXFLAGS_DEBUG = -D_DEBUG
CXXFLAGS_COVERAGE = $(CXXFLAGS_DEBUG) -fprofile-arcs -ftest-coverage

CXXFLAGS_PROFILE = $(CXXFLAGS_OPT) -fprofile-generate
CXXFLAGS_PROFILED = $(CXXFLAGS_OPT) -fprofile-use
