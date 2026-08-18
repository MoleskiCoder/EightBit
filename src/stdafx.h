#ifdef _MSC_VER
#pragma once
#endif

#include <cassert>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>

#include <sstream>
#include <iostream>  
#include <fstream>
#include <iomanip>
#include <ios>
#include <chrono>
#include <coroutine>
#include <limits>

#include <string>
#include <array>
#include <vector>
#include <map>

#ifdef _MSC_VER
#	include <intrin.h>
#endif

#ifdef __GNUG__
#	include <x86intrin.h>
#endif

#if !(defined(_MSC_VER) || defined(__GNUG__))
#	include <bitset>
#endif
