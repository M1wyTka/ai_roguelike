#pragma once

template<typename E>
constexpr auto to_underlying(E e) -> std::underlying_type<E>::type
{
    return static_cast<std::underlying_type<E>::type>(e);
}

