/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// Name        :
// Author      : Avi
// Revision    : $Revision: #85 $
//
// Copyright 2009-2017 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//
// Description :
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include "Task.hpp"
#include "DefsDoc.hpp"
#include "BoostPythonUtil.hpp"

using namespace ecf;
using namespace boost::python;
using namespace std;
namespace bp = boost::python;

task_ptr task_enter(task_ptr self) { return self;}
bool task_exit(task_ptr self,const boost::python::object& type,const boost::python::object& value,const boost::python::object& traceback){return false;}

// See: http://wiki.python.org/moin/boost.python/HowTo#boost.function_objects

void export_Task()
{
   // Turn off proxies by passing true as the NoProxy template parameter.
   // shared_ptrs don't need proxies because calls on one a copy of the
   // shared_ptr will affect all of them (duh!).
   class_<std::vector<task_ptr> >("TaskVec","Hold a list of :term:`task` nodes")
   .def(vector_indexing_suite<std::vector<task_ptr>, true >()) ;

   class_<Submittable, bases<Node>, boost::noncopyable >("Submittable",DefsDoc::submittable_doc(), no_init)
   .def("get_jobs_password" ,      &Submittable::jobsPassword, return_value_policy<copy_const_reference>(),        "The password. This generated by server")
   .def("get_process_or_remote_id",&Submittable::process_or_remote_id, return_value_policy<copy_const_reference>(),"The process or remote id of the running job")
   .def("get_try_no" ,             &Submittable::tryNo,                                                            "The current try number as a string.")
   .def("get_int_try_no" ,         &Submittable::try_no,                                                           "The current try number as integer.")
   .def("get_aborted_reason" ,     &Submittable::abortedReason,return_value_policy<copy_const_reference>(),        "If node was aborted and a reason was provided, return the string")
   ;

   class_<Task, bases<Submittable>, task_ptr>("Task",DefsDoc::task_doc() )
   .def("__init__",make_constructor(&Task::create), DefsDoc::task_doc())
   .def(self == self )                        // __eq__
   .def("__enter__", &task_enter)             // allow with statement, hence indentation support
   .def("__exit__",  &task_exit)              // allow with statement, hence indentation support
   .def("__str__",         &Task::to_string)  // __str__
   .def("__copy__",  copyObject<Task>)        // __copy__ uses copy constructor
   .add_property("aliases",boost::python::range( &Task::alias_begin,  &Task::alias_end), "Returns a list of aliases")
   .add_property("nodes",  boost::python::range( &Task::alias_begin,  &Task::alias_end), "Returns a list of aliases")
   ;

   class_<Alias, bases<Submittable>, alias_ptr>("Alias",DefsDoc::alias_doc(),no_init)
   .def(self == self )                  // __eq__
   .def("__str__", &Alias::to_string)   // __str__
   .def("__copy__", copyObject<Alias>)  // __copy__ uses copy constructor
   ;
}
