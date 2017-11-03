// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#include "demangle_text.hpp"
#include <utility>              // std::move, std::forward
#include <sstream>              // std::ostringstream
#include <iterator>             // std::prev
#include <functional>           // std::function
#include <cassert>              // assert

namespace demangle {

namespace detail {

class Converter {
  struct ConvStream {
    std::ostream & stream;
    TextOutput::Attributes const & attr;

    ConvStream(std::ostream & s, TextOutput::Attributes const & a) : stream(s), attr(a) {}

    template <typename T>
    ConvStream & operator<<(T && x) {
      std::ostringstream os;
      os << std::forward<T>(x);
      return (*this) << os.str();
    }

    ConvStream & operator<<(std::string && x) {
      return (*this) << const_cast<std::string const &>(x);
    }

    ConvStream & operator<<(std::string const & s) {
      // if (last == ' ' && !s.empty() && s.front() == ' ') {
      //   stream << s.substr(1);
      // } else {
        stream << s;
      // }
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
      // if (c == ' ' && c == last) {
      //   return *this;
      // }
      if ((c == '<' || c == '>') && c == last
          && attr[TextOutput::SPACE_BETWEEN_TEMPLATE_BRACKETS])
      {
        (*this) << ' ';
      }
      stream << c;
      last = c;
      fixup();
      return *this;
    }

    void fixup() {
      if (last == ',' && attr[TextOutput::SPACE_AFTER_COMMA]) {
        (*this) << ' ';
        last = ' ';
      }
    }

    char last;
  };

  ConvStream stream;
  DemangledType const & t;

  enum spaceloc_t { BEFORE, AFTER };

 public:
  Converter(TextOutput::Attributes const & a, std::ostream & s, DemangledType const & dt)
    : stream(s, a), t(dt)
  {}
  void operator()();
 private:
  Converter sub(DemangledType const & dt) {
    return Converter(stream.attr, stream.stream, dt);
  }
  void do_name(DemangledType const & n);
  void do_name(FullyQualifiedName const & name);
  void do_name(FullyQualifiedName::const_reverse_iterator b,
               FullyQualifiedName::const_reverse_iterator e);
  void do_template_params(DemangledTemplate const & tmpl);
  void do_template_param(DemangledTemplateParameter const & param);
  void do_args(FunctionArgs const & args);
  void do_type(DemangledType const & type, std::function<void()> name = nullptr);
  void do_pointer(DemangledType const & ptr, std::function<void()> name = nullptr);
  void do_pointer_type(DemangledType const & ptr);
  void do_function(DemangledType const & fn, std::function<void()> name = nullptr);
  void do_cv(DemangledType const & type, spaceloc_t sl);
  void do_refspec(DemangledType const & fn);
  void do_method_properties(DemangledType const & m);

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
Stream operator<<(Stream stream, Scope scope) {
  switch (scope) {
   case Scope::Unspecified: break;
   case Scope::Private: return stream << "private: ";
   case Scope::Protected: return stream << "protected: ";
   case Scope::Public: return stream << "public: ";
  }
  return stream;
}

void Converter::do_method_properties(DemangledType const & m)
{
  if (stream.attr[TextOutput::OUTPUT_EXTERN] && m.extern_c) stream << "extern \"C\"";
  if (stream.attr[TextOutput::OUTPUT_THUNKS]
      && m.method_property == MethodProperty::Thunk)
  {
    stream << "[thunk]:";
  }
  stream << m.scope;
  if (m.method_property == MethodProperty::Static) stream << "static ";
  if (m.method_property == MethodProperty::Virtual) stream << "virtual ";
}

void Converter::operator()()
{
  switch (t.symbol_type) {
   case SymbolType::StaticClassMember:
   case SymbolType::ClassMethod:
   case SymbolType::GlobalFunction:
    do_type(t,  [this] { do_name(t); });
    break;
   case SymbolType::GlobalObject:
    do_type(t,  [this] { stream << ' '; do_name(t.instance_name); });
    break;
   case SymbolType::Unspecified:
   case SymbolType::GlobalThing1:
   case SymbolType::GlobalThing2:
   case SymbolType::String:
   case SymbolType::VtorDisp:
   case SymbolType::StaticGuard:
   case SymbolType::MethodThunk:
   case SymbolType::HexSymbol:
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
  FullyQualifiedName::const_reverse_iterator e)
{
  // Iterate over the name fragments
  for (auto i = b; i != e; ++i) {
    if (i != b) {
      stream << "::";
    }
    auto & frag = *i;
    if (frag->is_embedded) {
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
        stream << "<ERRNOCLASS>";
      } else {
        auto save = tset(template_parameters_,
                         stream.attr[TextOutput::CDTOR_CLASS_TEMPLATE_PARAMETERS]);
        do_name(**std::prev(i));
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
    if (stream.attr[TextOutput::MS_SIMPLE_TYPES]) {
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
        if (stream.attr[TextOutput::OUTPUT_ANONYMOUS_NUMBERS]) {
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
    stream << name.simple_code << ' ';
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
           << name.n1 << "," << name.n2 << ","
           << name.n3 << "," << name.n4 << ")'";
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
    if (p.type->is_func && p.type->is_member) {
      stream << '{';
      do_type(*p.type);
      if (p.constant_value >= 1) {
        stream << ',' << p.type->n1;
      }
      if (p.constant_value >= 2) {
        stream << ',' << p.type->n2;
      }
      stream << '}';
    } else {
      stream << '&';
      do_type(*p.type);
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
    auto b = std::begin(tmpl);
    auto e = std::end(tmpl);
    for (auto i = b; i != e; ++i) {
      if (i != b) {
        stream << ',';
      }
      auto & tp = *i;
      if (tp) {
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

void Converter::do_pointer_type(
  DemangledType const & ptr)
{
  if (ptr.is_pointer) stream << (ptr.is_gc ? '^' : '*');
  if (ptr.is_reference) stream << (ptr.is_gc ? '%' : '&');
  if (ptr.is_refref) stream << "&&";
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
    if (inner.is_member) {
        // Method pointer
        do_name(inner);
        stream << "::";
    }
    do_pointer_type(type);
    do_cv(type, AFTER);
    if (name) name();
    if (parens) stream << ')';
  };
  do_type(*type.inner_type, iname);
}

void Converter::do_type(
  DemangledType const & type,
  std::function<void()> name)
{
  do_method_properties(type);
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
  if (type.is_pointer || type.is_reference || type.is_refref) {
    do_pointer(type, pname);
    return;
  }
  if (type.is_func) {
    do_function(type, pname);
    return;
  }
  do_name(type);
  do_cv(type, BEFORE);
  if (pname) {
    pname();
  }
}

void Converter::do_function(
  DemangledType const & fn,
  std::function<void()> name)
{
  auto fname = [this, &fn, name]() {
    {
      stream << ' ';
      if (fn.symbol_type != SymbolType::Unspecified) {
        stream << fn.calling_convention << ' ';
      }
      if (name) name();
      do_args(fn.args);
      do_cv(fn, AFTER);
      do_refspec(fn);
    }
  };
  auto rv = fn.retval;
  if (!rv) {
    rv.reset(new DemangledType("void"));
  }
  auto save = tset(retval_, rv.get());
  do_type(*retval_, fname);
}

void Converter::do_cv(
  DemangledType const & type, spaceloc_t sl)
{
  char const * a = (sl == BEFORE) ? " " : "";
  char const * b = (sl == AFTER) ? " " : "";
  if (type.is_const) stream << a << "const" << b;
  if (type.is_volatile) stream << a << "volatile" << b;
  if (type.ptr64) stream << " __ptr64";
}

void Converter::do_refspec(
  DemangledType const & fn)
{
  if (fn.is_reference) stream << '&';
  if (fn.is_refref) stream << "&&";
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


} // namespace demangle

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
