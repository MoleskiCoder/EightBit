#include "stdafx.h"

#include "FuseTestSuite.h"

int main(int, char*[]) {

	Fuse::TestSuite testSuite("C:\\github\\cpp\\EightBit\\Z80\\fusetest_Z80\\fuse-tests\\tests");
	testSuite.run();

	return 0;
}
