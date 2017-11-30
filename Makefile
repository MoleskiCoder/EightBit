.PHONY: all
all: opt

.PHONY: opt
opt:
	$(MAKE) -C src opt
	$(MAKE) -C Z80/src opt
	$(MAKE) -C Z80/test opt

.PHONY: debug
debug:
	$(MAKE) -C src debug
	$(MAKE) -C Z80/src debug
	$(MAKE) -C Z80/test debug

.PHONY: coverage
coverage:
	$(MAKE) -C src coverage
	$(MAKE) -C Z80/src coverage
	$(MAKE) -C Z80/test coverage

.PHONY: profile
profile:
	$(MAKE) -C src profile
	$(MAKE) -C Z80/src profile
	$(MAKE) -C Z80/test profile

.PHONY: profiled
profiled:
	$(MAKE) -C src profiled
	$(MAKE) -C Z80/src profiled
	$(MAKE) -C Z80/test profiled

.PHONY: clean
clean:
	$(MAKE) -C src clean
	$(MAKE) -C Z80/src clean
	$(MAKE) -C Z80/test clean
