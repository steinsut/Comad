#ifndef COMAD_COMMAND_TCC_
#define COMAD_COMMAND_TCC_

#include <set>
#include <type_traits>

#include "Command.h"

namespace comad::command {
	CommandOption& CommandOption::operator()(value::ValueRange auto&& range) {
		supported_values = value::SupportedValueHolder{ std::forward<decltype(range)>(range) };

		return *this;
	}

	template<value::ValidType T, value::ValidType ...TOthers> requires (std::conjunction_v<std::is_same<T, TOthers>...>)
	CommandOption& CommandOption::operator()(T value, TOthers ...others) {
		supported_values = value::SupportedValueHolder{ std::set{ { value, std::forward<TOthers>(others)... } } };

		return *this;
	}
}

#endif