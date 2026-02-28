#pragma once

#include "simdjson/simdjson.h"

class array_t {
private:
	const simdjson::dom::array m_raw;

protected:
	[[nodiscard]] simdjson::dom::array raw() const noexcept { return m_raw; }

public:
	[[nodiscard]] auto begin() const noexcept { return raw().begin(); }
	[[nodiscard]] auto end() const noexcept { return raw().end(); }
	[[nodiscard]] auto size() const noexcept { return raw().size(); }

	[[nodiscard]] auto at(size_t idx) const noexcept { return raw().at(idx); }
	[[nodiscard]] auto operator[](size_t idx) const noexcept { return at(idx); }

protected:

	[[nodiscard]] std::optional<int64_t> maybe_integer_at(size_t idx) const noexcept {
		auto possible = at(idx);
		if (possible.has_value() && possible.is_int64())
			return std::optional<int64_t>(possible.get_int64());
		return std::optional<int64_t>();
	}

	[[nodiscard]] std::optional<std::string_view> maybe_string_at(size_t idx) const noexcept {
		auto possible = at(idx);
		if (possible.has_value() && possible.is_string())
			return std::optional<std::string_view>(possible.get_string());
		return std::optional<std::string_view>();
	}

	template<typename T>
	[[nodiscard]] auto maybe_cast_integer_at(size_t idx) const noexcept {
		auto original = maybe_integer_at(idx);
		if (original.has_value())
			return std::optional<T>(T(original.value()));
		return std::optional<T>();
	}

	[[nodiscard]] auto integer_at(size_t idx) const noexcept { return maybe_integer_at(idx).value(); }
	[[nodiscard]] auto string_at(size_t idx) const noexcept { return maybe_string_at(idx).value(); }

public:
	array_t(const simdjson::dom::array input) noexcept
	: m_raw(input) {}

	[[nodiscard]] auto maybe_address_at(size_t idx) const noexcept {
		return maybe_cast_integer_at<uint16_t>(idx);
	}

	[[nodiscard]] auto maybe_byte_at(size_t idx) const noexcept {
		return maybe_cast_integer_at<uint8_t>(idx);
	}

	[[nodiscard]] auto address_at(size_t idx) const noexcept { return maybe_address_at(idx).value(); }
	[[nodiscard]] auto byte_at(size_t idx) const noexcept { return maybe_byte_at(idx).value(); }
};
