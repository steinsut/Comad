#include "Command.h"

namespace comad {
	namespace command {
		using namespace value;

		CommandOption& CommandOption::operator[](bool is_required) noexcept {
			required = is_required;

			return *this;
		}

		CommandOption& CommandOption::operator()(value::ValueType type) noexcept {
			supported_values = type;

			return *this;
		}
		CommandOption& CommandOption::operator()(value::ValueBounds bounds) noexcept {
			supported_values = bounds;

			return *this;
		}
	}
}