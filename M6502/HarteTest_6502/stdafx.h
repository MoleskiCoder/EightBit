#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <map>
#include <vector>

//#define TEST_JSON_PERFORMANCE

#define USE_SIMDJSON_JSON	// 14 seconds
//#define USE_BOOST_JSON	// 28 seconds
//#define USE_NLOHMANN_JSON	// 56 seconds
//#define USE_JSONCPP_JSON	// 97 seconds

#ifdef USE_SIMDJSON_JSON
#	define JSON_PREFER_PASS_BY_VALUE
#	define JSON_PREFER_REUSE_OF_PARSER
#	include "simdjson/simdjson.h"
#endif

#ifdef USE_BOOST_JSON
#	include <boost/json.hpp>
#endif

#ifdef USE_NLOHMANN_JSON
#	include "nlohmann/json.hpp"
#endif

#ifdef USE_JSONCPP_JSON
#	define JSON_PREFER_REUSE_OF_PARSER
#	include <json/json.h>
#endif
