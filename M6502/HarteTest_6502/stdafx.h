#pragma once

#include <cassert>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <Bus.h>
#include <Ram.h>
#include <mos6502.h>
#include <Disassembly.h>
#include <Symbols.h>

#define USE_COROUTINES

#ifdef USE_COROUTINES
#if __cplusplus >= 202002L
#	include <co_generator_t.h>
#else
#	include <boost/coroutine2/all.hpp>
#	include <boost/bind.hpp>
#endif
#endif

#include "simdjson/simdjson.h"
