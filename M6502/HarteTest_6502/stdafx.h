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

#define USE_BOOST_JSON
//#define USE_NLOHMANN_JSON
//#define USE_JSONCPP_JSON

#ifdef USE_BOOST_JSON
#	include <boost/json.hpp>
#endif

#ifdef USE_NLOHMANN_JSON
#	include "nlohmann/json.hpp"
#endif

#ifdef USE_JSONCPP_JSON
#	include <json/json.h>
#endif
