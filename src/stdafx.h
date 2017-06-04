#ifdef _MSC_VER
#pragma once
#endif

#include <cstdint>
#include <functional>

#include <iostream>
#include <fstream>

#include <string>
#include <array>
#include <vector>

#if defined(_M_X64) || defined(_M_IX86 )
#	define HOST_LITTLE_ENDIAN
#else
#	define HOST_BIG_ENDIAN
#endif
