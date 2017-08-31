// Copyright 2017 Carnegie Mellon University.  See LICENSE file for terms.

#include "json.hpp"

#include <vector>               // std::vector
#include <map>                  // std::map
#include <utility>              // std::move
#include <cctype>               // std::iscntrl
#include <ios>                  // std::hex
#include <iomanip>              // std::setw

namespace json {

#if __cplusplus < 201402L
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#else
using std::make_unique;
#endif

namespace wrapper {
NodeRef Simple::apply(Builder const & builder)
{
  switch(type) {
   case INT:
    return builder.simple(i);
   case DOUBLE:
    return builder.simple(d);
   case BOOL:
    return builder.simple(b);
   case NULLP:
    return builder.null();
   case STRING:
    return builder.simple(*s);
   case CSTRING:
    return builder.simple(c);
   case STRINGRR:
    return builder.simple(std::move(r));
  }
  return builder.null();
}
} // namespace wrapper

namespace simple {

namespace w = json::wrapper;

template <typename T>
class Simple : public w::Node
{
 protected:
  T val;
 public:
  Simple(T const & v) : val(v) {}
  Simple(T && v) : val(std::move(v)) {}
  std::ostream & write(std::ostream & stream) const override {
    return stream << val;
  }
};

class Bool : public Simple<bool> {
 public:
  using Simple<bool>::Simple;
  std::ostream & write(std::ostream & stream) const override {
    return stream << (val ? "true" : "false");
  }
};

std::ostream & output_string(std::ostream & stream, std::string const & s) {
    stream << '"';
    for (auto c : s) {
      const char *v;
      switch(c) {
       case '"':
        v = "\\\""; break;
       case '\\':
        v = "\\\\"; break;
       case '/':
        v = "\\/";  break;
       case '\b':
        v = "\\b";  break;
       case '\f':
        v = "\\f";  break;
       case '\n':
        v = "\\n";  break;
       case '\r':
        v = "\\r";  break;
       case '\t':
        v = "\\t";  break;
       default:
        // This should be unicode aware, but not yet
        if (std::iscntrl(c)) {
          auto flags = stream.flags();
          auto fill = stream.fill('0');
          stream << "\\u00" << std::setw(2) << std::hex << unsigned(c);
          stream.flags(flags);
          stream.fill(fill);
          continue;
        }
        stream.put(c);
        continue;
      }
      stream << v;
    }
    return stream << '"';
}

class String : public Simple<std::string> {
 public:
  using Simple<std::string>::Simple;
  std::ostream & write(std::ostream & stream) const override {
    return output_string(stream, val);
  }
};

class Null : public w::Node
{
 public:
  std::ostream & write(std::ostream & stream) const override {
    return stream << "null";
  }
};

class Array : public w::Array
{
  std::vector<w::NodeRef> vec;
 public:
  void add(w::NodeRef o) override {
    vec.push_back(std::move(o));
  }

  void add(w::Simple && o) override;

  std::ostream & write(std::ostream & stream) const override {
    stream << '[';
    auto n = begin(vec), last = end(vec);
    if (n != last) {
      while (true) {
        (*n++)->write(stream);
        if (n == last) break;
        stream << ',';
      }
    }
    return stream << ']';
  }
};

class Object : public w::Object {
  std::map<std::string, w::NodeRef> map;
 public:
  void add(std::string const & str, w::NodeRef o) override {
    map.emplace(str, std::move(o));
  }
  void add(std::string && str, w::NodeRef o) override {
    map.emplace(std::move(str), std::move(o));
  }

  void add(std::string const & str, w::Simple && v) override;
  void add(std::string && str, w::Simple && v) override;

  std::ostream & write(std::ostream & stream) const override {
    stream << '{';
    auto n = begin(map), last = end(map);
    if (n != last) {
      while (true) {
        output_string(stream, n->first) << ':';
        n++->second->write(stream);
        if (n == last) break;
        stream << ',';
      }
    }
    return stream << '}';
  }
};

class Builder : public w::Builder
{
 public:
  using w::Builder::simple;

  w::NodeRef simple(std::intmax_t i) const override {
    return make_unique<Simple<std::intmax_t>>(i);
  }
  w::NodeRef simple(double d) const override {
    return make_unique<Simple<double>>(d);
  }
  w::NodeRef simple(bool b) const override {
    return make_unique<Bool>(b);
  }
  w::NodeRef null() const override {
    return make_unique<Null>();
  }
  w::NodeRef simple(std::string && s) const override {
    return make_unique<String>(std::move(s));
  }
  w::NodeRef simple(std::string const & s) const override {
    return make_unique<String>(s);
  }
  w::ArrayRef array() const override {
    return make_unique<Array>();
  }
  w::ObjectRef object() const override {
    return make_unique<Object>();
  }
};

void Array::add(w::Simple && v) {
  add(Builder{}.simple(std::move(v)));
}

void Object::add(std::string const & str, w::Simple && v) {
  add(str, Builder{}.simple(std::move(v)));
}

void Object::add(std::string && str, w::Simple && v) {
  add(std::move(str), Builder{}.simple(std::move(v)));
}

} // namespace simple

namespace wrapper {

std::ostream & operator<<(std::ostream & stream, Node const & n)
{
  n.write(stream);
  return stream;
}

} // namespace wrapper

std::unique_ptr<wrapper::Builder> simple_builder()
{
  return make_unique<simple::Builder>();
}

} // namespace json

/* Local Variables:   */
/* mode: c++          */
/* fill-column:    95 */
/* comment-column: 0  */
/* End:               */
