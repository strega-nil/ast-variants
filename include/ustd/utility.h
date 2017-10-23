#pragma once

#include <utility>

namespace ustd {
namespace utility {
  void wait_until_enter();
} // namespace utility

template <typename T, typename U>
constexpr T the(U&& u) {
  return std::forward<U>(u);
}

} // namespace ustd