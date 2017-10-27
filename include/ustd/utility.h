#pragma once

#include <utility>

namespace ustd {

template <typename T, typename U>
constexpr T the(U&& u) {
  return std::forward<U>(u);
}

} // namespace ustd