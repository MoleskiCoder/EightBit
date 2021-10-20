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
#include <utility>
#include <vector>

#include <Bus.h>
#include <Ram.h>
#include <mos6502.h>
#include <Disassembly.h>
#include <Symbols.h>

//#define TEST_JSON_PERFORMANCE

#define USE_SIMDJSON_JSON	// 13 seconds (19)
//#define USE_RAPIDJSON_JSON	// 19 seconds (22)
//#define USE_BOOST_JSON	// 26 seconds (32)
//#define USE_NLOHMANN_JSON	// 56 seconds (65)
//#define USE_JSONCPP_JSON	// 80 seconds (87)

#ifdef USE_SIMDJSON_JSON
#	define JSON_PREFER_PASS_BY_VALUE
#	define JSON_PREFER_REUSE_OF_PARSER
#	define JSON_PREFER_ITERATOR_TO_INDEX
#	define JSON_ACCESS_DATA_VIA_OBJECT
#	include "simdjson/simdjson.h"
#endif

#ifdef USE_RAPIDJSON_JSON
#	define RAPIDJSON_HAS_STDSTRING	1
#	define RAPIDJSON_SSE42
#	define JSON_INSITU_PARSE
#	define JSON_ACCESS_DATA_VIA_OBJECT
#   include "rapidjson/document.h"
#endif

#ifdef USE_BOOST_JSON
#	define JSON_ACCESS_DATA_VIA_OBJECT
#	include <boost/json.hpp>
#endif

#ifdef USE_NLOHMANN_JSON
#	define JSON_USE_IMPLICIT_CONVERSIONS 0
#	define JSON_ACCESS_DATA_VIA_OBJECT
#	include "nlohmann/json.hpp"
#endif

#ifdef USE_JSONCPP_JSON
#	define JSON_PREFER_REUSE_OF_PARSER
#	define JSON_ACCESS_DATA_VIA_OBJECT
#	include <json/json.h>
#endif
