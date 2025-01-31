#ifndef COMMAND_HANDLER_TCC_
#define COMMAND_HANDLER_TCC_

#include <algorithm>
#include <charconv>
#include <format>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <ComadBuildOptions.h>
#include <ComadReturnCodes.h>

#include "CommandHandler.h"

namespace comad {
	namespace command {
		const char* detail::ThrowingFromChars(const char* first,
												const char* last,
												std::integral auto& var,
												int base) 
		{			
			using namespace build_options;
			
			auto result = std::from_chars(first, last, var, base);

			if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
				throw std::runtime_error{ std::format("failed to parse value\n", 
					std::make_error_code(result.ec).message()) };
			}

			if constexpr (!AllowPartialNumberParsing) {
				if (result.ptr != '\0' || result.ptr != last) {
					throw std::runtime_error{ "failed to parse\nnot all chars can be converted" };
				}
			}

			return result.ptr;
		}

		const char* detail::ThrowingFromChars(const char* first,
											const char* last,
											std::floating_point auto& var,
											std::chars_format fmt) 
		{
			using namespace build_options;

			auto result = std::from_chars(first, last, var, fmt);

			if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
				throw std::runtime_error{ std::format("failed to parse value\n",
					std::make_error_code(result.ec).message()) };
			}

			if constexpr (!AllowPartialNumberParsing) {
				if (result.ptr != '\0' || result.ptr != last) {
					throw std::runtime_error{ "failed to parse\nnot all chars can be converted" };
				}
			}

			return result.ptr;
		}


		template <std::input_iterator iter> requires
			(std::is_constructible_v<std::string_view, std::iter_value_t<iter>> ||
				std::is_convertible_v<std::iter_value_t<iter>, std::string_view>)
		const CommandNode& detail::FindNode(const CommandNode& start_node, iter& command_name_it, iter end_it)
		{
			std::reference_wrapper<const CommandNode> current_node = std::ref(start_node);

			while (command_name_it != end_it) {
				std::string_view str{ *command_name_it };
				if (current_node.get().HasChildAlias(str)) {
					str = current_node.get().GetChildNameFromAlias(str);
				}

				if (!current_node.get().HasChild(str)) {
					break;
				}
				else {
					current_node = current_node.get().GetChild(str);
					++command_name_it;
				}
			}

			return current_node.get();
		}

		template <std::ranges::input_range Range> requires
			(std::is_constructible_v<std::string_view, std::ranges::range_value_t<Range>> ||
			std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>)
		int CommandHandler::HandleCommand(const Range& range) const 
		{
			using namespace detail;
			using namespace build_options;

			auto range_count = std::ranges::ssize(range);

			if (range_count == 0 && !node_.GetExecutor()) {
				std::cerr << "no input provided" << std::endl;
				return retc::kNoInput;
			}

			auto current_iterator = range.begin();
			const CommandNode& current_node = FindNode(node_, current_iterator, range.end());
			
			CommandExecutor executor_ = current_node.GetExecutor();

			if (!executor_) {
				std::cerr << "unknown command " << current_node.GetName() << std::endl;
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
						std::cerr << "unknown flag " << flag_name << std::endl;
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
						try {
							ctx.args.emplace(cmd_template.args[arg_index].first,
								StringToValue(cmd_template.args[arg_index].second, *current_iterator));

							++arg_index;
						}
						catch (const std::exception& ex) {
							if constexpr (!SkipInvalidValueParse) {
								std::cerr << "invalid argument\n" << ex.what() << std::endl;
								return retc::kInvalidValueParse;
							}
						}
					}
					else {
						ctx.extra_args.emplace_back(*current_iterator);
					}
				}
			}

			if (ctx.required_option_count < current_node.GetRequiredOptionCount()) {
				std::cerr << "all required options have not been passed" << std::endl;
				return retc::kMissingRequiredOptions;
			}

			return executor_(ctx);

			return 0;
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
}

#endif