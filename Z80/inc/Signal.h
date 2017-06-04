#pragma once

#include <vector>
#include <functional>

template<class T> class Signal {
private:
	typedef std::function<void(T&)> delegate_t;
	typedef std::vector<delegate_t> delegates_t;

	delegates_t delegates;

public:
	void connect(delegate_t functor) {
		delegates.push_back(functor);
	}

	void fire(T& e) const {
		if (!delegates.empty())
			for (auto& delegate : delegates)
				delegate(e);
	}
};
