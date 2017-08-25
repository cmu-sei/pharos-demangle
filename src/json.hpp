// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#ifndef Include_json
#define Include_json

#include <cstddef>              // std::nullptr_t
#include <memory>               // std::unique_ptr
#include <ostream>              // std::ostrem
#include <cstdint>              // std::nullptr_t
#include <string>               // std::string

namespace json {
namespace wrapper {

class Node {
 public:
  virtual std::ostream & write(std::ostream & stream) const = 0;
  virtual ~Node() = default;
};

using NodeRef = std::unique_ptr<Node>;

class Array : public Node {
 public:
  virtual void add(NodeRef o) = 0;
};

using ArrayRef = std::unique_ptr<Array>;

class Object : public Node {
 public:
  virtual void add(std::string const & str, NodeRef o) = 0;
  virtual void add(std::string && str, NodeRef o) = 0;
};

using ObjectRef = std::unique_ptr<Object>;

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
};

std::ostream & operator<<(std::ostream & stream, Node const & n);
std::ostream & operator<<(std::ostream & stream, NodeRef const & n);

} // namespace json::wrapper

std::unique_ptr<wrapper::Builder> simple_builder();

} // namespace json

#endif // Include_json

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
