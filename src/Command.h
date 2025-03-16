#ifndef COMAD_COMMAND_H_
#define COMAD_COMMAND_H_

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ValueUtility.h"

namespace comad::command {
	struct CommandOption {
		value::SupportedValueHolder supported_values{ };
		char short_name{ 0 };
		bool required{ false };

		CommandOption& operator[](bool is_required) noexcept;
		CommandOption& operator()(value::ValueType type) noexcept;
		CommandOption& operator()(value::ValueBounds bounds) noexcept;
		CommandOption& operator()(value::ValueRange auto&& values);

		template <value::ValidType T, value::ValidType... TOthers> requires (std::conjunction_v<std::is_same<T, TOthers>...>)
		CommandOption& operator()(T value, TOthers... others);
	};

	using CommandArgument = std::pair<std::string, value::ValueType>;
	using CommandFlag = std::string;

	struct CommandTemplate {
		std::set<std::string, std::less<>> aliases{ };
		std::set<std::string, std::less<>> flags{ };
		std::map<std::string, CommandOption, std::less<>> options{ };
		std::vector<CommandArgument> args{ };
		std::string description{ };
	};

	class ExecutionContext {
	public:
		int required_option_count{ };
		std::map<std::string, value::ValueWrapper, std::less<>> options{ };
		std::map<std::string, bool, std::less<>> flags{ };
		std::map<std::string, value::ValueWrapper, std::less<>> args{ };
		std::vector<std::string> extra_args{ };
	};

	using CommandExecutor = int(*)(const ExecutionContext& info);
}

#include "Command.tcc"
#endif
