// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#include "codes.hpp"
#include <vector>

namespace demangle {

namespace {

#define CODE_ENUM(e, s) s

std::vector<char const *> code_strings = {
  #include "code_data.hpp"
};

} // unnamed namespace

char const * code_string(Code c) {
  auto v = static_cast<decltype(code_strings)::size_type>(c);
  return code_strings.at(v);
}

} // namespace demangle

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
