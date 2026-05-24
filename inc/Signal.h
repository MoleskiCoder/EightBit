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

//
//
//#pragma once
//
//#include <cstring>
//#include <type_traits>
//#include <vector>
//#include "EventArgs.h"
//
//namespace EightBit {
//	template<class T> class Signal final {
//	private:
//		using fn_t = void(*)(void*, T&);
//		using dtor_t = void(*)(void*);
//
//		struct Entry {
//			void* ctx;
//			fn_t   fn;
//			dtor_t dtor; // null for inline-stored (trivially-destructible) callables
//		};
//
//		int m_count = 0;
//		std::vector<Entry> m_delegates;
//
//		void destroy_entries() noexcept {
//			for (auto& e : m_delegates)
//				if (e.dtor) e.dtor(e.ctx);
//			m_delegates.clear();
//			m_count = 0;
//		}
//
//	public:
//		Signal() = default;
//
//		// Copies are connection-less: a copied chip instance starts with no listeners
//		Signal(const Signal&) noexcept {}
//		Signal& operator=(const Signal&) noexcept { destroy_entries(); return *this; }
//
//		~Signal() { destroy_entries(); }
//
//		template<typename Callable>
//		void connect(Callable&& callable) {
//			using Decay = std::decay_t<Callable>;
//
//			// [this] lambdas on x64: sizeof == 8 == sizeof(void*), trivially copyable/destructible.
//			// Pack the captured pointer directly into the void* slot — zero heap overhead,
//			// and the dispatch function knows the concrete type so c(e) can be inlined.
//			constexpr bool fits_inline =
//				sizeof(Decay) <= sizeof(void*) &&
//				alignof(Decay) <= alignof(void*) &&
//				std::is_trivially_copyable_v<Decay> &&
//				std::is_trivially_destructible_v<Decay>;
//
//			if constexpr (fits_inline) {
//				void* storage = nullptr;
//				std::memcpy(&storage, &callable, sizeof(Decay));
//				m_delegates.push_back({
//					storage,
//					[](void* raw, T& e) noexcept {
//						// Reconstruct the callable from the packed pointer bits.
//						// Trivially-copyable, so memcpy into aligned storage is valid.
//						alignas(Decay) char buf[sizeof(Decay)];
//						std::memcpy(buf, &raw, sizeof(Decay));
//						(*reinterpret_cast<Decay*>(buf))(e);
//					},
//					nullptr
//					});
//			}
//			else {
//				auto* heap = new Decay(std::forward<Callable>(callable));
//				m_delegates.push_back({
//					static_cast<void*>(heap),
//					[](void* raw, T& e) noexcept {
//						(*static_cast<Decay*>(raw))(e);
//					},
//					[](void* raw) noexcept {
//						delete static_cast<Decay*>(raw);
//					}
//					});
//			}
//			++m_count;
//		}
//
//		[[nodiscard]] constexpr auto count() const noexcept { return m_count; }
//		[[nodiscard]] constexpr auto inactive() const noexcept { return count() == 0; }
//		[[nodiscard]] constexpr auto singular() const noexcept { return count() == 1; }
//		[[nodiscard]] constexpr auto active() const noexcept { return count() != 0; }
//
//		void fire(T& e = EventArgs::empty()) const noexcept {
//			//if (inactive()) return;
//			//if (singular()) {
//			//	m_delegates[0].fn(m_delegates[0].ctx, e);
//			//	return;
//			//}
//			for (const auto& entry : m_delegates)
//				entry.fn(entry.ctx, e);
//		}
//	};
//}
