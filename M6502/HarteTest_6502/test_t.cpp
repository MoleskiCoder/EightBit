#include "stdafx.h"
#include "test_t.h"
#include <cassert>

test_t::test_t(const simdjson::dom::element serialised)
: m_name(serialised["name"]),
  m_initial_state(serialised["initial"]),
  m_final_state(serialised["final"]),
  m_cycles(serialised["cycles"].get_array()) {}
