#ifndef Include_codes_hpp
#define Include_codes_hpp

namespace demangle {

#define ENUM(e, s) e

enum class Code : unsigned {
  #include "code_data.hpp"
};

char const * code_string(Code c);

template <typename T>
T & operator<<(T s, Code c) {
  return s << code_string(c);
}

} // namespace demangle

#endif  // Include_codes_hpp
