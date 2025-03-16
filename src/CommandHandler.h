#ifndef COMAD_COMMAND_HANDLER_H_
#define COMAD_COMMAND_HANDLER_H_

#include <charconv>
#include <concepts>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <optional>

#include "Value.h"
#include "Logger.h"
#include "CommandNode.h"


namespace comad::command {
	namespace detail {
		bool IsValueValid(const CommandOption& option, const value::ValueWrapper& value);

		std::optional<value::ValueWrapper> StringToValue(value::ValueType type, std::string_view str);

		template<std::integral T>
		std::optional<T> OptionalFromChars(std::string_view str, int base = 10);

		template<std::floating_point T>
		std::optional<T> OptionalFromChars(std::string_view str, std::chars_format fmt = std::chars_format::general);

		template <std::input_iterator iter> requires
			(std::is_constructible_v<std::string_view, std::iter_value_t<iter>> ||
				std::is_convertible_v<std::iter_value_t<iter>, std::string_view>)
		const CommandNode& FindNode(const CommandNode& start_node, iter& command_name_it, iter end_it);

		int ParseOption(std::string_view name,
						std::string_view value,
						const CommandNode& node,
						ExecutionContext& ctx);
	}

	class CommandHandler {
	public:
		[[nodiscard]] CommandNode& GetCommandNode() noexcept;
		[[nodiscard]] const CommandNode& GetCommandNode() const noexcept;

		void SetCommandNode(CommandNode node) noexcept;
		int HandleCommand(int argc, const char** argv) const;

		template <std::ranges::input_range Range> requires
			(std::is_constructible_v<std::string_view, std::ranges::range_value_t<Range>> ||
			std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>)
		int HandleCommand(const Range& range) const;

		template <typename... TArgs> requires (... && (std::is_constructible_v<std::string_view, TArgs>
														|| std::is_convertible_v<TArgs, std::string_view>))
		int HandleCommand(TArgs&&... args) const;

	private:
		CommandNode node_{};
	};
}


#include "CommandHandler.tcc"
#endif