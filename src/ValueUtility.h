#ifndef COMAD_VALUE_UTILITY_H_
#define COMAD_VALUE_UTILITY_H_

#include <any>
#include <variant>

#include "Value.h"

namespace comad::value {
	class ValueWrapper {
	public:
		template<ValidType T>
		explicit ValueWrapper(T t);

		template <ValidType T>
		auto GetValue() -> T&;

		template <ValidType T>
		auto GetValue() const -> const T&;

		[[nodiscard]] ValueType GetType() const noexcept;

	private:
		ValueType type_{ ValueType::kUnknown };
		ValueVariant value_;
	};

	class ValueBounds {
	public:
		template <ValidType T>
		ValueBounds(T t1, T t2);

		bool IsInBounds(const ValidType auto& t) const;

		[[nodiscard]] ValueType GetValueType() const noexcept;
	private:
		ValueType type_;
		ValueWrapper min_;
		ValueWrapper max_;
	};

	class SupportedValueHolder {
	public:
		SupportedValueHolder() = default;
		explicit SupportedValueHolder(ValueType type);
		explicit SupportedValueHolder(ValueBounds bounds);
		explicit SupportedValueHolder(ValueRange auto&& values);

		template <ValidType T, ValidType... TOthers> requires (std::conjunction_v<std::is_same<T, TOthers>...>)
		explicit SupportedValueHolder(T value, TOthers... others);

		bool IsValid(const ValidType auto& t) const noexcept;

		[[nodiscard]] ValueType GetValueType() const noexcept;

	private:
		class List {
		public:
			explicit List(ValueRange auto&& range);

			bool IsValid(const ValidType auto& t) const noexcept;

			[[nodiscard]] ValueType GetValueType() const noexcept;

		private:
			std::any supported_values_{ nullptr };
			ValueType value_type_{ ValueType::kUnknown };
			bool (*contains_)(const std::any& supported_values, const void* object);
		};

		std::variant<ValueType, ValueBounds, List> supported_values_{ ValueType::kUnknown };
	};
}


#include "ValueUtility.tcc"
#endif