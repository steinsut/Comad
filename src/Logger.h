#ifndef COMAD_LOGGER_H_
#define COMAD_LOGGER_H_

#include <string_view>
#include <format>
#include <mutex>
#include <sstream>
#include <concepts>
#include <source_location>
#include <vector>
#include <memory>

#include "TypeTraits.h"

namespace comad::logger {
	enum class LogLevel {
		ERROR,
		DEBUG,
		INFO
	};

	template <typename T>
	concept LoggableCharType =
		std::same_as<T, char>;

	template <LoggableCharType CharT>
	struct LogStreams {
		using StreamType = std::basic_ostream<CharT>;
		using StreamRefWrapper = std::reference_wrapper<StreamType>;

		std::vector<StreamRefWrapper> info;
		std::vector<StreamRefWrapper> debug;
		std::vector<StreamRefWrapper> error;

		template <LogLevel L>
		constexpr std::vector<StreamRefWrapper>& GetStreamsFromLevel();
	};

	template <typename T, typename CharT>
	concept Formattable = requires() {
		std::formatter<T, CharT>();
	};

	template <typename T>
	concept LogLevelDependent = requires(T t, LogLevel l) {
		{ t.SetLogLevel(l) } -> std::same_as<void>;
	};

	struct LogLevelSpecifier {
		LogLevel level{LogLevel::INFO};

		constexpr void SetLogLevel(LogLevel level);
	};

	template <typename T>
	concept MessageDependent = requires(T t, std::string_view s) {
		{ t.SetMsg(s) } -> std::same_as<void>;
	};

	struct MessageSpecifier {
		std::string msg;

		constexpr void SetMsg(std::string_view msg);
	};

	template <typename T>
	concept SourceLocationDependent = requires(T t, std::source_location loc) {
		{ t.SetSourceLocation(loc) } -> std::same_as<void>;
	};

	struct SourceLocationSpecifier {
		std::source_location loc;

		constexpr void SetSourceLocation(std::source_location loc);
	};

	struct TimeSpecifier {};

	template <LoggableCharType CharT = char, typename... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	class Logger {
	public:
		using FmtPack = type_traits::Typepack<FmtTypes...>;
		using StreamsType = LogStreams<CharT>;

		template <LogLevel L>
		class Streamable {
		public:
			bool IsLoggerAlive();
			void UpdateSourceLocation(std::source_location loc = std::source_location::current());

			template <typename T>
			Streamable& operator<<(const T& t);

			Streamable& operator<<(typename StreamsType::StreamType&(*f)(typename StreamsType::StreamType&));

			~Streamable();
		private:
			Streamable(Logger& logger, std::source_location loc, std::weak_ptr<bool> logger_alive);

			Logger& logger_;
			std::source_location loc_;
			std::weak_ptr<bool> logger_alive_;
			std::basic_ostringstream<CharT> msg_buf_{};

			friend class Logger;
		};

		Logger(StreamsType&& streams, std::format_string<FmtTypes&...> fmt, FmtTypes&&... specifiers);

		void Info(std::string_view msg, std::source_location loc = std::source_location::current());
		void Debug(std::string_view msg, std::source_location loc = std::source_location::current());
		void Error(std::string_view msg, std::source_location loc = std::source_location::current());

		template <LogLevel L>
		void Log(std::string_view msg, std::source_location loc = std::source_location::current());

		template <LogLevel L>
		Streamable<L> MakeStream(std::source_location loc = std::source_location::current());

		template <std::size_t Index> requires(Index < sizeof...(FmtTypes))
		std::tuple_element_t<Index, std::tuple<FmtTypes...>>& GetSpecifierAtIndex();

		template <typename T>
		std::array<T&, FmtPack::template TypeCount<T>> GetSpecifiersOfType();

		template <template <typename> typename P>
		auto GetSpecifiersOfPred();

		~Logger();
	private:
		StreamsType streams_;
		std::format_string<FmtTypes&...> fmt_;
		std::tuple<FmtTypes...> specifiers_;
		std::shared_ptr<bool> alive_;


		void SetFmtLogLevel(LogLevel level);
		void SetFmtMsg(std::string_view msg);
		void SetFmtSrcLoc(std::source_location loc);

		template <std::size_t... Is>
		void SetFmtLogLevelImpl(LogLevel level, std::integer_sequence<size_t, Is...>);

		template <std::size_t... Is>
		void SetFmtMsgImpl(std::string_view msg, std::integer_sequence<size_t, Is...>);

		template <std::size_t... Is>
		void SetFmtSrcLocImpl(std::source_location loc, std::integer_sequence<size_t, Is...>);
	};
}

#include "Logger.tcc"
#endif