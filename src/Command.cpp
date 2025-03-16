#include "Command.h"

namespace comad::command {
	using namespace value;

	CommandOption& CommandOption::operator[](bool is_required) noexcept {
		required = is_required;

		return *this;
	}

	CommandOption& CommandOption::operator()(ValueType type) noexcept {
		supported_values = SupportedValueHolder(type);

		return *this;
	}
	CommandOption& CommandOption::operator()(ValueBounds bounds) noexcept {
		supported_values = SupportedValueHolder(std::move(bounds));

		return *this;
	}
}
