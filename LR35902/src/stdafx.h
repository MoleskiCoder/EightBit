#pragma once

#include <cstdint>
#include <stdexcept>

#include <functional>

#include <vector>
#include <array>
#include <map>

#include <bitset>
#include <string>

#include <algorithm>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <boost/format.hpp>

#include <SDL.h>

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define HOST_LITTLE_ENDIAN
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define HOST_BIG_ENDIAN
#endif

#ifdef _MSC_VER
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#endif
