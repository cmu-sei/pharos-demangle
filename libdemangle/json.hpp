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

#ifndef Include_json
#define Include_json

#include <cstddef>              // std::nullptr_t
#include <memory>               // std::unique_ptr
#include <ostream>              // std::ostrem
#include <cstdint>              // std::nullptr_t
#include <string>               // std::string

namespace json {
namespace wrapper {

class Builder;
class Simple;;

class Node {
 public:
  virtual std::ostream & write(std::ostream & stream) const = 0;
  virtual ~Node() = default;
};

using NodeRef = std::unique_ptr<Node>;

class Array : public Node {
 public:
  virtual void add(NodeRef o) = 0;
  virtual void add(Simple && v) = 0;
};

using ArrayRef = std::unique_ptr<Array>;

class Object : public Node {
 public:
  virtual void add(std::string const & str, NodeRef o) = 0;
  virtual void add(std::string const & str, Simple && v) = 0;
  virtual void add(std::string && str, NodeRef o) = 0;
  virtual void add(std::string && str, Simple && v) = 0;
};

using ObjectRef = std::unique_ptr<Object>;

class Simple {
 private:
  enum type_t { INT, DOUBLE, BOOL, NULLP, STRING, CSTRING, STRINGRR };
  type_t type;
  union {
    std::intmax_t i;
    double d;
    bool b;
    std::string const * s;
    char const * c;
    std::string r;
  };

 public:
  Simple(short int v) : type(INT), i(std::intmax_t(v)) {}
  Simple(unsigned short int v) : type(INT), i(std::intmax_t(v)) {}
  Simple(int v) : type(INT), i(std::intmax_t(v)) {}
  Simple(unsigned int v) : type(INT), i(std::intmax_t(v)) {}
  Simple(long int v) : type(INT), i(std::intmax_t(v)) {}
  Simple(unsigned long int v) : type(INT), i(std::intmax_t(v)) {}
  Simple(long long int v) : type(INT), i(std::intmax_t(v)) {}
  Simple(unsigned long long int v) : type(INT), i(std::intmax_t(v)) {}
  Simple(double v) : type(DOUBLE), d(v) {}
  Simple(bool v) : type(BOOL), b(v) {}
  Simple() : type(NULLP) {}
  Simple(std::nullptr_t) : type(NULLP) {}
  Simple(std::string const & v) : type(STRING), s(&v) {}
  Simple(char const * v) : type(CSTRING), c(v) {}
  Simple(std::string && v) : type(STRINGRR), r(std::move(v)) {}

  ~Simple() {
    if (type == STRINGRR) {
      r.std::string::~string();
    }
  }

  NodeRef apply(Builder const & b);
};

class Builder {
 public:

  virtual NodeRef simple(std::intmax_t i) const = 0;
  virtual NodeRef simple(double d) const = 0;
  virtual NodeRef simple(bool b) const = 0;
  virtual NodeRef null() const = 0;
  virtual NodeRef simple(std::string const & str) const = 0;
  virtual NodeRef simple(std::string && str) const = 0;
  virtual ArrayRef array() const = 0;
  virtual ObjectRef object() const = 0;

  NodeRef simple(std::nullptr_t) const {
    return null();
  }
  NodeRef simple(char const * s) const {
    return s ? simple(std::string(s)) : null();
  }
  virtual NodeRef simple(Simple && val) const {
    return val.apply(*this);
  }
};

std::ostream & operator<<(std::ostream & stream, Node const & n);

} // namespace json::wrapper

std::unique_ptr<wrapper::Builder> simple_builder();

} // namespace json

#endif // Include_json

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
