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

#ifndef Include_Demangle_H
#define Include_Demangle_H

#include <string>
#include <stdexcept>
#include <memory>
#include <vector>

#include "codes.hpp"

namespace demangle {

// Thrown for errors encountered while demangling names.
class Error : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

enum class SymbolType {
  Unspecified,
  StaticClassMember,
  GlobalObject,
  GlobalFunction,
  ClassMethod,
  RTTI,
  VTable,
  String,
  VtorDisp,
  StaticGuard,
  MethodThunk,
  HexSymbol
};

enum class Scope {
  Unspecified,
  Private,
  Protected,
  Public
};

enum class MethodProperty {
  Unspecified,
  Ordinary,
  Static,
  Virtual,
  Thunk
};

enum class Distance {
  Unspecified,
  Near,
  Far,
  Huge
};


// Forward declaration of the core "type" definition.
class DemangledType;

using DemangledTypePtr = std::shared_ptr<DemangledType>;

// Vectors of demangled types are used for several purposes.  Arguments to a function, the
// terms in a fully qualified name, and a stack of names or types for numbered references.
// While the underlying types are identical in practice, I'm going to attempt to keep them
// separate logically in case they ever need to diverge.
typedef std::vector<DemangledTypePtr> FunctionArgs;
typedef std::vector<DemangledTypePtr> FullyQualifiedName;
typedef std::vector<DemangledTypePtr> ReferenceStack;

// The classes describing the demangled results are demangler independent, but strictly
// speaking the boolean match" flag on the str() methods is specific to the Visual Studio
// Demangler.  Solutions could include overridding the str() implementation for each demangler,
// or just ignoring this problem since every demangler implementation ought to be emitting a
// consistent C++ representation.

// Template parameters can be either a type or a constant.
class DemangledTemplateParameter {
 public:
  // If the type pointer is NULL, then the constant value is used.
  DemangledTypePtr type;
  int64_t constant_value;

  // If pointer is true, the parameter is a constant pointer to type.
  bool pointer = false;

  DemangledTemplateParameter(DemangledTypePtr t);
  DemangledTemplateParameter(int64_t c);
};

using DemangledTemplateParameterPtr = std::shared_ptr<DemangledTemplateParameter>;

typedef std::vector<DemangledTemplateParameterPtr> DemangledTemplate;

class DemangledType {


 public:
  bool is_const = false;
  bool is_volatile = false;
  bool is_reference = false;
  bool is_pointer = false;
  bool is_array = false;

  // Array dimensions
  std::vector<uint64_t> dimensions;

  // Hacky thing for complex types that can't get rendered any better than putting them inside
  // a pair of single quotes.  e.g. ?X@??Y@@9@9 demangles to "`Y'::X".  The extra quotes aren't
  // present if this is the outermost symbol, but are if it's part of a namespace? ...
  bool is_embedded = false;

  // Currently used for signaling between functions, but might be useful in general.
  bool is_func = false;

  // Poorly understood features involving storage classes, see update_storage_class()...
  bool is_based = false;
  bool is_member = false;

  // True if the namespace is anonymous.  The simple_type string then contains the unique
  // identifier name that's not typically shown for anonymous namespaces.
  bool is_anonymous = false;

  // This is handled horribly by Microsoft, and equally horribly by me.  I want to think some
  // more about the correct approach after I know more about the other $$ cases.  For this
  // particular one, I would expect the correct answer to something more like a reference to a
  // reference to a type (although we may still need come custom outputing to avoid getting a
  // space between the references.)  Or maybe is_reference, is_pointer, and is_refref should be
  // an enum?  Apparently the correct name for this is "rvalue reference"?
  bool is_refref = false;

  // Enum controlling how to interpret this type.
  // 1=namespace, 2=static class member, 3=global object, 4=global function, 5=class method
  SymbolType symbol_type = SymbolType::Unspecified;

  // Really an enum: 0=near, 1=far, 2=huge
  Distance distance = Distance::Unspecified;

  // ptr64 is an integer because a global symbol that is a __ptr64 pointer can itself be
  // __ptr64.  A count of "2" here indicates this corner case.
  int ptr64 = 0;
  bool unaligned = false;
  bool restrict = false;

  bool is_gc = false;
  bool is_pin = false;

  // The type pointed to or referenced.
  DemangledTypePtr inner_type;

  // The real type of an enum (Usually int, and often coded assuch regardless).
  DemangledTypePtr enum_real_type;


  // If simple_code is undefined, use simple_string instead.  These fields hold function names,
  // type names, namespace names, and a bunch of other things as well.  simple_string is used
  // iff simple_code == Code::UNDEFINED.
  Code simple_code = Code::UNDEFINED;
  std::string simple_string;

  // The fully qualified name of a complex type (e.g. a templated class).
  FullyQualifiedName name;

  // I'm not sure that I've named this correctly.  Set by symbol types 6 & 7.
  FullyQualifiedName com_interface;

  // If the class was templated, these are the parameters.
  DemangledTemplate template_parameters;

  // Scope (private, protected, public) of class method. Only applicable to class methods.
  Scope scope = Scope::Unspecified;

  // Class method property (static, virtual, thunk). Only applicable to class methods.
  MethodProperty method_property = MethodProperty::Unspecified;

  // Calling convention
  std::string calling_convention;

  // Was this symbol exported?
  bool is_exported = false;

  // Ctors and dtors
  bool is_ctor = false;
  bool is_dtor = false;

  // The fully qualified name of a exported variable.   Names are still messy. :-(
  FullyQualifiedName instance_name;

  // Return value type.  Applicable only to functions and class methods.
  DemangledTypePtr retval;

  // Function arguments.  Applicable only to functions and class methods.
  FunctionArgs args;

  // And then the really obscure values (like parameters for RTTI data structures).
  std::vector<int64_t> n;

  // extern "C" (which shouldn't be mangled, but Microsoft)
  bool extern_c = false;

  DemangledType() = default;
  DemangledType(const DemangledType & other) = default;
  DemangledType(DemangledType && other) = default;
  ~DemangledType() = default;
  DemangledType & operator=(const DemangledType & other) = default;
  DemangledType & operator=(DemangledType && other) = default;

  DemangledType(std::string && simple_name) : simple_string(std::move(simple_name)) {}
  DemangledType(std::string const & simple_name) : simple_string(simple_name) {}
  DemangledType(char const * simple_name) : simple_string(simple_name) {}
  DemangledType(Code code) : simple_code(code) {}

  template <typename... T>
  DemangledTypePtr & add_name(T &&... nm) {
    name.push_back(std::make_shared<DemangledType>(std::forward<T>(nm)...));
    return name.back();
  }
};

// Main entry point to demangler
DemangledTypePtr visual_studio_demangle(const std::string & mangled, bool debug = false);

} // namespace demangle

#endif // Include_Demangle_H

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
