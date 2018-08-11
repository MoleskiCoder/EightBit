#include "stdafx.h"
#include "Configuration.h"

Configuration::Configuration() noexcept
:	m_debugMode(false),
	m_profileMode(false),
	m_romDirectory("roms") {
}
