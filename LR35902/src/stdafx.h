#ifdef _MSC_VER
#pragma once
#endif

#include <cstdint>
#include <stdexcept>

#include <functional>

#include <vector>
#include <array>
#include <map>
#include <bitset>
#include <string>
#include <tuple>
#include <cassert>

#include <algorithm>
#include <memory>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <boost/format.hpp>

#include <Memory.h>
#include <Processor.h>
#include <IntelProcessor.h>
#include <Register.h>
#include <Signal.h>
#include <Ram.h>
#include <Bus.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
