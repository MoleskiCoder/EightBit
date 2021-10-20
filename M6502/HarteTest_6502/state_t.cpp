#include "stdafx.h"
#include "state_t.h"

state_t::state_t(const simdjson::dom::element input)
: m_raw(input) {}
