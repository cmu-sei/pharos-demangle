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

#include "demangle.cpp"

#include <Python.h>

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
using namespace boost::python;

PyObject* pythondemangle(char* mangled)
{
    std::stringstream final_out_stream("");
    try {

        std::unique_ptr<demangle::JsonOutput> json_output;
        auto builder = json::simple_builder();
        json_output = std::unique_ptr<demangle::JsonOutput>(new JsonOutput(*builder));

        bool debug = false;
        auto t = demangle::visual_studio_demangle(mangled, debug);

        auto node = json_output->minimal(*t);
        node->add("symbol", mangled);
        //node->add("demangled", str(*t));

        final_out_stream << *node;

    }
    catch (const demangle::Error& e) {

        final_out_stream << "{}";

    }

    // necessary to copy the string or we lose it (temp memory)
    const std::string tmp = final_out_stream.str();
    const char* final_out = tmp.c_str();

    // convert to cstring for python
    return PyUnicode_FromString(final_out);
}

BOOST_PYTHON_MODULE(pydemangle)
{
    def("demangle", pythondemangle);
}

