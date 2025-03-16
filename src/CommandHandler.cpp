#include "CommandHandler.h"

#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <optional>
#include <Logger.tcc>
#include <utility>

#include "ComadBuildOptions.h"

using namespace std::string_view_literals;

namespace comad::command {
	using namespace value;
	using namespace logger;

	bool detail::IsValueValid(const CommandOption& option, const ValueWrapper& value) {
		const ValueType type = option.supported_values.GetValueType();
		if (type != value.GetType()) return false;

		switch (type)
		{
			case ValueType::kBool: return option.supported_values.IsValid(value.GetValue<bool>());
			case ValueType::kInt: return option.supported_values.IsValid(value.GetValue<int>());
			case ValueType::kFloat: return option.supported_values.IsValid(value.GetValue<float>());
			case ValueType::kString: return option.supported_values.IsValid(value.GetValue<std::string>());
			default: throw std::runtime_error{ "type is not supported." };
		}
	}

	std::optional<ValueWrapper> detail::StringToValue(ValueType type, std::string_view str) {
		using namespace build_options;

		if (Verbose) {
			comad_logger.MakeStream<LogLevel::DEBUG>()
				<< "trying to parse " << ValueTypeNames.at(type) << " from " << str;
		}
		switch (type)
		{
			case ValueType::kBool: {
				std::string value_str_copy{ str };
				std::ranges::transform(value_str_copy, value_str_copy.begin(),
				                       [](unsigned char c) { return std::tolower(c);  });

				if (value_str_copy == "0"sv || value_str_copy == "false"sv) {
					return std::make_optional<ValueWrapper>(false);
				}
				if (value_str_copy == "1"sv || value_str_copy == "true"sv) {
					return std::make_optional<ValueWrapper>(true);
				}

				if constexpr (Verbose) {
					comad_logger.MakeStream<LogLevel::ERROR>() << str << " cannot be converted to bool";
				}
				return std::nullopt;
			}
			case ValueType::kInt: {
				auto val = OptionalFromChars<int>(str);

				if (val == std::nullopt) return std::nullopt;
				return std::make_optional<ValueWrapper>(val.value());
			}
			case ValueType::kFloat: {
				auto val = OptionalFromChars<float>(str);

				if (val == std::nullopt) return std::nullopt;
				return std::make_optional<ValueWrapper>(val.value());
			}
			case ValueType::kString: return std::make_optional<ValueWrapper>(std::string{ str });
			default: {
				if constexpr (Verbose) comad_logger.Error("type is not supported."sv);
				return std::nullopt;
			}
		}
	}

	int detail::ParseOption(std::string_view name,
		std::string_view value,
		const CommandNode& node,
		ExecutionContext& ctx)
	{
		using namespace build_options;
		using namespace detail;

		auto debug_stream = comad_logger.MakeStream<LogLevel::DEBUG>();
		auto error_stream = comad_logger.MakeStream<LogLevel::ERROR>();

		if (name.starts_with(OptionPrefix)) {
			name.remove_prefix(OptionPrefix.length());

			if constexpr (Verbose) debug_stream << "searching for option with name " << name << std::flush;
		}
		else if (name.starts_with(ShortOptionPrefix)) {
			name.remove_prefix(ShortOptionPrefix.length());

			if (node.HasShortOption(name[0])) {
				if constexpr (Verbose)  debug_stream << "searching for option with short name " << name[0] << std::flush;
				name = node.GetShortOptionName(name[0]);
			}
			if constexpr (Verbose) debug_stream << "searching for option with short name " << name << std::flush;
		}

		if (!node.HasOption(name)) {
			if constexpr (!SkipUnknownOption) {
				if constexpr (Verbose) error_stream << "unknown option " << name << std::flush;
				return retc::kUnknownOption;
			}
			else {
				return retc::kOptionNotParsed;
			}
		}

		const CommandOption& option = node.GetOption(name);
		if constexpr (!SkipDupeOption) {
			if (ctx.options.contains(name)) {
				if constexpr (Verbose) error_stream << "option " << name << " has already been passed" << std::flush;
				return retc::kDupeOption;
			}
		}

		const SupportedValueHolder& option_values = option.supported_values;
		auto wrapped = StringToValue(option_values.GetValueType(), value);
		if (wrapped == std::nullopt) {
			if constexpr (!SkipInvalidValueParse) {
				if constexpr (Verbose) error_stream << "failed to parse value for option " << name << std::flush;
				return retc::kInvalidValueParse;
			}
			else {
				return retc::kOptionNotParsed;
			}
		}
		if (IsValueValid(option, *wrapped)) {
			ctx.options.emplace(name, std::move(*wrapped));

			ctx.required_option_count += option.required;
			return retc::kOptionParsed;
		}
		if constexpr (Verbose) error_stream << "invalid value " << value << " for option " << name << std::flush;

		return retc::kInvalidOptionValue;
	}

	CommandNode& CommandHandler::GetCommandNode() noexcept {
		return node_;
	}

	const CommandNode& CommandHandler::GetCommandNode() const noexcept {
		return node_;
	}

	void CommandHandler::SetCommandNode(CommandNode node) noexcept {
		node_ = std::move(node);
	}

	int CommandHandler::HandleCommand(int argc, const char** argv) const {
		return HandleCommand(std::span<const char*>(argv, argc));
	}
}