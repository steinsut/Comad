#ifndef COMMAND_NODE_H_
#define COMMAND_NODE_H_

#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "Command.h"

namespace comad {
	namespace command {
		template <typename T>
		concept PassableType = std::is_same_v<T, CommandFlag> ||
			std::is_same_v<T, CommandArgument> ||
			std::is_convertible_v<T, std::pair<std::string_view, CommandOption>>;


		class CommandNode {
		public:
			CommandNode();
			CommandNode(CommandTemplate cmd_template, CommandExecutor executor);


			bool AddNode(std::string_view name);
			bool AddNode(std::string_view name, CommandTemplate cmd_template, CommandExecutor executor);
			
			[[nodiscard]] std::string_view GetName() const noexcept;
			[[nodiscard]] bool HasOption(std::string_view option_name) const noexcept;
			[[nodiscard]] const CommandOption& GetOption(std::string_view option_name) const;
			[[nodiscard]] int GetRequiredOptionCount() const noexcept;

			[[nodiscard]] bool HasChild(std::string_view name) const noexcept;
			[[nodiscard]] CommandNode& GetChild(std::string_view name);
			[[nodiscard]] const CommandNode& GetChild(std::string_view name) const;
			[[nodiscard]] bool HasChildAlias(std::string_view name) const noexcept;
			[[nodiscard]] std::string_view GetChildNameFromAlias(std::string_view alias) const;
			[[nodiscard]] bool HasShortOption(char short_name) const noexcept;
			[[nodiscard]] const std::string& GetShortOptionName(char short_name) const;

			[[nodiscard]] bool HasParent() const noexcept;
			[[nodiscard]] const CommandNode& GetParent() const;

			bool Remove(std::string_view name);

			void SetTemplate(CommandTemplate cmd_template);
			[[nodiscard]] const CommandTemplate& GetTemplate() const noexcept;

			[[nodiscard]] const std::map<std::string, std::string, std::less<>>& GetChildAliasMapping() const noexcept;
			[[nodiscard]] const std::map<char, std::string, std::less<>>& GetShortOptionMapping() const noexcept;

			void SetExecutor(CommandExecutor executor) noexcept;
			[[nodiscard]] CommandExecutor GetExecutor() const noexcept;

			void SetCommand(CommandTemplate cmd_template, CommandExecutor executor);

			CommandNode& operator>>(std::string_view cmd);
			CommandNode& operator=(CommandExecutor executor);

			CommandNode& operator|(std::string_view alias);

			template <std::ranges::input_range Range> requires std::is_constructible_v<std::string, std::ranges::range_value_t<Range>>
			CommandNode& operator|(const Range& range);

			template <std::ranges::input_range Range> requires std::is_constructible_v<std::string, std::ranges::range_value_t<Range>>
			CommandNode& operator|(Range&& range);

			template <PassableType... Passables>
			CommandNode& operator()(Passables&&... passables);

		private:
			std::map<std::string, CommandNode, std::less<>> sub_nodes_{};
			std::map<std::string, std::string, std::less<>> alias_to_name_{};
			std::map<char, std::string, std::less<>> short_to_full_opt_{};

			std::string_view name_{""};
			CommandNode* parent_{ nullptr };
			CommandTemplate cmd_template_{};
			CommandExecutor executor_{ nullptr };
			int required_option_count_{ 0 };

			CommandNode(std::reference_wrapper<CommandNode> parent, std::string_view name);

			void ChildUpdated(std::string_view child_name);
		};
	}
}

#include "CommandNode.tcc"
#endif
