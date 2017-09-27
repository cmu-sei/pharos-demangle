// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#ifndef Include_demangle_text
#define Include_demangle_text

#include "demangle.hpp"
#include <ostream>              // std::ostream

namespace demangle {

class TextOutput {
 public:
  enum Attribute {
    // Output class template parameters on ctors and dtors
    CDTOR_CLASS_TEMPLATE_PARAMETERS = 1,
    // Output Microsoft legacy named names for [u]intX_t simple types
    MS_SIMPLE_TYPES                 = 2
  };

  class Attributes {
   public:
    Attributes() {}

    Attributes & set(Attribute a) {
      val |= static_cast<decltype(val)>(a);
      return *this;
    }
    Attributes & unset(Attribute a) {
      val &= ~static_cast<decltype(val)>(a);
      return *this;
    }

    bool operator[](Attribute a) const {
      return val & static_cast<decltype(val)>(a);
    }

   private:
    uint32_t val = 0;
  };

  static Attributes undname() {
    return Attributes();
  };

 public:
  TextOutput() = default;
  TextOutput(Attributes a) : attr(a) {}
  std::string convert(DemangledType const & sym) const;

  template <typename OStream>
  OStream & convert(OStream & stream, DemangledType const & sym) const {
    convert_(stream, sym);
    return stream;
  }
  template <typename OStream>
  OStream & operator()(OStream & stream, DemangledType const & sym) const {
    return convert(stream, sym);
  }

 private:
  void convert_(std::ostream & stream, DemangledType const & sym) const;

  Attributes attr;
};

} // namespace demangle

#endif // Include_demangle_text

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
