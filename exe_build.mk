LDFLAGS += -g

opt: $(EXE)
debug: $(EXE)
coverage: LDFLAGS += -lgcov
coverage: $(EXE)
profile: LDFLAGS += -lgcov
profile: $(EXE)
profiled: $(EXE)

$(EXE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXE) $(LDFLAGS)

.PHONY: clean
clean:
	-rm -f $(EXE) $(OBJECTS) $(PCH) *.gcov *.gcda *.gcno
