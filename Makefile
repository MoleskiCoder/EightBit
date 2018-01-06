.PHONY: all
all: opt

.PHONY: opt
opt:
	$(MAKE) -C src opt
	$(MAKE) -C Z80/src opt
	$(MAKE) -C Z80/test opt
	$(MAKE) -C Intel8080/src opt
	$(MAKE) -C Intel8080/test opt
	$(MAKE) -C M6502/src opt
	$(MAKE) -C M6502/test opt

.PHONY: debug
debug:
	$(MAKE) -C src debug
	$(MAKE) -C Z80/src debug
	$(MAKE) -C Z80/test debug
	$(MAKE) -C Intel8080/src debug
	$(MAKE) -C Intel8080/test debug
	$(MAKE) -C M6502/src debug
	$(MAKE) -C M6502/test debug

.PHONY: coverage
coverage:
	$(MAKE) -C src coverage
	$(MAKE) -C Z80/src coverage
	$(MAKE) -C Z80/test coverage
	$(MAKE) -C Intel8080/src coverage
	$(MAKE) -C Intel8080/test coverage
	$(MAKE) -C M6502/src coverage
	$(MAKE) -C M6502/test coverage

.PHONY: profile
profile:
	$(MAKE) -C src profile
	$(MAKE) -C Z80/src profile
	$(MAKE) -C Z80/test profile
	$(MAKE) -C Intel8080/src profile
	$(MAKE) -C Intel8080/test profile
	$(MAKE) -C M6502/src profile
	$(MAKE) -C M6502/test profile

.PHONY: profiled
profiled:
	$(MAKE) -C src profiled
	$(MAKE) -C Z80/src profiled
	$(MAKE) -C Z80/test profiled
	$(MAKE) -C Intel8080/src profiled
	$(MAKE) -C Intel8080/test profiled
	$(MAKE) -C M6502/src profiled
	$(MAKE) -C M6502/test profiled

.PHONY: clean
clean:
	$(MAKE) -C src clean
	$(MAKE) -C Z80/src clean
	$(MAKE) -C Z80/test clean
	$(MAKE) -C Intel8080/src clean
	$(MAKE) -C Intel8080/test clean
	$(MAKE) -C M6502/src clean
	$(MAKE) -C M6502/test clean
