#ifndef COMMAND_HANDLER_H_
#define COMMAND_HANDLER_H_

#include <charconv>
#include <concepts>
#include <ranges>
#include <string_view>
#include <type_traits>

#include "Value.h"
#include "CommandNode.h"

namespace comad {
	namespace command {
		namespace detail {
			bool IsValueValid(const CommandOption& option, std::string_view value_str_copy);
			value::ValueWrapper StringToValue(value::ValueType type, std::string_view value_str_copy);
			
			const char* ThrowingFromChars(const char* first,
											const char* last,
											std::integral auto& var,
											int base = 10);

			
			const char* ThrowingFromChars(const char* first,
											const char* last,
											std::floating_point auto& var,
											std::chars_format fmt = std::chars_format::general);

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

			void SetCommandNode(CommandNode pack) noexcept;
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
}

#include "CommandHandler.tcc"
#endif