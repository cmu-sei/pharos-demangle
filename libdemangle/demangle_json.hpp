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

#ifndef Include_demangle_json
#define Include_demangle_json

#include "demangle.hpp"
#include "demangle_text.hpp"
#include "json.hpp"

namespace demangle {

class JsonOutput {
 public:
  using Builder   = json::wrapper::Builder;
  using Object    = json::wrapper::Object;
  using ObjectRef = json::wrapper::ObjectRef;
  using NodeRef   = json::wrapper::NodeRef;

 public:
  JsonOutput(Builder const & b) : builder(b) {}
  void set_attributes(TextOutput::Attributes attr) {
    text.set_attributes(attr);
  }
  ObjectRef convert(DemangledType const & sym) const;
  ObjectRef operator()(DemangledType const & sym) const {
    return convert(sym);
  }
  ObjectRef raw(DemangledType const & sym) const;
  ObjectRef minimal(DemangledType const & sym) const;

 private:
  Builder const & builder;
  TextOutput text;

  void handle_symbol_type(Object & obj, DemangledType const & sym) const;
  void handle_scope(Object & obj, DemangledType const & sym) const;
  void handle_distance(Object & obj, DemangledType const & sym) const;
  void handle_method_property(Object & obj, DemangledType const & sym) const;
  void handle_namespace(Object & obj, DemangledType const & sym) const;
};

} // namespace demangle

#endif // Include_demangle_json

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
