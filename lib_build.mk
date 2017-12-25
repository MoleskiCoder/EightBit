opt: $(LIB)
debug: $(LIB)
coverage: $(LIB)
profile: $(LIB)
profiled: $(LIB)

$(LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIB) $(OBJECTS)

.PHONY: clean
clean:
	-rm -f $(LIB) $(OBJECTS) $(PCH)
