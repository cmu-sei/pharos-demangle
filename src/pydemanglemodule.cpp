
#include "demangle.cpp"

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
using namespace boost::python;

void internalmain(char* name)
{
  Demangler demangler;
  demangler.set_json(true);
  demangler.set_raw(true);

  std::string argstr(name);
  std::vector<std::string> args(1,argstr);

  // Do the demangling
  Driver driver(demangler);
  driver.nofile = true;
  driver.json = true;
  driver.batch = true;
  driver.run(args);
  //bool success = driver.run(args);
}

BOOST_PYTHON_MODULE(pydemangle)
{
    def("demangle", internalmain);
}
 
