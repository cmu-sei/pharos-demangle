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
   case SymbolType::RTTI:
    symbol_type = "RTTI";
    break;
   case SymbolType::VTable:
    symbol_type = "vtable";
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
   case SymbolType::HexSymbol:
    symbol_type = "hex symbol";
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

  obj.add("text", text.convert(sym));

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

  auto add_rlist = [&obj, this](char const * name, FullyQualifiedName const & names) {
                    if (!names.empty()) {
                      auto nlist = builder.array();
                      for (auto i = names.rbegin(); i != names.rend(); ++i) {
                        nlist->add(raw(**i));
                      }
                      obj.add(name, std::move(nlist));
                    }
                   };

  auto add_list = [&obj, this](char const * name, FullyQualifiedName const & names) {
                    if (!names.empty()) {
                      auto nlist = builder.array();
                      for (auto & n : names) {
                        nlist->add(raw(*n));
                      }
                      obj.add(name, std::move(nlist));
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
  add_bool("is_anonymous", sym.is_anonymous);
  add_bool("is_refref", sym.is_refref);

  handle_symbol_type(obj, sym);
  handle_distance(obj, sym);
  if (sym.ptr64) {
    obj.add("ptr64", sym.ptr64);
  }
  add_bool("unaligned", sym.unaligned);
  add_bool("restrict", sym.restrict);
  add_bool("is_gc", sym.is_gc);
  add_bool("is_pin", sym.is_pin);
  if (sym.inner_type) {
    obj.add("inner_type", raw(*sym.inner_type));
  }
  if (sym.enum_real_type) {
    obj.add("enum_real_type", raw(*sym.enum_real_type));
  }
  if (!sym.simple_string.empty()) {
    obj.add("simple_string", sym.simple_string);
  }
  if (sym.simple_code != Code::UNDEFINED) {
    obj.add("simple_code", code_string(sym.simple_code));
  }
  add_rlist("name", sym.name);
  add_list("com_interface", sym.com_interface);
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
  add_list("instance_name", sym.instance_name);
  if (sym.retval) {
    obj.add("retval", raw(*sym.retval));
  }
  add_list("args", sym.args);
  if (!sym.n.empty()) {
    auto values = builder.array();
    for (auto & n : sym.n) {
      values->add(n);
    }
    obj.add("n", std::move(values));
  }
  add_bool("extern_c", sym.extern_c);

  return node;
}

JsonOutput::ObjectRef JsonOutput::minimal(DemangledType const & sym) const
{
  auto node = builder.object();
  auto & obj = *node;
  
  auto add_bool = [&obj, this](char const * name, bool val) {
                    if (val) {
                      obj.add(name, val);
                    }
                  };

  handle_symbol_type(obj, sym);
  handle_scope(obj, sym);

  auto add_string = [&obj](char const * name, std::string && val) {
                      if (!val.empty()) {
                        obj.add(name, std::move(val));
                      }
                    };
           
  if (sym.symbol_type == SymbolType::GlobalFunction
      || sym.symbol_type == SymbolType::ClassMethod)
  {               
    if (!sym.calling_convention.empty()) {
      obj.add("calling_convention", sym.calling_convention);
    }
    handle_distance(obj, sym);
    add_string("class_name", text.get_class_name(sym));
    add_string("function_name", text.get_method_name(sym));
    add_string("function_signature", text.get_method_signature(sym));

    auto args = builder.array();
    for (auto & arg : sym.args) {
      args->add(text.convert(*arg));
    }
    obj.add("args", std::move(args));
    add_string("return_type", text.convert(*sym.retval));
    bool is_ctor = false;
    bool is_dtor = false;
    for (auto & part : sym.name) {
        is_ctor |= part->is_ctor;
        is_dtor |= part->is_dtor;
    }
    add_bool("is_ctor", is_ctor);
    add_bool("is_dtor", is_dtor);    
  } else {
    return raw(sym);
  }

  return node;
}


} // namespace demangle

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
