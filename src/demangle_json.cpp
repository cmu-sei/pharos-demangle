// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#include "demangle_json.hpp"
#include <utility>              // std::move

namespace demangle {

void JsonOutput::handle_symbol_type(Object & obj, DemangledType const & sym) const
{
  // Symbol type
  char const * symbol_type = nullptr;
  switch (sym.symbol_type) {
   case SymbolType::Unspecified:
    return;
    // Fall through
   case SymbolType::Namespace:
    symbol_type = "namespace";
    break;
   case SymbolType::StaticClassMember:
    symbol_type = "static class member";
    break;
   case SymbolType::GlobalObject:
    symbol_type = "global object";
    break;
   case SymbolType::GlobalFunction:
    symbol_type = "global function";
    break;
   case SymbolType::ClassMethod:
    symbol_type = "class method";
    break;
   case SymbolType::GlobalThing1:
    symbol_type = "global thing 1";
    break;
   case SymbolType::GlobalThing2:
    symbol_type = "global thing 2";
    break;
   case SymbolType::String:
    symbol_type = "string";
    break;
   case SymbolType::VtorDisp:
    symbol_type = "vtordisp";
    break;
   case SymbolType::StaticGuard:
    symbol_type = "static guard";
    break;
   case SymbolType::MethodThunk:
    symbol_type = "method thunk";
    break;
  }
  obj.add("symbol_type", symbol_type);
}

void JsonOutput::handle_scope(Object & obj, DemangledType const & sym) const
{
  char const * scope = nullptr;
  switch (sym.scope) {
   case Scope::Unspecified:
    return;
   case Scope::Private:
    scope = "private";
    break;
   case Scope::Protected:
    scope = "protected";
    break;
   case Scope::Public:
    scope = "public";
    break;
  }
  obj.add("scope", scope);
}

void JsonOutput::handle_distance(Object & obj, DemangledType const & sym) const
{
  char const * distance = nullptr;
  switch (sym.distance) {
   case Distance::Unspecified:
    return;
   case Distance::Near:
    distance = "near";
    break;
   case Distance::Far:
    distance = "far";
    break;
   case Distance::Huge:
    distance = "huge";
    break;
  }
  obj.add("distance", distance);
}

void JsonOutput::handle_method_property(Object & obj, DemangledType const & sym) const
{
  char const * prop = nullptr;
  switch (sym.method_property) {
   case MethodProperty::Unspecified:
    return;
   case MethodProperty::Ordinary:
    prop = "ordinary";
    break;
   case MethodProperty::Static:
    prop = "static";
    break;
   case MethodProperty::Virtual:
    prop = "virtual";
    break;
   case MethodProperty::Thunk:
    prop = "thunk";
    break;
  }
  obj.add("method_property", prop);
}

void JsonOutput::handle_namespace(Object & obj, DemangledType const & sym) const
{
  if (sym.name.empty()) {
    return;
  }
  auto ns = builder.array();
  for (auto & part : sym.name) {
    ns->add(convert(*part));
  }
  obj.add("namespace", std::move(ns));
}

JsonOutput::ObjectRef JsonOutput::convert(DemangledType const & sym) const
{
  // This is not yet finished

  auto node = builder.object();
  auto & obj = *node;

  handle_symbol_type(obj, sym);
  handle_scope(obj, sym);

  if (sym.symbol_type == SymbolType::GlobalFunction
      || sym.symbol_type == SymbolType::ClassMethod)
  {
    handle_distance(obj, sym);
    if (sym.retval) {
      obj.add("return_type", convert(*sym.retval));
    }
    obj.add("calling_convention", sym.calling_convention);
  }
  handle_namespace(obj, sym);

  obj.add("text", sym.str(match));

  return std::move(node);
}

JsonOutput::ObjectRef JsonOutput::raw(DemangledType const & sym) const
{
  auto node = builder.object();
  auto & obj = *node;

  auto add_bool = [&obj, this](char const * name, bool val) {
                    if (val) {
                      obj.add(name, val);
                    }
                  };

  add_bool("is_const", sym.is_const);
  add_bool("is_volatile", sym.is_volatile);
  add_bool("is_reference", sym.is_reference);
  add_bool("is_pointer", sym.is_pointer);
  add_bool("is_array", sym.is_array);

  if (!sym.dimensions.empty()) {
    auto dim = builder.array();
    for (auto d : sym.dimensions) {
      dim->add(std::intmax_t(d));
    }
    obj.add("dimensions", std::move(dim));
  }

  add_bool("is_embedded", sym.is_embedded);
  add_bool("is_func", sym.is_func);
  add_bool("is_based", sym.is_based);
  add_bool("is_member", sym.is_member);
  add_bool("is_namespace", sym.is_namespace);
  add_bool("is_anonymous", sym.is_anonymous);
  add_bool("is_refref", sym.is_refref);

  handle_symbol_type(obj, sym);
  handle_distance(obj, sym);
  add_bool("ptr64", sym.ptr64);
  add_bool("unaligned", sym.unaligned);
  add_bool("restrict", sym.restrict);
  if (sym.inner_type) {
    obj.add("inner_type", raw(*sym.inner_type));
  }
  if (sym.enum_real_type) {
    obj.add("enum_real_type", raw(*sym.enum_real_type));
  }
  if (!sym.simple_type.empty()) {
    obj.add("simple_type", sym.simple_type);
  }
  if (!sym.name.empty()) {
    auto names = builder.array();
    for (auto & name : sym.name) {
      names->add(raw(*name));
    }
    obj.add("name", std::move(names));
  }
  if (sym.com_interface) {
    obj.add("com_interface", raw(*sym.com_interface));
  }
  if (!sym.template_parameters.empty()) {
    auto params = builder.array();
    for (auto & param : sym.template_parameters) {
      if (param) {
        auto p = builder.object();
        if (param->type) {
          p->add("type", raw(*param->type));
          if (param->pointer) {
            p->add("pointer", param->pointer);
          }
        } else {
          p->add("constant_value", param->constant_value);
        }
        params->add(std::move(p));
      }
    }
    obj.add("template_parameters", std::move(params));
  }
  handle_scope(obj, sym);
  handle_method_property(obj, sym);
  if (!sym.calling_convention.empty()) {
    obj.add("calling_convention", sym.calling_convention);
  }
  add_bool("is_ctor", sym.is_ctor);
  add_bool("is_dtor", sym.is_dtor);
  if (!sym.method_name.empty()) {
    obj.add("method_name", sym.method_name);
  }
  if (!sym.class_name.empty()) {
    auto names = builder.array();
    for (auto & name : sym.class_name) {
      names->add(raw(*name));
    }
    obj.add("class_name", std::move(names));
  }
  if (!sym.instance_name.empty()) {
    auto names = builder.array();
    for (auto & name : sym.instance_name) {
      names->add(raw(*name));
    }
    obj.add("instance_name", std::move(names));
  }
  if (sym.retval) {
    obj.add("retval", raw(*sym.retval));
  }
  if (!sym.args.empty()) {
    auto args = builder.array();
    for (auto & arg : sym.args) {
      args->add(raw(*arg));
    }
    obj.add("args", std::move(args));
  }
  if (sym.n1 || sym.n2 || sym.n3 || sym.n4) {
    obj.add("n1", sym.n1);
    obj.add("n2", sym.n2);
    obj.add("n3", sym.n3);
    obj.add("n4", sym.n4);
  }
  return std::move(node);
}

} // namespace demangle

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
