.PHONY: all
all: opt

opt:
	$(MAKE) -C src opt
	$(MAKE) -C Z80/src opt
	$(MAKE) -C Z80/test opt

debug:
	$(MAKE) -C src debug
	$(MAKE) -C Z80/src debug
	$(MAKE) -C Z80/test debug

coverage:
	$(MAKE) -C src coverage
	$(MAKE) -C Z80/src coverage
	$(MAKE) -C Z80/test coverage

.PHONY: clean
clean:
	$(MAKE) -C src clean
	$(MAKE) -C Z80/src clean
	$(MAKE) -C Z80/test clean
