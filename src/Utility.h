#ifndef COMAD_UTILITY_H
#define COMAD_UTILITY_H

namespace comad::utiliy {
    template <typename Seq1,typename Seq2, std::integral IntT>
    struct combine_integer_sequence {
        static_assert(false, "invalid sequence types");
    };

    template <std::integral IntT, template <typename, IntT...> typename Seq1, template <typename, IntT...> typename Seq2, IntT... Ints1, IntT... Ints2>
    struct combine_integer_sequence<Seq1<IntT, Ints1...>, Seq2<IntT, Ints2...>, IntT> {
        using value = std::integer_sequence<IntT, Ints1..., Ints2...>;
    };

    template <std::integral IntT, template <typename, IntT...> typename Seq1, IntT... Ints1>
    struct combine_integer_sequence<Seq1<IntT, Ints1...>, void, IntT> {
        using value = std::integer_sequence<IntT, Ints1...>;
    };

    template <std::integral IntT, template <typename, IntT...> typename Seq1, IntT... Ints1>
    struct combine_integer_sequence<void, Seq1<IntT, Ints1...>, IntT> {
        using value = std::integer_sequence<IntT, Ints1...>;
    };

    template <std::integral IntT>
    struct combine_integer_sequence<void, void, IntT> {
        using value = std::integer_sequence<IntT>;
    };

    template <typename Seq1, typename Seq2, std::integral IntT>
    using combine_integer_sequence_v = typename combine_integer_sequence<Seq1, Seq2, IntT>::value;

    template <std::integral IntT, IntT... Is>
    constexpr auto IntegerSequenceToArray(std::integer_sequence<IntT, Is...>) -> std::array<IntT, sizeof...(Is)>;

    template <std::integral IntT, IntT... Is, class T>
    constexpr auto MapArrayFromTupleSequence(T& tuple, std::integer_sequence<IntT, Is...>) -> std::array<T&, sizeof...(Is)>;

    template <std::integral IntT, IntT... Is, class T>
    constexpr auto MapTupleFromTupleSequence(T& tuple, std::integer_sequence<IntT, Is...>);
}


#include "Utility.tcc"
#endif //COMAD_UTILITY_H
