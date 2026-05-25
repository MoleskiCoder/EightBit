#pragma once

#include <vector>
#include <functional>
#include "EventArgs.h"

namespace EightBit {
	template<class T> class Signal final {
	private:
		typedef std::function<void(T&)> delegate_t;
		typedef std::vector<delegate_t> delegates_t;

		delegates_t m_delegates;

	public:
		[[nodiscard]] constexpr auto count() const noexcept { return m_delegates.size(); }
		[[nodiscard]] constexpr auto inactive() const noexcept { return count() == 0; }
		[[nodiscard]] constexpr auto singular() const noexcept { return count() == 1; }
		[[nodiscard]] constexpr auto active() const noexcept { return count() != 0; }

		constexpr void connect(const delegate_t functor) {
			m_delegates.push_back(functor);
		}

		void fire(T& e = EventArgs::empty()) const noexcept {
			for (auto& delegate : m_delegates)
				delegate(e);
		}
	};
}
