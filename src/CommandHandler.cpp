#include "CommandHandler.h"

#include <charconv>
#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

#include "ComadBuildOptions.h"

using namespace std::string_view_literals;

namespace comad {
	namespace command {
		using namespace value;

		bool detail::IsValueValid(const CommandOption& option, std::string_view value_str) {
			const value::ValueType type = option.supported_values.GetValueType();
			const ValueWrapper value = StringToValue(type, value_str);

			switch (type)
			{
				case value::ValueType::kBool: return option.supported_values.IsValid(value.GetValue<bool>());
				case value::ValueType::kInt: return option.supported_values.IsValid(value.GetValue<int>());
				case value::ValueType::kFloat: return option.supported_values.IsValid(value.GetValue<float>());
				case value::ValueType::kString: return option.supported_values.IsValid(value.GetValue<std::string>());
				default: throw std::runtime_error{ "type is not supported." };
			}
		}

		ValueWrapper detail::StringToValue(value::ValueType type, std::string_view value_str) {
			using namespace build_options;

			switch (type) 
			{
				case value::ValueType::kBool: {
					std::string value_str_copy{ value_str };
					std::transform(value_str_copy.begin(), value_str_copy.end(), value_str_copy.begin(),
						[](unsigned char c) { return std::tolower(c);  });

					if (value_str_copy == "0"sv || value_str_copy == "false"sv) {
						return ValueWrapper{ false };
					}
					if (value_str_copy == "1"sv || value_str_copy == "true"sv) {
						return ValueWrapper{ true };
					}

					throw std::runtime_error{ std::format("\"{}\" cannot be converted to bool", value_str) };
				}
				case value::ValueType::kInt: {
					int value;
					auto ptr = ThrowingFromChars(value_str.data(), value_str.data() + value_str.length(), value);

					return ValueWrapper{ value };
				}
				case value::ValueType::kFloat: {
					float value;
					auto ptr = ThrowingFromChars(value_str.data(), value_str.data() + value_str.length(), value);

					return ValueWrapper{ value };
				}
				case value::ValueType::kString: return ValueWrapper(std::string{ value_str });
				default: throw std::runtime_error{ "type is not supported." };
			}
		}

		int detail::ParseOption(std::string_view name,
			std::string_view value,
			const CommandNode& node,
			ExecutionContext& ctx)
		{
			using namespace build_options;
			using namespace detail;

			if (name.starts_with(OptionPrefix)) {
				name.remove_prefix(OptionPrefix.length());
			}
			else if (name.starts_with(ShortOptionPrefix)) {
				name.remove_prefix(ShortOptionPrefix.length());

				if (node.HasShortOption(name[0])) {
					name = node.GetShortOptionName(name[0]);
				}
			}

			if (!node.HasOption(name)) {
				if constexpr (!SkipUnknownOption) {
					std::cerr << "unknown option " << name << std::endl;
					return retc::kUnknownOption;
				}
				else {
					return retc::kOptionNotParsed;
				}
			}

			const CommandOption& option = node.GetOption(name);
			if constexpr (!SkipDupeOption) {
				if (ctx.options.find(name) != ctx.options.end()) {
					std::cerr << "option " << name << " has already been passed";
					return retc::kDupeOption;
				}
			}

			const value::SupportedValueHolder& option_values = option.supported_values;
			try {
				if (IsValueValid(option, value)) {
					ctx.options.emplace(name,
						StringToValue(option_values.GetValueType(), value));

					ctx.required_option_count += option.required;

					return retc::kOptionParsed;
				}
				else if (option.required) {
					std::cerr << "invalid value " << value << std::endl;
					return retc::kInvalidOptionValue;
				}
				return retc::kOptionNotParsed;
			}
			catch (const std::exception& ex) {
				if constexpr (!SkipInvalidValueParse) {
					std::cerr << "failed to parse required option\n" << ex.what() << std::endl;
					return retc::kInvalidValueParse;
				}
				else {
					return retc::kOptionNotParsed;
				}
			}
		}

		CommandNode& CommandHandler::GetCommandNode() noexcept {
			return node_;
		}

		const CommandNode& CommandHandler::GetCommandNode() const noexcept {
			return node_;
		}

		void CommandHandler::SetCommandNode(CommandNode node) noexcept {
			node_ = node;
		}

		int CommandHandler::HandleCommand(int argc, const char** argv) const {
			return HandleCommand(std::span<const char*>(argv, argc));
		}
	}
}