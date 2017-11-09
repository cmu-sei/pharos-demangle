// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#ifndef Include_demangle_text
#define Include_demangle_text

#include "demangle.hpp"
#include <ostream>              // std::ostream

namespace demangle {

class TextOutput {
 public:
  enum Attribute {
    // class template parameters on ctors and dtors
    CDTOR_CLASS_TEMPLATE_PARAMETERS              = 0x01,
    // Microsoft legacy named names for [u]intX_t simple types
    MS_SIMPLE_TYPES                              = 0x02,
    // [thunk]: for thunks
    OUTPUT_THUNKS                                = 0x04,
    // extern "C"
    OUTPUT_EXTERN                                = 0x08,
    // spaces after commas
    SPACE_AFTER_COMMA                            = 0x10,
    // Include anonymous namespace numbers
    OUTPUT_ANONYMOUS_NUMBERS                     = 0x20,
    // spaces for templates between << and >>
    SPACE_BETWEEN_TEMPLATE_BRACKETS              = 0x40,
    // template parameters come before (instead of after) the type in user-defined conversion
    // operators
    USER_DEFINED_CONVERSION_TEMPLATE_BEFORE_TYPE = 0x80,
    // undname outputs an extra " }'"
    MS_BROKEN_METHODTHUNK                        = 0x100,
    // undname discards cv on pointer return values
    DISCARD_CV_ON_RETURN_POINTER                 = 0x200,
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
    auto attr =  Attributes();
    attr.set(OUTPUT_EXTERN);
    attr.set(OUTPUT_THUNKS);
    attr.set(CDTOR_CLASS_TEMPLATE_PARAMETERS);
    attr.set(MS_SIMPLE_TYPES);
    attr.set(SPACE_BETWEEN_TEMPLATE_BRACKETS);
    attr.set(USER_DEFINED_CONVERSION_TEMPLATE_BEFORE_TYPE);
    attr.set(MS_BROKEN_METHODTHUNK);
    attr.set(DISCARD_CV_ON_RETURN_POINTER);
    return attr;
  };

 public:
  TextOutput() = default;
  TextOutput(Attributes a) : attr(a) {}
  std::string convert(DemangledType const & sym) const;

  void set_attributes(Attributes a) {
    attr = a;
  }

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
