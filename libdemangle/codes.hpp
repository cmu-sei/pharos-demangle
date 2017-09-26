// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#ifndef Include_codes_hpp
#define Include_codes_hpp

namespace demangle {

#define CODE_ENUM(e, s) e

enum class Code : unsigned {
  #include "code_data.hpp"
};

char const * code_string(Code c);

template <typename T>
T & operator<<(T & s, Code c) {
  return s << code_string(c);
}

} // namespace demangle

#endif  // Include_codes_hpp

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
