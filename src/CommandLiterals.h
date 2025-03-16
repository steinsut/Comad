#ifndef COMAD_LITERALS_H_
#define COMAD_LITERALS_H_

#include <cstddef>
#include <string_view>

#include "Command.h"

namespace comad::literals {
	constexpr command::CommandFlag operator""_fl(const char* name, std::size_t size);

	constexpr command::CommandArgument operator""_ab(const char* name, std::size_t size);
	constexpr command::CommandArgument operator""_ai(const char* name, std::size_t size);
	constexpr command::CommandArgument operator""_af(const char* name, std::size_t size);
	constexpr command::CommandArgument operator""_as(const char* name, std::size_t size);

	class CommandOptionLiteral {
	public:
		template <typename... TArgs>
		CommandOptionLiteral& operator()(TArgs... args);

		CommandOptionLiteral& operator[](bool is_required);

		constexpr operator std::pair<std::string_view, command::CommandOption>() const;
	private:
		std::string_view name_{};
		command::CommandOption option_{};

		constexpr CommandOptionLiteral(std::string_view name);

		friend constexpr CommandOptionLiteral operator""_o(const char* name, std::size_t size);
	};

	constexpr CommandOptionLiteral operator""_o(const char* name, std::size_t size);
}

#include "CommandLiterals.tcc"
#endif