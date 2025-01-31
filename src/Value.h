#ifndef COMAD_VALUE_TRAITS_H_
#define COMAD_VALUE_TRAITS_H_

#include <any>
#include <variant>
#include <concepts>
#include <string>
#include <variant>

namespace comad {
	template<typename T>
	concept CVRef = std::is_reference_v<T>
		|| std::is_const_v<T>
		|| std::is_volatile_v<T>;

	namespace value {
		enum class ValueType {
			kUnknown,
			kBool,
			kInt,
			kFloat,
			kString
		};

		using ValueVariant = std::variant<bool, int, float, std::string>;

		template<typename T>
		struct ValueTypeTraits;

		template<CVRef T>
		struct ValueTypeTraits<T> : ValueTypeTraits<std::remove_cvref_t<T>> {};

		template<>
		struct ValueTypeTraits<bool> {
			static constexpr value::ValueType type = value::ValueType::kBool;
			static constexpr int variant_index = 0;
		};

		template<>
		struct ValueTypeTraits<int> {
			static constexpr value::ValueType type = value::ValueType::kInt;
			static constexpr int variant_index = 1;
		};

		template<>
		struct ValueTypeTraits<float> {
			static constexpr value::ValueType type = value::ValueType::kFloat;
			static constexpr int variant_index = 2;
		};

		template<>
		struct ValueTypeTraits<std::string> {
			static constexpr value::ValueType type = value::ValueType::kString;
			static constexpr int variant_index = 3;
		};

		template<typename T>
		concept ValidType = requires(T t) {
			{ ValueTypeTraits<T>::type } -> std::convertible_to<value::ValueType>;
			{ ValueTypeTraits<T>::variant_index } -> std::convertible_to<int>;
		};

		template <typename T>
		concept ValueRange = std::ranges::input_range<T> &&
			ValidType<std::ranges::range_value_t<T>>;
	}
}

#endif