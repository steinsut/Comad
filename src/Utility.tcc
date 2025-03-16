#ifndef COMAD_UTILITY_TCC
#define COMAD_UTILITY_TCC

#include "Utility.h"

namespace comad::utility {
    template <std::integral IntT, IntT... Is>
    constexpr auto IntegerSequenceToArray(std::integer_sequence<IntT, Is...>) -> std::array<IntT, sizeof...(Is)> {
        return {Is...};
    }

    template <std::integral IntT, IntT... Is, class T>
    constexpr auto MapArrayFromTupleSequence(T& tuple, std::integer_sequence<IntT, Is...>) -> std::array<T&, sizeof...(Is)> {
        return { std::get<Is>(tuple)... };
    }

    template <std::integral IntT, IntT... Is, class T>
    constexpr auto MapTupleFromTupleSequence(T& tuple, std::integer_sequence<IntT, Is...>) {
        return std::tie(static_cast<std::tuple_element_t<Is, T>&>(std::get<Is>(tuple))...);
    }
}

#endif