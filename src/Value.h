#ifndef COMAD_VALUE_TRAITS_H_
#define COMAD_VALUE_TRAITS_H_

#include <map>
#include <variant>
#include <concepts>
#include <string>
#include <string_view>
#include <cstddef>

namespace comad {
	template<typename T>
	concept CVRef = std::is_reference_v<T>
		|| std::is_const_v<T>
		|| std::is_volatile_v<T>;

	namespace value {
		enum class ValueType: std::size_t {
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
			static constexpr ValueType type = ValueType::kBool;
			static constexpr std::size_t variant_index = ValueVariant{ false }.index();
			static constexpr std::string_view name = std::string_view{"bool"};
		};

		template<>
		struct ValueTypeTraits<int> {
			static constexpr ValueType type = ValueType::kInt;
			static constexpr std::size_t variant_index = ValueVariant{ 0 }.index();
			static constexpr std::string_view name = std::string_view{"int"};
		};

		template<>
		struct ValueTypeTraits<float> {
			static constexpr ValueType type = ValueType::kFloat;
			static constexpr std::size_t variant_index = ValueVariant{ .0f }.index();
			static constexpr std::string_view name = std::string_view{"float"};
		};

		template<>
		struct ValueTypeTraits<std::string> {
			static constexpr ValueType type = ValueType::kString;
			static constexpr std::size_t variant_index = ValueVariant{ "" }.index();
			static constexpr std::string_view name = std::string_view{"string"};
		};

		template<typename T>
		concept ValidType = requires() {
			{ ValueTypeTraits<T>::type } -> std::convertible_to<ValueType>;
			{ ValueTypeTraits<T>::variant_index } -> std::convertible_to<std::size_t>;
			{ ValueTypeTraits<T>::name } -> std::convertible_to<std::string_view>;
		};

		template <typename T>
		concept ValueRange = std::ranges::input_range<T> &&
			ValidType<std::ranges::range_value_t<T>>;

		inline const std::map<ValueType, std::string_view> ValueTypeNames = []<typename... Ts>(std::variant<Ts...>) {
			std::map<ValueType, std::string_view> ret;

			(ret.emplace(ValueTypeTraits<Ts>::type, ValueTypeTraits<Ts>::name), ...);

			return ret;
		}(ValueVariant{});
	}
}

#endif