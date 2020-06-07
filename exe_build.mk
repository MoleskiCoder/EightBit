LDFLAGS += -g

LDFLAGS_OPT = -flto
LDFLAGS_COVERAGE = -lgcov
LDFLAGS_PROFILE = $(LDFLAGS_COVERAGE) $(LDFLAGS_OPT)

opt: LDFLAGS += $(LDFLAGS_OPT)
opt: $(EXE)
debug: $(EXE)
coverage: LDFLAGS += $(LDFLAGS_COVERAGE)
coverage: $(EXE)
profile: LDFLAGS += $(LDFLAGS_PROFILE)
profile: $(EXE)
profiled: $(EXE)

$(EXE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXE) $(LDFLAGS)

.PHONY: clean
clean:
	-rm -f $(EXE) $(OBJECTS) $(PCH) *.gcov *.gcda *.gcno
