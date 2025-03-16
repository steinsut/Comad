#include <string_view>
#include <iostream>

#include "Comad.h"

int main() {
	using namespace comad;
	using namespace comad::command;
	using namespace comad::literals;
	using namespace comad::value;
	using namespace comad::logger;
	using namespace std::string_view_literals;
	using namespace std::string_literals;

	Version version = GetLinkedVersion();

	std::cout << "comad version " << version << std::endl << std::endl;

	//test logger behaviour
	Logger logger{
		{.info = {std::ref(std::cout)},
				.debug = {std::ref(std::cout)},
				.error = {std::ref(std::cerr)}},
		"comad: [{0:M}] ({1:%F}T{1:%R%z}) {2:%F:%L:%C/}: {3}",
		LogLevelSpecifier{},
		TimeSpecifier{},
		SourceLocationSpecifier{},
		MessageSpecifier{}};

	logger.Info("Testing logger with log level info from log method");
	logger.MakeStream<LogLevel::DEBUG>() << "Testing logger with " << "log level debug " << "from stream ";
	logger.MakeStream<LogLevel::ERROR>() << "Testing logger with log level error from stream" << std::endl;

	bool failed = false;



	//test simple command parsing
	CommandHandler parse_test{};

	parse_test.GetCommandNode() >> "test1"sv = [](const ExecutionContext&) {
		return 1;
	};

	if (parse_test.HandleCommand("test1"sv) != 1) {
		std::cerr << "parse test failed"sv << std::endl << std::endl;
		failed = true;
	}


	//test command aliases
	CommandHandler alias_test{};

	alias_test.GetCommandNode() >> "test2"sv | "t2"sv | "tt2"sv = [](const ExecutionContext&) {
		return 2;
	};

	if (alias_test.HandleCommand("t2"sv) != 2 || alias_test.HandleCommand("tt2"sv) != 2) {
		std::cerr << "alias test failed"sv << std::endl << std::endl;
		failed = true;
	}

	//test subcommands
	CommandHandler subcmd_test{};

	subcmd_test.GetCommandNode() >> "test3"sv >> "subcmd" = [](const ExecutionContext&) {
		return 3;
	};

	if (subcmd_test.HandleCommand("test3"sv, "subcmd"sv) != 3) {
		std::cerr << "subcommand test failed" << std::endl << std::endl;
		failed = true;
	}

	//test flags
	CommandHandler flag_test{};

	(flag_test.GetCommandNode() >> "test4"sv)("flag1"_fl, "flag2"_fl) = [](const ExecutionContext& ctx) {
		if (ctx.flags.find("flag1"sv)->second && ctx.flags.find("flag2"sv)->second) {
			return 4;
		}

		return -1;
	};
	if (flag_test.HandleCommand("test4"sv, "-fflag1"sv, "-fflag2"sv) != 4) {
		std::cerr << "flag test failed"sv << std::endl << std::endl;
		failed = true;
	}

	//test arguments
	CommandHandler arg_test{};

	(arg_test.GetCommandNode() >> "test5"sv)("bool"_ab, "integer"_ai, "float"_af, "string"_as) = [](const ExecutionContext& ctx) {
		float arg_test_float{};
		std::string_view arg_test_float_str = "1.0"sv;
		std::from_chars(arg_test_float_str.data(), arg_test_float_str.data() + arg_test_float_str.length(), arg_test_float);

		if (ctx.args.find("bool"sv)->second.GetValue<bool>() &&
			ctx.args.find("integer"sv)->second.GetValue<int>() == 5 &&
			ctx.args.find("float"sv)->second.GetValue<float>() == arg_test_float &&
			ctx.args.find("string"sv)->second.GetValue<std::string>() == "value"sv) {

			return 5;
		}

		return -1;
	};

	if (arg_test.HandleCommand("test5"sv, "true"sv, "5"sv, "1.0f"sv, "value"sv) != 5) {
		std::cerr << "argument parsing test failed"sv << std::endl << std::endl;
		failed = true;
	}

	//test options
	CommandHandler option_test{};
	(option_test.GetCommandNode() >> "test6"sv)("boolopt"_o(value::ValueType::kBool)[true],
		"intopt"_o(ValueBounds{ 0, 100 }),
		"floatopt"_o(value::ValueType::kFloat)[true],
		"stringopt"_o("value1"s, "value2"s)[true]) =
	[](const ExecutionContext& ctx) {
		if (ctx.options.find("boolopt"sv)->second.GetType() == value::ValueType::kBool &&
			ctx.options.find("intopt"sv)->second.GetValue<int>() == 6 &&
			ctx.options.find("floatopt"sv)->second.GetType() == value::ValueType::kFloat &&
			ctx.options.find("stringopt"sv)->second.GetValue<std::string>() == "value1"sv) {

			return 6;
		}

		return -1;
	};

	if (option_test.HandleCommand("test6"sv, "--boolopt"sv, "false"sv,
		"--intopt"sv, "6"sv,
		"--floatopt"sv, "1.0f"sv,
		"--stringopt"sv, "value1"sv) != 6) {

		std::cout << "option test failed"sv << std::endl << std::endl;
		failed = true;
	}

	if (failed) {
		std::cerr << "all tests did not succeed"sv << std::endl;
		return -1;
	}

	std::cout << "all tests passed" << std::endl;
	return 0;
}