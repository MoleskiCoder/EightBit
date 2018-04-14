#pragma once

#include <vector>
#include <functional>

namespace EightBit {
	template<class T> class Signal final {
	private:
		typedef std::function<void(const T&)> delegate_t;
		typedef std::vector<delegate_t> delegates_t;

		delegates_t m_delegates;

	public:
		void connect(const delegate_t functor) {
			m_delegates.push_back(functor);
		}

		void fire(const T& e) const {
			for (auto& delegate : m_delegates)
				delegate(e);
		}
	};
}
