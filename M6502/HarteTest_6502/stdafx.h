#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

//#define TEST_JSON_PERFORMANCE

#define USE_SIMDJSON_JSON	// 15 seconds
//#define USE_BOOST_JSON	// 32 seconds
//#define USE_NLOHMANN_JSON	// 58 seconds
//#define USE_JSONCPP_JSON	// 88 seconds

#ifdef USE_BOOST_JSON
#	include <boost/json.hpp>
#endif

#ifdef USE_NLOHMANN_JSON
#	include "nlohmann/json.hpp"
#endif

#ifdef USE_JSONCPP_JSON
#	include <json/json.h>
#endif

#ifdef USE_SIMDJSON_JSON
#	define JSON_PREFER_PASS_BY_VALUE
#	include "simdjson/simdjson.h"
#endif
