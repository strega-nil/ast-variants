#include <ustd/utility.h>

#include <iostream>

namespace ustd {
namespace utility {
  void wait_until_enter() {
    std::cout << "Please press enter to continue\n";
    char c[2];
    std::cin.getline(c, 1);
  }
} // namespace utility
} // namespace ustd