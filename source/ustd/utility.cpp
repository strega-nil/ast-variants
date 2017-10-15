#include <ustd/utility.h>

#include <iostream>

namespace ustd::utility {
void wait_until_enter() {
  std::cout << "Please press enter to continue\n";
  char c[2];
  std::cin.getline(c, 1);
}
}