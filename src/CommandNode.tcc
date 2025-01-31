#ifndef COMMAND_NODE_TCC_
#define COMMAND_NODE_TCC_

#include <ranges>
#include <type_traits>

#include "CommandNode.h"

namespace comad {
	namespace command {
		template <std::ranges::input_range Range> 
			requires std::is_constructible_v<std::string, std::ranges::range_value_t<Range>>
		CommandNode& CommandNode::operator|(const Range& range) {
			using alias_type = std::ranges::range_value_t<Range>;

			for (const alias_type& alias : range) {
				cmd_template_.aliases.emplace(alias);
			}

			return *this;
		}

		template <std::ranges::input_range Range> 
			requires std::is_constructible_v<std::string, std::ranges::range_value_t<Range>>
		CommandNode& CommandNode::operator|(Range&& range) {
			using alias_type = std::ranges::range_value_t<Range>;

			for (alias_type& alias : range) {
				cmd_template_.aliases.emplace(std::move(alias));
			}

			return *this;
		}

		template<PassableType ...Passables>
		CommandNode& CommandNode::operator()(Passables && ...passables) {
			CommandTemplate tmp{ std::move(cmd_template_) };

			(...,
				[&tmp](auto&& passable) {
					using passable_type = decltype(passable);

					if constexpr (std::is_same_v<CommandFlag, std::remove_cvref_t<passable_type>>) {
						tmp.flags.insert(std::forward<passable_type>(passable));
					}
					else if constexpr (std::is_same_v<CommandArgument, std::remove_cvref_t<passable_type>>) {
						tmp.args.push_back(std::forward<passable_type>(passable));
					}
					else {
						std::pair<std::string_view, CommandOption> option_pair = passable;
						tmp.options.emplace(option_pair.first, option_pair.second);
					}
			}(passables));

			SetTemplate(tmp);

			return *this;
		}
	}
}
#endif