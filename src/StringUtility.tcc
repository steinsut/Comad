#ifndef COMAD_STRING_UTILITY_TCC_
#define COMAD_STRING_UTILITY_TCC_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <stdexcept>
#include <string_view>

#include "StringUtility.h"

namespace comad::utility {
	constexpr bool HasWhitespace(std::string_view str) {
		using namespace std::string_view_literals;

		for (char c : str) {
			if (" \t\n\f\r\v"sv.find_first_of(c) != std::string_view::npos) {
				return true;
			}
		}
		return false;
	}

	constexpr std::string_view CStringToStringView(const char* c_str, std::size_t max_size) {
		if (std::is_constant_evaluated()) {
			std::size_t size = 0;
			bool found = false;
			for (std::size_t i = 0; size < max_size; ++size) {
				if (c_str[i] == '\0') {
					found = true;
					break;
				}
			}

			if (found) {
				return std::string_view{ c_str, size };
			}
			else {
				throw std::out_of_range{ "c-string is either too big or missing the null terminator" };
			}
		}
		else {
			const char* end = (const char*)std::memchr(c_str, 0, max_size);
			if (end != nullptr) {
				return std::string_view{ c_str, (std::uintptr_t)end - (std::uintptr_t)c_str };
			}
			else {
				throw std::out_of_range{ "c-string is either too big or missing the null terminator" };
			}
		}
	}
}

#endif