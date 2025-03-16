#ifndef COMAD_TYPE_TRAITS_H_
#define COMAD_TYPE_TRAITS_H_

#include <type_traits>

#define CONCEPT_TO_PRED(_name, _concept) template <typename T> struct _name: std::bool_constant<false> {};\
template <_concept T> struct _name<T>: std::bool_constant<true> {}

#include <array>
#include <cstddef>
#include <concepts>

#include "Utility.h"

namespace comad::type_traits {
	namespace detail {
		template <typename T> struct is_same_pred {
			template <typename S>
			struct pred: std::is_same<S, T> {};
		};

		template <std::integral IntT, template <typename> typename P, std::size_t N, std::size_t I, typename T, typename... Pack>
		struct _integer_sequence_from_pack_impl {
			static_assert(I != sizeof...(Pack), "size mismatch");

			using type = utiliy::combine_integer_sequence_v<
				std::conditional_t<P<T>::value, std::integer_sequence<IntT, N - I>, void>,
				typename _integer_sequence_from_pack_impl<IntT, P, N, I - 1, Pack...>::type,
				IntT>;
		};

		template <std::integral IntT, template <typename> typename P, std::size_t N, typename T>
		struct _integer_sequence_from_pack_impl<IntT, P, N, 1, T> {
			using type = std::conditional_t<P<T>::value, std::integer_sequence<IntT, N - 1>, void>;
		};

		template <std::integral IntT, template <typename> typename P, typename... Pack>
		struct integer_sequence_from_pack : _integer_sequence_from_pack_impl<IntT, P, sizeof...(Pack), sizeof...(Pack), Pack...> {
			static_assert(sizeof...(Pack) > 0, "pack size must be greater than zero");
		};

		template <std::integral IntT, template <typename> typename P, typename... Pack>
		using integer_sequence_from_pack_v = integer_sequence_from_pack<IntT, P, Pack...>::type;
	}

	template <typename... Pack>
	struct Typepack {
		template <template <typename> typename P>
		static constexpr std::size_t PredCount = (P<Pack>::value + ...);

		template <template <typename> typename P, std::integral IntT = std::size_t>
		using PredIndexSequence = detail::integer_sequence_from_pack_v<IntT, P, Pack...>;

		template <template <typename> typename P, std::integral IntT = std::size_t>
		static constexpr std::array<IntT, PredCount<P>> PredIndices =
			utiliy::IntegerSequenceToArray(PredIndexSequence<P, IntT>{});

		template <typename T>
		static constexpr std::size_t TypeCount = PredCount<detail::is_same_pred<T>::template pred>;

		template <typename T, std::integral IntT = std::size_t>
		using TypeIndexSequence = detail::integer_sequence_from_pack_v<IntT, detail::is_same_pred<T>::template pred, Pack...>;

		template <typename T, std::integral IntT = std::size_t>
		static constexpr std::array<IntT, TypeCount<T>> TypeIndices =
			utiliy::IntegerSequenceToArray(PredIndexSequence<detail::is_same_pred<T>::template pred, IntT>{});
	};
}

#endif