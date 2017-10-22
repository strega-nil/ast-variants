#pragma once

#include <utility>

namespace ustd {
namespace utility {
  void wait_until_enter();
} // namespace utility

template <typename T, typename U>
T as(U&& u) {
  return std::forward<U>(u);
}

} // namespace ustd