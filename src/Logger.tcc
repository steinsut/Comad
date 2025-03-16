#ifndef COMAD_LOGGER_TCC_
#define COMAD_LOGGER_TCC_

#include <chrono>
#include <iostream>
#include <regex>
#include <string>
#include <source_location>

#include "Logger.h"
#include "TypeTraits.h"

template <comad::logger::LoggableCharType CharT>
struct std::formatter<comad::logger::LogLevelSpecifier, CharT> {
	char casing = 'U';

	template<typename ParseContext>
	constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
		using namespace std::string_view_literals;

		auto it = ctx.begin();
		if (it == ctx.end()) return it;

		if ("UML"sv.find(*it) != std::string_view::npos) {
			casing = *it;
			++it;
		}

		if (it != ctx.end() && *it != '}') {
			throw std::format_error("invalid format args for log level specifier");
		}

		return it;
	}

	template<typename FormatContext>
	constexpr typename FormatContext::iterator format(comad::logger::LogLevelSpecifier spec, FormatContext& ctx) const {
		using namespace std::string_literals;

		std::string str;
		switch (spec.level) {
			case comad::logger::LogLevel::INFO:
				str = "INFO"s;
				break;
			case comad::logger::LogLevel::DEBUG:
				str = "DEBUG"s;
				break;
			case comad::logger::LogLevel::ERROR:
				str = "ERROR"s;
				break;
			default:
				str = "UNKNOWN"s;
				break;
		}

		switch (casing) {
			case 'L': std::transform(str.begin(), str.end(), str.begin(), ::tolower); break;
			case 'M': std::transform(str.begin() + 1, str.end(), str.begin() + 1, ::tolower); break;
			default: break;
		}
		return std::ranges::copy(str, ctx.out()).out;
	}
};

template <comad::logger::LoggableCharType CharT>
struct std::formatter<comad::logger::MessageSpecifier, CharT> {
	template<typename ParseContext>
	static constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
		return ctx.begin();
	}

	template<typename FormatContext>
	static constexpr typename FormatContext::iterator format(const comad::logger::MessageSpecifier& spec, FormatContext& ctx) {
		return std::ranges::copy(spec.msg, ctx.out()).out;
	}
};

template <comad::logger::LoggableCharType CharT>
struct std::formatter<comad::logger::TimeSpecifier, CharT> {
	std::formatter<std::chrono::zoned_time<std::chrono::system_clock::duration>, CharT> fmt;

	template<typename ParseContext>
	constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
		return fmt.parse(ctx);
	}

	template<typename FormatContext>
	typename FormatContext::iterator format(comad::logger::TimeSpecifier& spec, FormatContext& ctx) const {
		return fmt.format(std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now() }, ctx);
	}
};


template <comad::logger::LoggableCharType CharT>
struct std::formatter<comad::logger::SourceLocationSpecifier, CharT> {
	std::vector<std::pair<char, char>> replacements{};

	template<typename ParseContext>
	constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
		auto it = ctx.begin();

		for (; it != ctx.end() && *it != '}'; it = std::next(it)) {
			char c = *it;

			auto it_next = std::next(it);
			if (c == '%' && it_next != ctx.end()) {
				char next = *it_next;

				if ("FLCD"sv.find(next) == std::string_view::npos) {
					throw std::format_error("invalid format args for source location specifier");
				}

				replacements.emplace_back(next, 0);

				it = it_next;
			}
			else if (":/"sv.find(c) != std::string_view::npos) {
				if (replacements.empty() || replacements[replacements.size() - 1].second != 0) {
					throw std::format_error("invalid format args for source location specifier");
				}
				replacements[replacements.size() - 1].second = c;
			}
			else {
				throw std::format_error("invalid format args for source location specifier");
			}
		}

		return it;
	}

	template<typename FormatContext>
	constexpr typename FormatContext::iterator format(const comad::logger::SourceLocationSpecifier& spec, FormatContext& ctx) const {
		std::basic_ostringstream<CharT> stream;
		if (replacements.empty()) {
			stream << spec.loc.file_name() << ':' << spec.loc.function_name() << ':' << spec.loc.line() << ':' << spec.loc.column();
		}
		else {
			for (auto& p: replacements) {
				switch (p.first) {
					case 'F': stream << spec.loc.function_name(); break;
					case 'L': stream << spec.loc.line(); break;
					case 'C': stream << spec.loc.column(); break;
					case 'D': stream << spec.loc.file_name(); break;
					default: break;
				}
				if (p.second != 0) stream << p.second;
			}
		}

		return std::ranges::copy(std::move(stream).str(), ctx.out()).out;
	}
};

namespace comad::logger {
	CONCEPT_TO_PRED(log_level_pred, LogLevelDependent);
	CONCEPT_TO_PRED(msg_pred, MessageDependent);
	CONCEPT_TO_PRED(src_loc_pred, SourceLocationDependent);

	constexpr void LogLevelSpecifier::SetLogLevel(LogLevel level) { this->level = level; }

	constexpr void MessageSpecifier::SetMsg(std::string_view msg) { this->msg = msg; }

	constexpr void SourceLocationSpecifier::SetSourceLocation(std::source_location loc) { this->loc = loc; }

	template<LoggableCharType CharT>
	template<LogLevel L>
	constexpr std::vector<typename LogStreams<CharT>::StreamRefWrapper> & LogStreams<CharT>::GetStreamsFromLevel() {
		if constexpr (L == LogLevel::INFO) {
			return info;
		}
		else if constexpr (L == LogLevel::DEBUG) {
			return debug;
		}
		else if constexpr (L == LogLevel::ERROR) {
			return info;
		}
		else {
			throw std::runtime_error{"log level not supported"};
		}
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template <LogLevel L>
	Logger<CharT, FmtTypes...>::Streamable<L>::Streamable(Logger& logger,
		std::source_location loc,
		std::weak_ptr<bool> logger_alive): logger_(logger), loc_{loc}, logger_alive_{std::move(logger_alive)} {
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template<LogLevel L>
	void Logger<CharT, FmtTypes...>::Streamable<L>::UpdateSourceLocation(std::source_location loc) {
		loc_ = loc;
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template <LogLevel L>
	template<typename T>
	typename Logger<CharT, FmtTypes...>::template Streamable<L>& Logger<CharT, FmtTypes...>::Streamable<L>::operator<<(const T& t) {
		if (logger_alive_.expired() || !*logger_alive_.lock()) {
			throw std::runtime_error("cant stream to a logger that has been destroyed");
		}

		if constexpr (L == LogLevel::DEBUG) {
#ifdef NDEBUG
			return *this;
#endif
		}

		msg_buf_ << t;

		return *this;
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template<LogLevel L>
	typename Logger<CharT, FmtTypes...>::template Streamable<L> & Logger<CharT, FmtTypes...>::Streamable<L>::operator
	<<(typename StreamsType::StreamType &(*f)(typename StreamsType::StreamType &)) {
		if (logger_alive_.expired() || !*logger_alive_.lock()) {
			throw std::runtime_error("cant stream to a logger that has been destroyed");
		}

		if constexpr (L == LogLevel::DEBUG) {
#ifdef NDEBUG
			return *this;
#endif
		}

		if (f == &std::flush<CharT, std::char_traits<CharT>> || f == &std::endl<CharT, std::char_traits<CharT>>) {
			logger_.Log<L>(msg_buf_.view(), loc_);
			msg_buf_.clear();
			msg_buf_.str(std::string{});
		}
		else {
			msg_buf_ << f;
		}

		return *this;
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template<LogLevel L>
	Logger<CharT, FmtTypes...>::Streamable<L>::~Streamable() {
		if constexpr (L == LogLevel::DEBUG) {
#ifdef NDEBUG
			return;
#endif
		}

		if (!logger_alive_.expired() && *logger_alive_.lock()  && msg_buf_.view().size() > 0) {
			logger_.Log<L>(msg_buf_.view(), loc_);
		}
	}

	template <LoggableCharType CharT, typename... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	Logger<CharT, FmtTypes...>::Logger(
		StreamsType&& streams,
		std::format_string<FmtTypes&...> fmt,
		FmtTypes&&... specifiers
	)
	:
		streams_{ streams },
		fmt_{ std::move(fmt) },
		specifiers_{ std::forward<FmtTypes>(specifiers)... },
		alive_(std::make_shared<bool>(true))
	{

	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	void Logger<CharT, FmtTypes...>::Info(std::string_view msg, std::source_location loc) {
		Log<LogLevel::INFO>(msg, loc);
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
		void Logger<CharT, FmtTypes...>::Debug(std::string_view msg, std::source_location loc) {
#ifndef NDEBUG
		Log<LogLevel::DEBUG>(msg, loc);
#endif
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	void Logger<CharT, FmtTypes...>::Error(std::string_view msg, std::source_location loc) {
		Log<LogLevel::ERROR>(msg, loc);
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template <LogLevel L>
	void Logger<CharT, FmtTypes...>::Log(std::string_view msg, std::source_location loc) {
		SetFmtLogLevel(L);
		SetFmtMsg(msg);
		SetFmtSrcLoc(loc);
		std::string formatted = std::apply([&] (FmtTypes&... specifiers) {
			return std::format(fmt_, specifiers...);
		}, specifiers_);
		if constexpr (L == LogLevel::DEBUG) {
#ifdef NDEBUG
			return
#endif
		}

		for (typename StreamsType::StreamRefWrapper s: streams_.template GetStreamsFromLevel<L>()) {
			s.get() << formatted << '\n';
		}
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template<LogLevel L>
	typename Logger<CharT, FmtTypes...>::template Streamable<L> Logger<CharT, FmtTypes...>::MakeStream(std::source_location loc) {
		return Streamable<L>{ *this, loc, alive_ };
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template<std::size_t Index> requires (Index < sizeof...(FmtTypes))
	std::tuple_element_t<Index, std::tuple<FmtTypes...>> & Logger<CharT, FmtTypes...>::GetSpecifierAtIndex() {
		return std::get<Index>(specifiers_);
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template<typename T>
	std::array<T &, type_traits::Typepack<FmtTypes...>::template TypeCount<T>> Logger<CharT, FmtTypes...>::
	GetSpecifiersOfType() {
		using Seq = typename FmtPack::template TypeIndexSequence<T>;

		return utility::MapArrayFromTupleSequence(specifiers_, Seq{});
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template <template <typename> typename P>
	auto Logger<CharT, FmtTypes...>::GetSpecifiersOfPred() {
		using Seq = typename FmtPack::template PredIndexSequence<P>;

		return utility::MapTupleFromTupleSequence(specifiers_, Seq{});
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	Logger<CharT, FmtTypes...>::~Logger() {
		*alive_ = false;
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	void Logger<CharT, FmtTypes...>::SetFmtLogLevel(LogLevel level) {
		using Seq = typename FmtPack::template PredIndexSequence<log_level_pred>;

		if constexpr (Seq::size() > 0) {
			SetFmtLogLevelImpl(level, Seq{});
		}
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	void Logger<CharT, FmtTypes...>::SetFmtMsg(std::string_view msg) {
		using Seq = typename FmtPack::template PredIndexSequence<msg_pred>;

		if constexpr (Seq::size() > 0) {
			SetFmtMsgImpl(msg, Seq{});
		}
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	void Logger<CharT, FmtTypes...>::SetFmtSrcLoc(std::source_location loc) {
		using Seq = typename FmtPack::template PredIndexSequence<src_loc_pred>;

		if constexpr (Seq::size() > 0) {
			SetFmtSrcLocImpl(loc, Seq{});
		}
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template <std::size_t... Is>
	void Logger<CharT, FmtTypes...>::SetFmtLogLevelImpl(LogLevel level, std::integer_sequence<size_t, Is...>) {
		(std::get<Is>(specifiers_).SetLogLevel(level),...);
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template <std::size_t... Is>
	void Logger<CharT, FmtTypes...>::SetFmtMsgImpl(std::string_view msg, std::integer_sequence<size_t, Is...>) {
		(std::get<Is>(specifiers_).SetMsg(msg),...);
	}

	template<LoggableCharType CharT, typename ... FmtTypes> requires (Formattable<FmtTypes, CharT> && ...)
	template <std::size_t... Is>
	void Logger<CharT, FmtTypes...>::SetFmtSrcLocImpl(std::source_location loc, std::integer_sequence<size_t, Is...>) {
		(std::get<Is>(specifiers_).SetSourceLocation(loc),...);
	}

	inline Logger comad_logger{
					{
						.info = {std::ref(std::cout)},
						.debug = {std::ref(std::cout)},
						.error = {std::ref(std::cerr)}
					},
					"comad: [{0:M}] ({1:%F}T{1:%R%z}) {2:%F:%L:%C}: {3}",
					LogLevelSpecifier{},
					TimeSpecifier{},
					SourceLocationSpecifier{},
					MessageSpecifier{}
	};
}

#endif