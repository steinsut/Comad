#ifndef COMAD_LITERALS_TCC_
#define COMAD_LITERALS_TCC_

#include "CommandLiterals.h"
#include "StringUtility.h"

namespace comad::literals {
	constexpr command::CommandFlag operator""_fl(const char* name, std::size_t size) {
		return command::CommandFlag{ name, size };
	}

	constexpr command::CommandArgument operator""_ab(const char* name, std::size_t size) {
		std::string_view str = utility::CStringToStringView(name, size + 1);
		if (str.empty()) {
			throw std::invalid_argument("argument name cannot be empty");
		}
		if (utility::HasWhitespace(str)) {
			throw std::invalid_argument("argument name cannot have whitespaces");
		}
		return command::CommandArgument{ std::string{ str }, value::ValueType::kBool };
	}
	constexpr command::CommandArgument operator""_ai(const char* name, std::size_t size) {
		std::string_view str = utility::CStringToStringView(name, size + 1);
		if (str.empty()) {
			throw std::invalid_argument("argument name cannot be empty");
		}
		if (utility::HasWhitespace(str)) {
			throw std::invalid_argument("argument name cannot have whitespaces");
		}
		return command::CommandArgument{ std::string{ str }, value::ValueType::kInt };
	}
	constexpr command::CommandArgument operator""_af(const char* name, std::size_t size) {
		std::string_view str = utility::CStringToStringView(name, size + 1);
		if (str.empty()) {
			throw std::invalid_argument("argument name cannot be empty");
		}
		if (utility::HasWhitespace(str)) {
			throw std::invalid_argument("argument name cannot have whitespaces");
		}
		return command::CommandArgument{ std::string{ str }, value::ValueType::kFloat };
	}
	constexpr command::CommandArgument operator""_as(const char* name, std::size_t size) {
		std::string_view str = utility::CStringToStringView(name, size + 1);
		if (str.empty()) {
			throw std::invalid_argument("argument name cannot be empty");
		}
		if (utility::HasWhitespace(str)) {
			throw std::invalid_argument("argument name cannot have whitespaces");
		}
		return command::CommandArgument{ std::string{ str }, value::ValueType::kString };
	}

	template <typename... TArgs>
	CommandOptionLiteral& CommandOptionLiteral::operator()(TArgs... args) {
		option_.short_name = name_[0];
		option_.operator()(std::forward<TArgs>(args)...);

		return *this;
	}

	constexpr CommandOptionLiteral::operator std::pair<std::string_view, command::CommandOption>() const {
		return { name_, option_ };
	}

	constexpr CommandOptionLiteral::CommandOptionLiteral(std::string_view name) : name_{ name } {};

	constexpr CommandOptionLiteral operator""_o(const char* name, std::size_t size) {
		return CommandOptionLiteral{ { name, size } };
	}
}

#endif