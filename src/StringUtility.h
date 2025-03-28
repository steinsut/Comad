#ifndef COMAD_STRING_UTILITY_H_
#define COMAD_STRING_UTILITY_H_

#include <string>
#include <string_view>

#include <ComadBuildOptions.h>

namespace comad::utility {
	constexpr bool HasWhitespace(std::string_view str);

	constexpr std::string_view CStringToStringView(const char* c_str, std::size_t max_size = comad::build_options::kMaxCStringLength);
}

#include "StringUtility.tcc"

#endif