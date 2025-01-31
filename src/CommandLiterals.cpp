#include "CommandLiterals.h"

namespace comad {
	namespace literals {
		CommandOptionLiteral& CommandOptionLiteral::operator[](bool is_required) {
			option_[is_required];

			return *this;
		}
	}
}