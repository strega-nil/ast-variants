#include <iostream>
#include <ustd/utility.h>

#include <lambda.h>

using namespace ustd;

using lambda::term;

int main() {
  auto tm = the<term::fat>(term::var("x"));
  auto ast = lambda::finish(tm);
}
