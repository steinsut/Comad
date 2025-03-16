#ifndef COMAD_VALUE_UTILS_TCC_
#define COMAD_VALUE_UTILS_TCC_

#include <set>
#include <stdexcept>

#include "Value.h"

namespace comad::value {
	template <ValidType T>
	ValueWrapper::ValueWrapper(T t) :
		type_{ ValueTypeTraits<T>::type },
		value_{ t } {}

	template <ValidType T>
	auto ValueWrapper::GetValue() -> T& {
		if (type_ != ValueTypeTraits<T>::type) {
			throw std::runtime_error("value wrapper does not hold the requested type");
		}
		return std::get<ValueTypeTraits<T>::variant_index>(value_);
	}

	template <ValidType T>
	auto ValueWrapper::GetValue() const -> const T& {
		if (type_ != ValueTypeTraits<T>::type) {
			throw std::runtime_error("value wrapper does not hold the requested type");
		}
		return std::get<ValueTypeTraits<T>::variant_index>(value_);
	}

	template<ValidType T>
	ValueBounds::ValueBounds(T t1, T t2) :
		type_{ ValueTypeTraits<T>::type },
		min_{ t1 },
		max_{ t2 }
	{
		if (t2 < t1) std::swap(min_, max_);
	}

	bool ValueBounds::IsInBounds(const ValidType auto& t) const {
		using type = std::remove_cvref_t<decltype(t)>;

		if (type_ != ValueTypeTraits<type>::type) {
			throw std::invalid_argument("bounds are not for the passed type");
		}

		return min_.GetValue<type>() < t &&
			t < max_.GetValue<type>();
	}

	SupportedValueHolder::SupportedValueHolder(ValueRange auto&& values) :
		supported_values_{ List(std::forward<decltype(values)>(values)) }
	{}

	template <ValidType T, ValidType... TOthers> requires (std::conjunction_v<std::is_same<T, TOthers>...>)
	SupportedValueHolder::SupportedValueHolder(T value, TOthers ...others) :
		SupportedValueHolder(std::set<T>{ { value, std::forward<TOthers>(others)... } })
	{}

	bool SupportedValueHolder::IsValid(const ValidType auto& t) const noexcept {
		if (const ValueType* type = std::get_if<ValueType>(&supported_values_)) {
			return *type == ValueTypeTraits<decltype(t)>::type;
		}
		if (const ValueBounds* bounds = std::get_if<ValueBounds>(&supported_values_)) {
			return bounds->IsInBounds(t);
		}
		return std::get<List>(supported_values_).IsValid(t);
	}

	bool SupportedValueHolder::List::IsValid(const ValidType auto& t) const noexcept {
		if (value_type_ == ValueTypeTraits<decltype(t)>::type) {
			return contains_(supported_values_, &t);
		}
		return false;
	}

	SupportedValueHolder::List::List(ValueRange auto&& range) :
		supported_values_{ range },
		value_type_{ ValueTypeTraits<std::ranges::range_value_t<decltype(range)>>::type },
		contains_{ [](const std::any& supported_values, const void* object) -> bool {
			using range_type = std::remove_cvref_t<decltype(range)>;
			using value_type = std::ranges::range_value_t<range_type>;

			const value_type& val = *static_cast<const value_type*>(object);
			const range_type& range = std::any_cast<const range_type&>(supported_values);

			return std::ranges::find(range, val) != range.end();
		} }
	{}
}

#endif