#include "stdafx.h"

#include "FuseTestSuite.h"

int main(int, char*[]) {

	Fuse::TestSuite testSuite("C:\\github\\cpp\\invaders\\fuse-tests\\tests");
	testSuite.run();

	return 0;
}
