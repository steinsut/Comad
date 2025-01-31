#include "ValueUtils.h"

namespace comad {
	namespace value {
		SupportedValueHolder::SupportedValueHolder(ValueType type) :
			supported_values_{ type }
		{}

		SupportedValueHolder::SupportedValueHolder(ValueBounds bounds) :
			supported_values_{ bounds }
		{}

		ValueType SupportedValueHolder::GetValueType() const noexcept {
			if (const ValueType* type = std::get_if<ValueType>(&supported_values_)) {
				return *type;
			}
			else if (const ValueBounds* bounds = std::get_if<ValueBounds>(&supported_values_)) {
				return bounds->GetValueType();
			}
			else {
				return std::get<List>(supported_values_).GetValueType();
			}
		}

		ValueType SupportedValueHolder::List::GetValueType() const noexcept {
			return value_type_;
		}
		
		ValueType ValueWrapper::GetType() const noexcept {
			return type_;
		}

		ValueType ValueBounds::GetValueType() const noexcept {
			return type_;
		}

	}
}