#include "codes.hpp"
#include <vector>

namespace demangle {

namespace {

#define ENUM(e, s) s

std::vector<char const *> code_strings = {
  #include "code_data.hpp"
};

} // unnamed namespace

char const * code_sring(Code c) {
  auto v = static_cast<decltype(code_strings)::size_type>(c);
  return code_strings.at(v);
}

} // namespace demangle
