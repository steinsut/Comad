#ifndef COMAD_COMMAND_NODE_TCC_
#define COMAD_COMMAND_NODE_TCC_

#include <Logger.tcc>
#include <type_traits>

#include "CommandNode.h"
#include "ComadBuildOptions.h"

namespace comad::command {
	template <std::ranges::input_range Range>
		requires std::is_constructible_v<std::string, std::ranges::range_value_t<Range>>
	CommandNode& CommandNode::operator|(const Range& range) {
		using alias_type = std::ranges::range_value_t<Range>;

		auto debug_stream = logger::comad_logger.MakeStream<logger::LogLevel::DEBUG>();

		for (const alias_type& alias : range) {
			if constexpr (build_options::Verbose) debug_stream << "adding alias" << alias << " for node " <<  name_ << std::flush;
			if (alias.empty()) {
				throw std::invalid_argument("subcommand alias cannot be empty");
			}
			if (HasWhitespace(alias)) {
				throw std::invalid_argument("subcommand alias cannot have whitespaces");
			}
			cmd_template_.aliases.emplace(alias);
		}

		if (parent_) {
			parent_->ChildUpdated(name_);
		}

		return *this;
	}

	template <std::ranges::input_range Range>
		requires std::is_constructible_v<std::string, std::ranges::range_value_t<Range>>
	CommandNode& CommandNode::operator|(Range&& range) {
		using alias_type = std::ranges::range_value_t<Range>;

		auto debug_stream = logger::comad_logger.MakeStream<logger::LogLevel::DEBUG>();

		for (alias_type& alias : range) {
			if constexpr (build_options::Verbose) debug_stream << "adding alias" << alias << " for node " <<  name_ << std::flush;
			if (alias.empty()) {
				throw std::invalid_argument("subcommand alias cannot be empty");
			}
			if (HasWhitespace(alias)) {
				throw std::invalid_argument("subcommand alias cannot have whitespaces");
			}
			cmd_template_.aliases.emplace(std::move(alias));
		}

		if (parent_) {
			parent_->ChildUpdated(name_);
		}

		return *this;
	}

	template<CommandPassable ...Passables>
	CommandNode& CommandNode::operator()(Passables && ...passables) {
		using namespace build_options;
		using namespace logger;

		auto debug_stream = comad_logger.MakeStream<LogLevel::DEBUG>();

		CommandTemplate tmp{ std::move(cmd_template_) };


		([this, &tmp, &debug_stream]<typename T>(T&& passable) {
				if constexpr (std::is_same_v<CommandFlag, std::remove_cvref_t<T>>) {
					if constexpr (Verbose) debug_stream << "adding flag " << passable << " to node " << name_ << std::flush;
					tmp.flags.insert(std::forward<T>(passable));
				}
				else if constexpr (std::is_same_v<CommandArgument, std::remove_cvref_t<T>>) {
					if constexpr (Verbose) debug_stream << "adding argument " << passable.first
						<< " of type " << value::ValueTypeNames.at(passable.second)
						<< " to node " << name_ << std::flush;
					tmp.args.push_back(std::forward<T>(passable));
				}
				else {
					std::pair<std::string_view, CommandOption> option_pair = passable;
					if constexpr (Verbose) debug_stream << "adding option " << option_pair.first
						<< " accepting values of type " << value::ValueTypeNames.at(option_pair.second.supported_values.GetValueType())
						<< " to node " << name_ << std::flush;
					tmp.options.emplace(option_pair.first, std::move(option_pair.second));
				}
		}(passables), ...);

		SetTemplate(tmp);

		return *this;
	}
}

#endif