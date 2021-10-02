#pragma once

#include <type_traits>

template<typename T>
[[__nodiscard__]]
constexpr typename std::remove_reference_t<T> && mv(T && v) noexcept {
    return static_cast<typename std::remove_reference_t<T> &&>(v);
}
