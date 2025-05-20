#pragma once
#include "HAL/PlatformType.h"
#include <type_traits>

template <typename T>
FORCEINLINE constexpr T Align(T Val, uint64 Alignment)
{
    static_assert(std::is_integral_v<T> || std::is_pointer_v<T>,
        "Align expects an integer or pointer type");

    // 포인터라면 uintptr_t로 변환하여 연산
    using UIntPtr = std::conditional_t<std::is_pointer_v<T>, std::uintptr_t, std::uint64_t>;
    UIntPtr V = static_cast<UIntPtr>(Val);
    UIntPtr Aligned = (V + Alignment - 1) & ~(Alignment - 1);
    return static_cast<T>(Aligned);
}
