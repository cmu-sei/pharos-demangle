// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.
// Pharos Demangler
//
// Copyright 2017 Carnegie Mellon University. All Rights Reserved.
//
// NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE ENGINEERING
// INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. CARNEGIE MELLON
// UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR
// IMPLIED, AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF
// FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS
// OBTAINED FROM USE OF THE MATERIAL. CARNEGIE MELLON UNIVERSITY DOES NOT
// MAKE ANY WARRANTY OF ANY KIND WITH RESPECT TO FREEDOM FROM PATENT,
// TRADEMARK, OR COPYRIGHT INFRINGEMENT.
//
// Released under a BSD-style license, please see license.txt or contact
// permission@sei.cmu.edu for full terms.
//
// [DISTRIBUTION STATEMENT A] This material has been approved for public
// release and unlimited distribution.  Please see Copyright notice for
// non-US Government use and distribution.
//
// DM17-0949

#ifndef Include_demangle_text
#define Include_demangle_text

#include "demangle.hpp"
#include <ostream>              // std::ostream

namespace demangle {

enum class TextAttribute : std::uint32_t {
  // class template parameters on ctors and dtors
  CDTOR_CLASS_TEMPLATE_PARAMETERS              = 0x01,
  // Microsoft legacy names for [u]intX_t simple types
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
  // undname adds extra apostrophe on static guards
  MS_BROKEN_STATIC_GUARD                       = 0x400,
  // verbose constant string symbols
  VERBOSE_CONSTANT_STRING                      = 0x800,
  // Output near distances
  OUTPUT_NEAR                                  = 0x1000,
};

class TextAttributes {
 public:
  TextAttributes() {}
  TextAttributes(TextAttribute a) : val(uint32_t(a)) {}
  TextAttributes(uint32_t a) : val(a) {}

  TextAttributes & set(TextAttribute a) {
    val |= static_cast<decltype(val)>(a);
    return *this;
  }
  TextAttributes & unset(TextAttribute a) {
    val &= ~static_cast<decltype(val)>(a);
    return *this;
  }

  bool operator[](TextAttribute a) const {
    return val & static_cast<decltype(val)>(a);
  }

  static TextAttributes undname() {
    auto attr = TextAttributes();
    attr.set(TextAttribute::OUTPUT_EXTERN);
    attr.set(TextAttribute::OUTPUT_THUNKS);
    attr.set(TextAttribute::CDTOR_CLASS_TEMPLATE_PARAMETERS);
    attr.set(TextAttribute::MS_SIMPLE_TYPES);
    attr.set(TextAttribute::SPACE_BETWEEN_TEMPLATE_BRACKETS);
    attr.set(TextAttribute::USER_DEFINED_CONVERSION_TEMPLATE_BEFORE_TYPE);
    attr.set(TextAttribute::MS_BROKEN_METHODTHUNK);
    attr.set(TextAttribute::DISCARD_CV_ON_RETURN_POINTER);
    attr.set(TextAttribute::MS_BROKEN_STATIC_GUARD);
    return attr;
  };

  static TextAttributes pretty() {
    auto attr =  TextAttributes();
    attr.set(TextAttribute::OUTPUT_THUNKS);
    attr.set(TextAttribute::SPACE_BETWEEN_TEMPLATE_BRACKETS);
    attr.set(TextAttribute::VERBOSE_CONSTANT_STRING);
    attr.set(TextAttribute::SPACE_AFTER_COMMA);
    attr.set(TextAttribute::OUTPUT_ANONYMOUS_NUMBERS);
    return attr;
  };

  static std::vector<std::pair<const TextAttribute, const std::string>> const &
  explain();

 private:
  std::uint32_t val = 0;
};

class TextOutput {
 public:
  TextOutput() = default;
  TextOutput(TextAttributes a) : attr(a) {}

  std::string convert(DemangledType const & sym) const;

  void set_attributes(TextAttributes a) {
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

  class StreamApplyObject {
    TextOutput const & obj;
    DemangledType const & sym;
   public:
    StreamApplyObject(TextOutput const & t, DemangledType const & s) : obj(t), sym(s) {}

    template <typename OStream>
    OStream & operator()(OStream & stream) const {
      return obj(stream, sym);
    }
  };

  StreamApplyObject operator()(DemangledType const & sym) const {
    return StreamApplyObject(*this, sym);
  }

  std::string get_class_name(DemangledType const & sym) const;
  std::string get_method_name(DemangledType const & sym) const;
  std::string get_method_signature(DemangledType const & sym) const;

 private:
  void convert_(std::ostream & stream, DemangledType const & sym) const;

  TextAttributes attr;
};

template <typename OStream>
OStream & operator<<(OStream & stream, TextOutput::StreamApplyObject const & sao) {
  return sao(stream);
}

} // namespace demangle

#endif // Include_demangle_text

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
