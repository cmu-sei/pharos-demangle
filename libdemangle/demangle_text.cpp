// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#include "demangle_text.hpp"
#include <utility>              // std::move, std::forward
#include <sstream>              // std::ostringstream
#include <iterator>             // std::prev
#include <functional>           // std::function

namespace demangle {

namespace {

class Converter {
 private:
  enum manip { BREAK };

  struct ConvStream {
    ConvStream(std::ostream & s) : stream(s) {}

    template <typename T>
    ConvStream & operator<<(T && val) {
      if (std_break) {
        stream << ' ';
        std_break = false;
      }
      stream << std::forward<T>(val);
      return *this;
    }

    ConvStream & operator<<(manip val) {
      switch (val) {
       case BREAK:
        std_break = true;
        break;
      }
      return *this;
    }

    std::ostream & stream;
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
  void do_function(DemangledType const & fn);
  void do_array(DemangledType const & arr, std::function<void()> name = nullptr);
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
                         attr[TextOutput::CDTOR_CLASS_TEMPLATE_PARAMETERS]);
        do_name(**std::prev(i));
      }
    } else {
      // Normal case
      do_name(*frag);
    }
  }
}


void Converter::do_name(
  DemangledType const & name)
{
  auto stype = [this, &name](char const * s) {
                 if (attr[TextOutput::MS_SIMPLE_TYPES]) {
                   stream << s;
                 } else {
                   stream << "std::" << code_string(name.simple_code);
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
      stream << "operator ";
      do_type(*retval_);
    } else {
      stream << name.simple_code;
    }
    break;

   case Code::RTTI_BASE_CLASS_DESC:
    stream << "`RTTI Base Class Descriptor at ("
           << name.n1 << ", " << name.n2 << ", " << name.n3 << ", " << name.n4 << ")'";
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
    if (p.type->symbol_type == SymbolType::ClassMethod
        || (p.type->is_func && p.type->is_member))
    {
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
    auto iname = [&name, &type]() {
      stream << '(';
      do_pointer_type(type);
      do_cv(type);
      if (name) name();
      stream << ')';
    };
    if (inner.is_func) {
      auto save = tset(retval_, type.inner_type->retval.get());
      do_type(*retval_, name);
      return;
    }
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
  if (type.is_pointer || type.is_reference || type.is_refref) {
    do_pointer(type, name);
    return;
  }
  if (type.is_array) {
    do_array(type, name);
    return;
  }
  do_name(type);
  if (name) {
    stream << BREAK;
    name();
  }
}

void Converter::do_function(
  DemangledType const & fn)
{
  auto name = [this, &fn]() {
    if (fn.symbol_type == SymbolType::GlobalFunction
        || fn.symbol_type == SymbolType::ClassMethod
        || fn.symbol_type == SymbolType::VtorDisp)
    {
      if (!fn.name.empty()) {
        do_name(fn.name);
        do_template_params(fn.name.back()->template_parameters);
      }
      do_args(fn.args);
      do_cv(fn);
      do_refspec(fn);
    }
  };
  auto save = tset(retval_, fn.retval.get());
  do_type(*retval_, name);
}

void Converter::do_array(
  DemangledType const & arr,
  std::function<void()> name)
{
  auto & inner = *arr.inner_type;
  if (inner.is_func) {
  } else {
    do_type(inner);
    if (name) {
      name();
    }
    for (auto dim : arr.dimensions) {
      stream << '[' << dim << ']';
    }
  }
}

void Converter::do_cv(
  DemangledType const & type)
{
  if (type.is_const) stream << "const" << BREAK;
  if (type.is_volatile) stream << "volatile" << BREAK;
}

void Converter::do_refspec(
  DemangledType const & fn)
{
  if (fn.is_reference) stream << '&' << BREAK;
  if (fn.is_refref) stream << "&&" << BREAK;
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
