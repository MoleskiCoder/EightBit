#include "stdafx.h"
#include "state_t.h"

state_t::state_t(const simdjson::dom::element serialised)
: m_pc((uint16_t)(int64_t)serialised["pc"]),
  m_s((uint8_t)(int64_t)serialised["s"]),
  m_a((uint8_t)(int64_t)serialised["a"]),
  m_x((uint8_t)(int64_t)serialised["x"]),
  m_y((uint8_t)(int64_t)serialised["y"]),
  m_p((uint8_t)(int64_t)serialised["p"]),
  m_ram(serialised["ram"].get_array()) {}
