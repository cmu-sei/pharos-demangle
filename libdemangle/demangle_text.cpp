// Pharos Demangler
//
// Copyright 2017-2020 Carnegie Mellon University. All Rights Reserved.
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

#include "demangle_text.hpp"
#include <utility>              // std::move, std::forward
#include <sstream>              // std::ostringstream
#include <iterator>             // std::prev
#include <functional>           // std::function
#include <cassert>              // assert

namespace demangle {

namespace detail {

constexpr bool SPACE_MUNGING = true;

class Converter {

  template <typename T>
  struct Raw {
    T val;
  };

  template <typename T>
  static Raw<T> raw(T val) {
    return Raw<T>{val};
  }

  struct ConvStream {
    std::ostream & stream;
    TextAttributes const & attr;

    ConvStream(std::ostream & s, TextAttributes const & a) : stream(s), attr(a) {}

    template <typename T>
    ConvStream & operator<<(Raw<T> && x) {
      stream << x.val;
      last = '\0';
      return *this;
    }

    template <typename T>
    ConvStream & operator<<(T && x) {
      std::ostringstream os;
      os << std::forward<T>(x);
      (*this) << os.str();
      return *this;
    }

    ConvStream & operator<<(std::string && x) {
      (*this) << const_cast<std::string const &>(x);
      return *this;
    }

    static bool is_symbol_char(char c) {
      return c == '_' || std::isalnum(c);
    }

    ConvStream & operator<<(std::string const & s) {
      if (SPACE_MUNGING && !s.empty() && is_symbol_char(last) && is_symbol_char(s.front())) {
        // Ensure a space between symbols
        stream << ' ' << s;
      } else if (SPACE_MUNGING && last == ' ' && !s.empty() && s.front() == ' ') {
        // Don't allow double-spaces
        stream << s.substr(1);
      } else {
        stream << s;
      }
      if (!s.empty()) {
        last = s.back();
      }
      fixup();
      return *this;
    }

    ConvStream & operator<<(char const * s) {
      return (*this) << std::string(s);
    }

    ConvStream & operator<<(char c) {
      if (SPACE_MUNGING && c == ' ' && c == last) {
        // Don't allow double-spaces
        return *this;
      }
      if ((c == '<' || c == '>') && c == last
          && attr[TextAttribute::SPACE_BETWEEN_TEMPLATE_BRACKETS])
      {
        (*this) << ' ';
      }
      stream << c;
      last = c;
      fixup();
      return *this;
    }

    void fixup() {
      if (last == ',' && attr[TextAttribute::SPACE_AFTER_COMMA]) {
        (*this) << ' ';
        last = ' ';
      }
    }

    char last = ' ';
  };

  ConvStream stream;
  DemangledType const & t;
  bool do_cconv = true;
  bool in_op_type = false;

  enum cv_context_t { BEFORE, AFTER };

 public:
  Converter(TextAttributes const & a, std::ostream & s, DemangledType const & dt)
    : stream(s, a), t(dt)
  {}
  void operator()();

  void class_name();
  void method_name();
  void method_signature();

 private:
  Converter sub(DemangledType const & dt) {
    return Converter(stream.attr, stream.stream, dt);
  }
  void do_name(DemangledType const & n);
  void do_name(FullyQualifiedName const & name);
  void do_name(FullyQualifiedName::const_reverse_iterator b,
               FullyQualifiedName::const_reverse_iterator e,
               bool only_last = false);
  void do_template_params(DemangledTemplate const & tmpl);
  void do_template_param(DemangledTemplateParameter const & param);
  void do_args(FunctionArgs const & args);
  void do_type(DemangledType const & type, std::function<void()> name = nullptr);
  void do_pointer(DemangledType const & ptr, std::function<void()> name = nullptr);
  void do_function(DemangledType const & fn, std::function<void()> name = nullptr);
  void do_storage_properties(DemangledType const & type, cv_context_t ctx);
  void do_method_properties(DemangledType const & m);
  void output_quoted_string(std::string const & s);

  bool template_parameters_ = true;
  DemangledType const * retval_ = nullptr;

  template <typename R, typename T>
  struct tset_impl {
    tset_impl(R & lc, T && val) : loc(lc), v(lc) {
      loc = std::forward<T>(val);
    }
    ~tset_impl() {
      loc = std::move(v);
    }
    private:
      R & loc;
      R v;
  };
  template <typename R, typename T>
  static tset_impl<R, T> tset(R & lc, T && val) {
    return tset_impl<R, T>(lc, std::forward<T>(val));
  }
};

template <typename Stream>
Stream & operator<<(Stream & stream, Scope scope) {
  switch (scope) {
   case Scope::Unspecified: break;
   case Scope::Private: stream << "private: "; break;
   case Scope::Protected: stream << "protected: "; break;
   case Scope::Public: stream << "public: "; break;
  }
  return stream;
}

template <typename Stream>
Stream & operator<<(Stream & stream, Distance distance) {
  switch (distance) {
   case Distance::Unspecified: break;
   case Distance::Near: stream << "near "; break;
   case Distance::Far: stream << "far "; break;
   case Distance::Huge: stream << "huge "; break;
  }
  return stream;
}

void Converter::output_quoted_string(std::string const & s)
{
  static std::string special_chars("\"\\\a\b\f\n\r\t\v\0", 10);
  static std::string names("\"\\abfnrtv0", 10);
  stream << '\"';
  for (auto c : s) {
    auto pos = special_chars.find_first_of(c);
    if (pos == std::string::npos) {
      stream << raw(c);
    } else {
      stream << raw('\\') << raw(names[pos]);
    }
  }
  stream << '\"';
}

void Converter::do_method_properties(DemangledType const & m)
{
  if (stream.attr[TextAttribute::OUTPUT_EXTERN] && m.extern_c) stream << "extern \"C\"";
  if (stream.attr[TextAttribute::OUTPUT_THUNKS]
      && m.method_property == MethodProperty::Thunk)
  {
    stream << "[thunk]: ";
  }
  stream.operator<<(m.scope); // written this way due to operator lookup ambiguity
  if (m.method_property == MethodProperty::Static) stream << "static ";
  if (m.method_property == MethodProperty::Virtual
      // Thunks are virtual
      || (m.method_property == MethodProperty::Thunk &&
          // Except for vcall?
          (!m.name.empty() && m.name.front()->simple_code != Code::VCALL)))
  {
    stream << "virtual ";
  }
}

void Converter::operator()()
{
  switch (t.symbol_type) {
   case SymbolType::ClassMethod:
   case SymbolType::GlobalFunction:
   case SymbolType::VtorDisp:
    // function-like
    do_type(t,  [this] { do_name(t); });
    break;
   case SymbolType::RTTI:
    // RTTI
    if (t.retval) {
      do_type(*t.retval);
      stream << ' ';
    }
    do_name(t);
    break;
   case SymbolType::StaticClassMember:
   case SymbolType::GlobalObject:
    // static objects
    do_type(t,  [this] { stream << ' '; do_name(t.instance_name); });
    break;
   case SymbolType::MethodThunk:
    do_method_properties(t);
    stream << t.calling_convention << ' ';
    do_name(t);
    stream << '{' << t.n[0] << ",{flat}}'";
    if (stream.attr[TextAttribute::BROKEN_UNDNAME]) {
      // undname.exe ouputs an extra brace and quote
      stream << " }'";
    }
    break;
   case SymbolType::VTable:
    // vtables
    do_storage_properties(t, AFTER);
    do_name(t.instance_name);
    if (!t.com_interface.empty()) {
      stream << "{for ";
      auto i = t.com_interface.begin();
      auto e = t.com_interface.end();
      while (i != e) {
        stream << '`';
        do_name((*i++)->name);
        stream << '\'';
        if (i != e) {
          stream << "s ";
        }
      }
      stream << '}';
    }
    break;
   case SymbolType::StaticGuard:
    // Static variable guards
    do_name(t.name);
    stream << '{' << t.n[0] << '}';
    if (stream.attr[TextAttribute::BROKEN_UNDNAME]) {
      // undname.exe ouputs an extra quote
      stream << '\'';
    }
    break;
   case SymbolType::String:
    // Constant string
    if (!stream.attr[TextAttribute::VERBOSE_CONSTANT_STRING]) {
      stream << "`string'";
    } else {
      do_type(*t.inner_type);
      stream << '[' << t.n[0] << "] = ";
      output_quoted_string(t.name[0]->simple_string);
      if (t.n[0] > 32) {
        stream << "...";
      }
    }
    break;
   case SymbolType::HexSymbol:
    // Simple hex numbers
    stream << t.simple_string;
    break;
   case SymbolType::Unspecified:
    // Guess based on contents
    if (t.instance_name.empty()) {
      do_type(t);
    } else {
      do_type(t,  [this] { stream << ' '; do_name(t.instance_name); });
    }
    break;
  }
}

void Converter::do_name(
  FullyQualifiedName const & name)
{
  do_name(name.rbegin(), name.rend());
}

void Converter::do_name(
  FullyQualifiedName::const_reverse_iterator b,
  FullyQualifiedName::const_reverse_iterator e,
  bool only_last)
{
  // Iterate over the name fragments
  for (auto i = only_last ? std::prev(e) : b; i != e; ++i) {
    if (!only_last && i != b) {
      stream << "::";
    }
    auto & frag = *i;
    auto next = std::next(i);
    if (next != e && (*next)->simple_code == Code::DYNAMIC_ATEXIT_DTOR) {
      // Special case for DYNAMIC_ATEXIT_DTOR
      stream << "`dynamic atexit destructor for '";
      do_name(i, next);
      stream << "''";
      i = next;
    } else if (frag->is_embedded) {
      // Embedded symbols get `' around them
      stream << '`';
      sub(*frag)();
      stream << '\'';
    } else if (frag->is_ctor || frag->is_dtor) {
      // ctors and dtors need to get their name from the class name,
      // which should be the previous name
      if (frag->is_dtor) {
        stream << '~';
      }
      if (i == b) {
        stream << "{ERRNOCLASS}";
      } else {
        auto save = tset(template_parameters_,
                         stream.attr[TextAttribute::CDTOR_CLASS_TEMPLATE_PARAMETERS]);
        do_name(std::prev(i), i);
      }
    } else if (frag->simple_code == Code::OP_TYPE) {
      // Where do we place template parameters in an operator type construct?  Microsoft does
      // it one way, the rest of the world does it another.
      stream << "operator";
      if (stream.attr[TextAttribute::USER_DEFINED_CONVERSION_TEMPLATE_BEFORE_TYPE]) {
        do_template_params(frag->template_parameters);
      }
      stream << ' ';
      if (retval_) {
        auto save = tset(in_op_type, true);
        do_type(*retval_);
      } else {
        stream << "{UNKNOWN_TYPE}";
      }
      if (stream.attr[TextAttribute::USER_DEFINED_CONVERSION_TEMPLATE_BEFORE_TYPE]) {
        continue;
      }
    } else {
      // Normal case
      do_name(*frag);
    }
    do_template_params(frag->template_parameters);
  }
}


void Converter::do_name(
  DemangledType const & name)
{
  auto stype = [this, &name](char const * s) {
    if (stream.attr[TextAttribute::MS_SIMPLE_TYPES]) {
      stream << s;
    } else {
      stream << "std::" << code_string(name.simple_code);
    }
  };

  switch (name.simple_code) {
   case Code::UNDEFINED:
    if (name.name.empty()) {
      if (name.is_anonymous) {
        stream << "`anonymous namespace";
        if (stream.attr[TextAttribute::OUTPUT_ANONYMOUS_NUMBERS]) {
          stream << ' ' << name.simple_string;
        }
        stream << '\'';
      } else {
        stream << name.simple_string;
      }
    } else {
      do_name(name.name);
    }
    break;

   case Code::CLASS: case Code::STRUCT: case Code::UNION: case Code::ENUM:
    if (!stream.attr[TextAttribute::DISABLE_PREFIXES]) {
      stream << name.simple_code << ' ';
    }
    do_name(name.name);
    break;

   case Code::INT8:    stype("__int8"); break;
   case Code::INT16:   stype("__int16"); break;
   case Code::INT32:   stype("__int32"); break;
   case Code::INT64:   stype("__int64"); break;
   case Code::UINT8:   stype("unsigned __int8"); break;
   case Code::UINT16:  stype("unsigned __int16"); break;
   case Code::UINT32:  stype("unsigned __int32"); break;
   case Code::UINT64:  stype("unsigned __int64"); break;

   case Code::OP_TYPE:
    if (retval_) {
      stream << "operator ";
      do_type(*retval_);
    } else {
      stream << name.simple_code;
    }
    break;

   case Code::RTTI_BASE_CLASS_DESC:
    stream << "`RTTI Base Class Descriptor at ("
           << name.n[0] << "," << name.n[1] << ","
           << name.n[2] << "," << name.n[3] << ")'";
    break;

   default:
    stream << name.simple_code;
  }
}

void Converter::do_template_param(
  DemangledTemplateParameter const & p)
{
  if (!p.type) {
    stream << p.constant_value;
  } else if (p.pointer) {
    if (p.type->is_func && p.type->is_member && !p.type->n.empty()) {
      stream << '{';
      sub(*p.type)();
      for (auto v : p.type->n) {
        stream << ',' << v;
      }
      stream << '}';
    } else {
      stream << '&';
      sub(*p.type)();
    }
  } else {
    do_type(*p.type);
  }
}

void Converter::do_template_params(
  DemangledTemplate const & tmpl)
{
  if (template_parameters_ && !tmpl.empty()) {
    stream << '<';
    bool first = true;
    for (auto & tp : tmpl) {
      if (tp) {
        if (first) {
          first = false;
        } else {
          stream << ',';
        }
        do_template_param(*tp);
      }
    }
    stream << '>';
  }
}

void Converter::do_args(
  FunctionArgs const & args)
{
  stream << '(';
  auto b = std::begin(args);
  auto e = std::end(args);
  for (auto i = b; i != e; ++i) {
    if (i != b) {
      stream << ',';
    }
    do_type(**i);
  }
  stream << ')';
}

void Converter::do_pointer(
  DemangledType const & type,
  std::function<void()> name)
{
  auto iname = [this, &name, &type]() {
    auto & inner = *type.inner_type;
    bool parens = inner.is_func || inner.is_array;
    stream << (parens ? '(' : ' ');
    if (inner.is_func) {
      stream << inner.calling_convention << ' ';
    }
    if (inner.is_member && !type.name.empty()) {
      // Method or member pointer
      do_name(type);
      stream << "::";
    }
    do_storage_properties(type, BEFORE);
    if (type.ptr64 > 1) {
      // Gross hack to deal with the fact the the symbol can be __ptr64 as well as the type
      stream << " __ptr64";
    }
    if (name) name();
    if (parens) stream << ')';
  };
  if (type.inner_type->is_func) {
    auto save = tset(do_cconv, false);
    do_type(*type.inner_type, iname);
  } else {
    do_type(*type.inner_type, iname);
  }
}

void Converter::do_type(
  DemangledType const & type,
  std::function<void()> name)
{
  do_method_properties(type);
  if (type.distance != Distance::Near || stream.attr[TextAttribute::OUTPUT_NEAR]) {
    stream.operator<<(type.distance); // written this way due to operator lookup ambiguity
  }
  auto pname = name;
  if (type.is_array) {
    auto aname = [this, &type, name]() {
      if (name) name();
      for (auto dim : type.dimensions) {
        stream << '[' << dim << ']';
      }
    };
    pname = aname;
  }
  if (type.is_func) {
    do_function(type.inner_type ? *type.inner_type : type, pname);
    return;
  }
  if (type.is_pointer || type.is_reference || type.is_refref) {
    do_pointer(type, pname);
    return;
  }
  do_name(type);
  do_storage_properties(type, BEFORE);
  if (pname) {
    pname();
  }
}

void Converter::do_function(
  DemangledType const & fn,
  std::function<void()> name)
{
  auto cconv = do_cconv;
  auto fname = [this, &fn, name, cconv]() {
    {
      stream << ' ';
      if (fn.symbol_type != SymbolType::Unspecified || cconv) {
        stream << fn.calling_convention << ' ';
      }
      if (name) name();
      if (fn.symbol_type == SymbolType::VtorDisp) {
        stream << "`vtordisp{" << fn.n[0] << ',' << fn.n[1] << "}' ";
      } else if (fn.method_property == MethodProperty::Thunk && fn.n.size() >= 2) {
        stream << "`adjustor{" << fn.n[1] << "}' ";
      }
      do_args(fn.args);
      do_storage_properties(fn, AFTER);
    }
  };
  auto rv = fn.retval;
  if (!rv) {
    rv.reset(new DemangledType("void"));
  }
  auto save = tset(retval_, rv.get());
  auto save2 = tset(do_cconv, true);
  if (!fn.name.empty() && fn.name.front()->simple_code == Code::OP_TYPE) {
    // operator <type>
    fname();
  } else {
    do_type(*retval_, fname);
  }
}

void Converter::do_storage_properties(
  DemangledType const & type, cv_context_t ctx)
{
  bool is_retval = retval_ == &type;
  bool discard = stream.attr[TextAttribute::DISCARD_CV_ON_RETURN_POINTER]
                 && type.is_pointer && is_retval && !in_op_type;
  char const * a = (ctx == BEFORE) ? " " : "";
  char const * b = (ctx == AFTER) ? " " : "";

  auto cv = [this, &type, a, b]() {
    if (type.is_const) stream << a << "const" << b;
    if (type.is_volatile) stream << a << "volatile" << b;
  };

  if (!discard && ctx == AFTER) cv();
  if (type.unaligned && stream.attr[TextAttribute::MS_QUALIFIERS]) {
    stream << a << "__unaligned" << b;
  }
  if (type.is_pointer) stream << a << (type.is_gc ? '^' : '*') << b;
  if (type.is_reference) stream << a << (type.is_gc ? '%' : '&') << b;
  if (type.is_refref) stream << a << "&&" << b;
  if (type.ptr64 && stream.attr[TextAttribute::OUTPUT_PTR64]) stream << a << "__ptr64" << b;
  if (type.restrict && stream.attr[TextAttribute::MS_QUALIFIERS]) {
    stream << a << "__restrict" << b;
  }
  if (!discard && ctx == BEFORE) cv();
}

void Converter::class_name()
{
  if (!t.name.empty()) {
    do_name(t.name.rbegin(), std::prev(t.name.rend()));
  }
}

void Converter::method_name()
{
  if (!t.name.empty()) {
    auto rv = t.retval;
    if (!rv) {
      rv.reset(new DemangledType("void"));
    }
    auto save = tset(retval_, rv.get());
    do_name(t.name.rbegin(), t.name.rend(), true);
  }
}

void Converter::method_signature()
{
  auto rv = t.retval;
  if (!rv) {
    rv.reset(new DemangledType("void"));
  }
  auto save = tset(retval_, rv.get());
  do_type(t, [this] { method_name(); });
}

} // namespace detail

std::string TextOutput::convert(DemangledType const & sym) const
{
  std::ostringstream os;
  convert(os, sym);
  return os.str();
}

void TextOutput::convert_(std::ostream & stream, DemangledType const & sym) const
{
  detail::Converter(attr, stream, sym)();
}

std::string TextOutput::get_class_name(DemangledType const & sym) const
{
  std::ostringstream os;
  detail::Converter(attr, os, sym).class_name();
  return os.str();
}

std::string TextOutput::get_method_name(DemangledType const & sym) const
{
  std::ostringstream os;
  detail::Converter(attr, os, sym).method_name();
  return os.str();
}

std::string TextOutput::get_method_signature(DemangledType const & sym) const
{
  std::ostringstream os;
  detail::Converter(attr, os, sym).method_signature();
  return os.str();
}

std::vector<std::pair<const TextAttribute, const std::string>> const &
TextAttributes::explain()
{
  static std::vector<std::pair<const TextAttribute, const std::string>> names{
    {TextAttribute::SPACE_AFTER_COMMA,
     "Add a space after a comma"},
    {TextAttribute::SPACE_BETWEEN_TEMPLATE_BRACKETS,
     "Output spaces between adjacent identical template brackets"},
    {TextAttribute::VERBOSE_CONSTANT_STRING,
     "Include partial string content for constant string symbols"},
    {TextAttribute::CDTOR_CLASS_TEMPLATE_PARAMETERS,
     "Output a class's template parameters on the ctor or dtor name as well"},
    {TextAttribute::USER_DEFINED_CONVERSION_TEMPLATE_BEFORE_TYPE,
     "On templated user-defined conversion operators, put the template before the type"},
    {TextAttribute::OUTPUT_NEAR,
     "Include the near keyword on symbols marked as near"},
    {TextAttribute::MS_SIMPLE_TYPES,
     "Use Microsoft legacy names for [u]intX_t, like __int64"},
    {TextAttribute::OUTPUT_THUNKS,
     "Output [thunk]: in front of thunks"},
    {TextAttribute::OUTPUT_EXTERN,
     "Include extern \"C\" on names mangled(!) as extern \"C\""},
    {TextAttribute::OUTPUT_ANONYMOUS_NUMBERS,
     "Include namespace numbers in anonymous namespace outputs"},
    {TextAttribute::DISCARD_CV_ON_RETURN_POINTER,
     "Discard const on pointer return values"},
    {TextAttribute::MS_QUALIFIERS,
     "Output Microsoft type qualifiers (__restrict, __unaligned)"},
    {TextAttribute::OUTPUT_PTR64,
     "Output __ptr64"},
    {TextAttribute::DISABLE_PREFIXES,
     "Disable enum/class/struct/union prefixes"},
    {TextAttribute::BROKEN_UNDNAME,
     "Include incorrect output that matches undname.exe when possible"},
  };

  return names;
}

} // namespace demangle

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
