/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// Name        :
// Author      : Avi
// Revision    : $Revision: #85 $
//
// Copyright 2009- ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//
// Description :
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// file deepcode ignore CppConstantBinaryExpression: <comment the reason here>
#include <boost/core/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "BoostPythonUtil.hpp"
#include "Defs.hpp"
#include "DefsDoc.hpp"
#include "Family.hpp"
#include "NodeUtil.hpp"
#include "Suite.hpp"
#include "Task.hpp"

using namespace ecf;
using namespace boost::python;
using namespace std;
namespace bp = boost::python;

// See: http://wiki.python.org/moin/boost.python/HowTo#boost.function_objects

/// Since we don't pass in a child pos, the nodes are added to the end
family_ptr add_family(NodeContainer* self, family_ptr f) {
    self->addFamily(f);
    return f;
}
task_ptr add_task(NodeContainer* self, task_ptr t) {
    self->addTask(t);
    return t;
}

suite_ptr add_clock(suite_ptr self, const ClockAttr& clk) {
    self->addClock(clk);
    return self;
}
suite_ptr add_end_clock(suite_ptr self, const ClockAttr& clk) {
    self->add_end_clock(clk);
    return self;
}

// Sized and Container protocol
size_t family_len(family_ptr self) {
    return self->nodeVec().size();
}
size_t suite_len(suite_ptr self) {
    return self->nodeVec().size();
}
bool family_container(family_ptr self, const std::string& name) {
    size_t pos;
    return (self->findImmediateChild(name, pos)) ? true : false;
}
bool suite_container(suite_ptr self, const std::string& name) {
    size_t pos;
    return (self->findImmediateChild(name, pos)) ? true : false;
}

// Context management, Only used to provide indentation
suite_ptr suite_enter(suite_ptr self) {
    return self;
}
bool suite_exit(suite_ptr self, const bp::object& type, const bp::object& value, const bp::object& traceback) {
    return false;
}
family_ptr family_enter(family_ptr self) {
    return self;
}
bool family_exit(family_ptr self, const bp::object& type, const bp::object& value, const bp::object& traceback) {
    return false;
}

family_ptr family_init(const std::string& name, bp::list the_list, bp::dict kw) {
    // cout << "family_init : " << name << " the_list: " << len(the_list) << " dict: " << len(kw) << endl;
    family_ptr node = Family::create(name);
    (void)NodeUtil::add_variable_dict(node, kw);
    (void)NodeUtil::node_iadd(node, the_list);
    return node;
}

suite_ptr suite_init(const std::string& name, bp::list the_list, bp::dict kw) {
    // cout << "suite_init : " << name << " the_list: " << len(the_list) << " dict: " << len(kw) << endl;
    suite_ptr node = Suite::create(name);
    (void)NodeUtil::add_variable_dict(node, kw);
    (void)NodeUtil::node_iadd(node, the_list);
    return node;
}

void export_SuiteAndFamily() {
    // Turn off proxies by passing true as the NoProxy template parameter.
    // shared_ptrs don't need proxies because calls on one a copy of the
    // shared_ptr will affect all of them (duh!).
    class_<std::vector<family_ptr>>("FamilyVec", "Hold a list of `family`_ nodes")
        .def(vector_indexing_suite<std::vector<family_ptr>, true>());

    class_<std::vector<suite_ptr>>("SuiteVec", "Hold a list of `suite`_ nodes's")
        .def(vector_indexing_suite<std::vector<suite_ptr>, true>());

    // choose the correct overload
    class_<NodeContainer, bases<Node>, boost::noncopyable>("NodeContainer", DefsDoc::node_container_doc(), no_init)
        .def("__iter__", bp::range(&NodeContainer::node_begin, &NodeContainer::node_end))
        .def("add_family", &NodeContainer::add_family, DefsDoc::add_family_doc())
        .def("add_family", add_family)
        .def("add_task", &NodeContainer::add_task, DefsDoc::add_task_doc())
        .def("add_task", add_task)
        .def("find_node", &NodeContainer::find_by_name, "Find immediate child node given a name")
        .def("find_task", &NodeContainer::findTask, "Find a task given a name")
        .def("find_family", &NodeContainer::findFamily, "Find a family given a name")
        .add_property(
            "nodes", bp::range(&NodeContainer::node_begin, &NodeContainer::node_end), "Returns a list of Node's");

    class_<Family, bases<NodeContainer>, family_ptr>("Family", DefsDoc::family_doc())
        .def("__init__", raw_function(&NodeUtil::node_raw_constructor, 1)) // will call -> family_init
        .def("__init__", make_constructor(&family_init), DefsDoc::family_doc())
        .def("__init__", make_constructor(&Family::create_me), DefsDoc::family_doc())
        .def(self == self)                      // __eq__
        .def("__str__", &Family::to_string)     // __str__
        .def("__copy__", copyObject<Family>)    // __copy__ uses copy constructor
        .def("__enter__", &family_enter)        // allow with statement, hence indentation support
        .def("__exit__", &family_exit)          // allow with statement, hence indentation support
        .def("__len__", &family_len)            // Implement sized protocol for immediate children
        .def("__contains__", &family_container) // Implement container protocol for immediate children
        ;
#if ECF_ENABLE_PYTHON_PTR_REGISTER
    bp::register_ptr_to_python<family_ptr>(); // needed for mac and boost 1.6
#endif

    class_<Suite, bases<NodeContainer>, suite_ptr>("Suite", DefsDoc::suite_doc())
        .def("__init__", raw_function(&NodeUtil::node_raw_constructor, 1)) // will call -> suite_init
        .def("__init__", make_constructor(&suite_init), DefsDoc::suite_doc())
        .def("__init__", make_constructor(&Suite::create_me), DefsDoc::suite_doc())
        .def(self == self)                     // __eq__
        .def("__str__", &Suite::to_string)     // __str__
        .def("__copy__", copyObject<Suite>)    // __copy__ uses copy constructor
        .def("__enter__", &suite_enter)        // allow with statement, hence indentation support
        .def("__exit__", &suite_exit)          // allow with statement, hence indentation support
        .def("__len__", &suite_len)            // Implement sized protocol for immediate children
        .def("__contains__", &suite_container) // Implement container protocol for immediate children
        .def("add_clock", &add_clock)
        .def("get_clock", &Suite::clockAttr, "Returns the `suite`_ `clock`_")
        .def("add_end_clock", &add_end_clock, "End clock, used to mark end of simulation")
        .def("get_end_clock", &Suite::clock_end_attr, "Return the suite's end clock. Can be NULL")
        .def("begun", &Suite::begun, "Returns true if the `suite`_ has begun, false otherwise");
#if ECF_ENABLE_PYTHON_PTR_REGISTER
    bp::register_ptr_to_python<suite_ptr>(); // needed for mac and boost 1.6
#endif
}
