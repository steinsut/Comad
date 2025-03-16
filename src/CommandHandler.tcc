#ifndef COMAD_COMMAND_HANDLER_TCC_
#define COMAD_COMMAND_HANDLER_TCC_

#include <algorithm>
#include <charconv>
#include <stdexcept>
#include <cstring>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <ComadBuildOptions.h>
#include <ComadReturnCodes.h>
#include <Logger.tcc>

#include "CommandHandler.h"


namespace comad::command {
	template<std::integral T>
	std::optional<T> detail::OptionalFromChars(std::string_view str, int base)
	{
		using namespace build_options;
		using namespace logger;

		T var;
		const char* first = str.data();
		const char* last = first + str.length();
		auto result = std::from_chars(first, last, var, base);

		if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
			comad_logger.MakeStream<LogLevel::ERROR>() << "failed to parse number from " <<
				str << ": " << std::make_error_code(result.ec).message();
			return std::nullopt;
		}

		if constexpr (!AllowPartialNumberParsing) {
			if (*result.ptr != '\0' || result.ptr != last) {
				comad_logger.MakeStream<LogLevel::ERROR>() << "failed to parse number from " <<
					str << ": partial number parsing is disabled";
				return std::nullopt;
			}
		}

		return var;
	}

	template<std::floating_point T>
	std::optional<T> detail::OptionalFromChars(std::string_view str, std::chars_format fmt)
	{
		using namespace build_options;
		using namespace logger;

		T var;
		const char* first = str.data();
		const char* last = first + str.length();
		auto result = std::from_chars(first, last, var, fmt);

		if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
			comad_logger.MakeStream<LogLevel::ERROR>() << "failed to parse number from " <<
				str << ": " << std::make_error_code(result.ec).message();
			return std::nullopt;
		}

		if constexpr (!AllowPartialNumberParsing) {
			if (*result.ptr != '\0' || result.ptr != last) {
				comad_logger.MakeStream<LogLevel::ERROR>() << "failed to parse number from " <<
					str << ": partial number parsing is disabled";
				return std::nullopt;
			}
		}

		return var;
	}


	template <std::input_iterator iter> requires
		(std::is_constructible_v<std::string_view, std::iter_value_t<iter>> ||
			std::is_convertible_v<std::iter_value_t<iter>, std::string_view>)
	const CommandNode& detail::FindNode(const CommandNode& start_node, iter& command_name_it, iter end_it)
	{
		using namespace logger;
		using namespace build_options;

		auto debug_stream = comad_logger.MakeStream<LogLevel::DEBUG>();

		if constexpr(Verbose) debug_stream << "searching end node" << std::flush;

		std::reference_wrapper<const CommandNode> current_node = std::ref(start_node);

		while (command_name_it != end_it) {
			std::string_view str{ *command_name_it };
			if constexpr(Verbose) debug_stream << "searching for node " << str << std::flush;

			if (current_node.get().HasChildAlias(str)) {
				str = current_node.get().GetChildNameFromAlias(str);
				if constexpr(Verbose) debug_stream << "node was aliasing " << str << std::flush;
			}

			if (!current_node.get().HasChild(str)) {
				break;
			}
			else {
				current_node = current_node.get().GetChild(str);
				if constexpr(Verbose) debug_stream << "node " << str << " found" << std::flush;
				++command_name_it;
			}
		}

		if constexpr(Verbose) debug_stream << "end node " << current_node.get().GetName() << " found" << std::flush;
		return current_node.get();
	}

	template <std::ranges::input_range Range> requires
		(std::is_constructible_v<std::string_view, std::ranges::range_value_t<Range>> ||
		std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>)
	int CommandHandler::HandleCommand(const Range& range) const
	{
		using namespace detail;
		using namespace logger;
		using namespace build_options;

		auto debug_stream = comad_logger.MakeStream<LogLevel::DEBUG>();

		auto range_count = std::ranges::ssize(range);

		if (range_count == 0 && !node_.GetExecutor()) {
			if constexpr (Verbose) comad_logger.Error("no input provided");
			return retc::kNoInput;
		}

		auto current_iterator = range.begin();
		const CommandNode& current_node = FindNode(node_, current_iterator, range.end());

		CommandExecutor executor_ = current_node.GetExecutor();

		if (!executor_) {
			if constexpr (Verbose) debug_stream << "unknown command " << current_node.GetName() << std::flush;
			return retc::kUnknownCommand;
		}

		const CommandTemplate& cmd_template = current_node.GetTemplate();
		ExecutionContext ctx{};
		int arg_index = 0;

		for (const std::string& flag_name : cmd_template.flags) {
			ctx.flags.emplace(flag_name, false);
		}

		for (; current_iterator != range.end(); ++current_iterator) {
			bool processing = true;
			std::string_view element{ *current_iterator };

			if (element.starts_with(FlagPrefix)) {
				std::string_view flag_name{ element };
				flag_name.remove_prefix(FlagPrefix.size());

				if (cmd_template.flags.contains(flag_name)) {
					auto it = ctx.flags.find(flag_name);
					it->second = true;
					processing = false;
				}
				else if constexpr (!SkipUnknownFlag) {
					if constexpr (Verbose) {
						comad_logger.MakeStream<LogLevel::ERROR>() << "unknown flag " << flag_name;
					}
					return retc::kUnknownFlag;
				}
			}

			if (processing && current_iterator + 1 != range.end()) {
				int ret = ParseOption(*current_iterator, *(current_iterator + 1), current_node, ctx);
				if (ret < 0) { return ret; }

				if (ret == retc::kOptionParsed) {
					processing = false;
					++current_iterator;
				}
			}

			if (processing) {
				if (arg_index < cmd_template.args.size()) {
					auto arg_value = StringToValue(cmd_template.args[arg_index].second, *current_iterator);
					if (arg_value == std::nullopt) {
						if constexpr (!SkipInvalidValueParse) {
							if constexpr (Verbose) comad_logger.Error("invalid argument");
							return retc::kInvalidValueParse;
						}
					}
					else {
						ctx.args.emplace(cmd_template.args[arg_index].first, std::move(*arg_value));

						++arg_index;
					}
				}
				else {
					if constexpr(CacheExtraArgs) ctx.extra_args.emplace_back(*current_iterator);
				}
			}
		}

		if (ctx.required_option_count < current_node.GetRequiredOptionCount()) {
			if constexpr (Verbose) comad_logger.Error("all required options have not been passed");
			return retc::kMissingRequiredOptions;
		}

		return executor_(ctx);
	}

	template <typename... TArgs> requires (... && (std::is_constructible_v<std::string_view, TArgs>
													|| std::is_convertible_v<TArgs, std::string_view>))
	int CommandHandler::HandleCommand(TArgs&& ...args) const {
		std::vector<std::string_view> arg_vector{ };
		arg_vector.reserve(sizeof...(TArgs));

		([&arg_vector](auto&& arg) {
				using arg_type = decltype(arg);

				if constexpr (std::is_convertible_v<arg_type, const char *>) {
					const char* end = std::memchr(arg, 0, build_options::kMaxCStringLength);
					if (end != nullptr) {
						arg_vector.emplace_back(arg, end - arg);
					}
					else {
						throw std::invalid_argument("c-string is either too big or missing the null terminator");
					}
				}
				else {
					arg_vector.emplace_back(arg);
				}
		}(args), ...);

		return HandleCommand(arg_vector);
	}
}

#endif