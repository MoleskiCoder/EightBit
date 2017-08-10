#include "stdafx.h"

#include "FuseTestSuite.h"

int main(int, char*[]) {

	Fuse::TestSuite testSuite("C:\\github\\cpp\\EightBitWrapper\\libraries\\EightBit\\LR35902\\fusetest_LR35902\\fuse-tests\\tests");
	testSuite.run();

	return 0;
}
