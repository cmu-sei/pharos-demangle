// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#include "demangle_text.hpp"
#include <utility>              // std::move, std::forward
#include <sstream>              // std::ostringstream
#include <iterator>             // std::prev
#include <functional>           // std::function
#include <cassert>              // assert

namespace demangle {

namespace {

class Converter {
 private:
  enum manip { BREAK, WBREAK, PBREAK, NBREAK };

  template <typename T>
  struct Raw {
    Raw(T x) : val(x) {}
    T val;
  };

  template <typename T>
  static Raw<T> raw(T x) { return Raw<T>{x}; }

  struct ConvStream {
    ConvStream(std::ostream & s) : stream(s) {}

    template <typename T>
    ConvStream & operator<<(T && val) {
      return redir(std::forward<T>(val));
    }

    template <typename T>
    ConvStream & operator<<(Raw<T> val) {
      return redir(std::forward<T>(val.val));
    }

    template <typename T>
    ConvStream & redir(T && val) {
      if (std_break || wbreak > 1 || pbreak > 1) {
        stream << ' ';
      }
      stream << std::forward<T>(val);
      std_break = false;
      wbreak = 0;
      pbreak = 0;
      return *this;
    }

    ConvStream & operator<<(manip val) {
      switch (val) {
       case BREAK: std_break = true; break;
       case WBREAK: ++wbreak; break;
       case PBREAK: ++pbreak; break;
       case NBREAK: break;
      }
      return *this;
    }

    static manip manip_of(char c) {
      switch (c) {
       case '(': case ')': case '<': case '>': return NBREAK;
       case '_': return WBREAK;
       default:
        if (c == '_' || std::isalnum(c)) {
          return WBREAK;
        }
        if (std::iswspace(c)) {
          return NBREAK;
        }
        return PBREAK;
      }
    }

    ConvStream & operator<<(char c) {
      return (*this) << manip_of(c) << raw(c) << manip_of(c);
    }

    ConvStream & operator<<(char const * s) {
      if (!*s) {
        return *this;
      }
      (*this) << manip_of(*s);
      while (*(s + 1)) {
        (*this) << raw(*s++);
      }
      return (*this) << raw(*s) << manip_of(*s);
    }

    ConvStream & operator<<(std::string const & s) {
      if (s.empty()) {
        return *this;
      }
      return (*this) << manip_of(s.front()) << raw(s) << manip_of(s.back());
    }

    std::ostream & stream;
    size_t wbreak = 0;
    size_t pbreak = 0;
    bool std_break = false;
  };

  TextOutput::Attributes const & attr;
  ConvStream stream;
  DemangledType const & t;

 public:
  Converter(TextOutput::Attributes const & a, std::ostream & s, DemangledType const & dt)
    : attr(a), stream(s), t(dt)
  {}
  void operator()();
 private:
  Converter sub(DemangledType const & dt) {
    return Converter(attr, stream.stream, dt);
  }
  void do_name(DemangledType const & n);
  void do_name(FullyQualifiedName const & name);
  void do_name(FullyQualifiedName::const_iterator b,
               FullyQualifiedName::const_iterator e);
  void do_template_params(DemangledTemplate const & tmpl);
  void do_template_param(DemangledTemplateParameter const & param);
  void do_args(FunctionArgs const & args);
  void do_type(DemangledType const & type, std::function<void()> name = nullptr);
  void do_pointer(DemangledType const & ptr, std::function<void()> name = nullptr);
  void do_pointer_type(DemangledType const & ptr);
  void do_function(DemangledType const & fn, std::function<void()> name = nullptr);
  void do_cv(DemangledType const & type);
  void do_refspec(DemangledType const & fn);

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


void Converter::operator()()
{
  switch (t.symbol_type) {
   case SymbolType::StaticClassMember:
   case SymbolType::ClassMethod:
   case SymbolType::GlobalFunction:
   case SymbolType::GlobalObject:
    do_type(t,  [this] { do_name(t); });
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
  do_name(std::begin(name), std::end(name));
}

void Converter::do_name(
  FullyQualifiedName::const_iterator b,
  FullyQualifiedName::const_iterator e)
{
  // Iterate over the name fragments
  for (auto i = b; i != e; ++i) {
    if (i != b) {
      stream << raw("::");
    }
    auto & frag = *i;
    if (frag->is_embedded) {
      // Embedded symbols get `' around them
      stream << raw('`');
      sub(*frag)();
      stream << raw('\'');
    } else if (frag->is_ctor || frag->is_dtor) {
      // ctors and dtors need to get their name from the class name,
      // which should be the previous name
      if (frag->is_dtor) {
        stream << raw('~');
      }
      if (i == b) {
        stream << WBREAK << raw("<ERRNOCLASS>") << WBREAK;
      } else {
        auto save = tset(template_parameters_,
                         attr[TextOutput::CDTOR_CLASS_TEMPLATE_PARAMETERS]);
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
    if (attr[TextOutput::MS_SIMPLE_TYPES]) {
      stream << s;
    } else {
      stream << raw("std::") << code_string(name.simple_code);
    }
  };

  switch (name.simple_code) {
   case Code::UNDEFINED:
    if (name.name.empty()) {
      stream << name.simple_string;
    } else {
      do_name(name.name);
    }
    break;

   case Code::CLASS: case Code::STRUCT: case Code::UNION: case Code::ENUM:
    stream << name.simple_code << BREAK;
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
      stream << WBREAK << "operator" << BREAK;
      do_type(*retval_);
    } else {
      stream << name.simple_code;
    }
    break;

   case Code::RTTI_BASE_CLASS_DESC:
    stream << raw("`RTTI Base Class Descriptor at (")
           << name.n1 << raw(",") << name.n2 << raw(",")
           << name.n3 << raw(",") << name.n4 << raw(")'");
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
  auto & inner = *type.inner_type;
  if (inner.is_func || inner.is_array) {
    auto iname = [this, &name, &type]() {
      stream << '(';
      if (type.inner_type->is_member) {
        // Method pointer
        do_name(*type.inner_type);
        stream << raw("::");
      }
      do_pointer_type(type);
      do_cv(type);
      if (name) name();
      stream << ')';
    };
    do_type(*type.inner_type, iname);
  } else {
    do_type(inner);
    do_pointer_type(type);
    do_cv(type);
    if (name) {
      name();
    }
  }
}

void Converter::do_type(
  DemangledType const & type,
  std::function<void()> name)
{
  if (type.is_array) {
    auto aname = [this, &type, name]() {
      if (name) {
        name();
      }
      if (name) {
        name();
      }
      for (auto dim : type.dimensions) {
        stream << '[' << dim << ']';
      }
    };
    name = aname;
  }
  if (type.is_pointer || type.is_reference || type.is_refref) {
    do_pointer(type, name);
    return;
  }
  if (type.is_func) {
    do_function(type, name);
    return;
  }
  do_name(type);
  if (name) {
    name();
  }
}

void Converter::do_function(
  DemangledType const & fn,
  std::function<void()> name)
{
  auto fname = [this, &fn, name]() {
    {
      if (name) name();
      do_args(fn.args);
      do_cv(fn);
      do_refspec(fn);
    }
  };
  auto save = tset(retval_, fn.retval.get());
  do_type(*retval_, name);
}

void Converter::do_cv(
  DemangledType const & type)
{
  if (type.is_const) stream << "const";
  if (type.is_volatile) stream << "volatile";
}

void Converter::do_refspec(
  DemangledType const & fn)
{
  if (fn.is_reference) stream << '&';
  if (fn.is_refref) stream << "&&";
}

} // unnamed namespace

std::string TextOutput::convert(DemangledType const & sym) const
{
  std::ostringstream os;
  convert(os, sym);
  return os.str();
}

void TextOutput::convert_(std::ostream & stream, DemangledType const & sym) const
{
  Converter(attr, stream, sym)();
}


} // namespace demangle

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
