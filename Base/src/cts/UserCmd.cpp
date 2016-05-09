/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// Name        :
// Author      : Avi
// Revision    : $Revision: #65 $
//
// Copyright 2009-2016 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//
// Description :
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
#include <iostream>
#include <sstream>
#include <fstream>

#include <pwd.h>       /* getpwuid */
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>      /* tolower */
#include <string.h>     // for strerror()
#include <errno.h>      // for errno()

#include "ClientToServerCmd.hpp"

#include "AbstractServer.hpp"
#include "Log.hpp"
#include "Str.hpp"

using namespace std;
using namespace boost;
using namespace ecf;

bool UserCmd::equals(ClientToServerCmd* rhs) const
{
   UserCmd* the_rhs = dynamic_cast< UserCmd* > ( rhs );
   if ( !the_rhs ) return false;
   return user_ == the_rhs->user();
}

bool UserCmd::authenticate(AbstractServer* as, STC_Cmd_ptr& ) const
{
   // The user should NOT be empty. Rather than asserting and killing the server, fail authentication
   // ECFLOW-577 and ECFLOW-512. When user_ empty ??
   if (!user_.empty() && as->authenticateReadAccess(user_)) {

      // Does this user command require write access
      if ( isWrite() ) {
         // command requires write access. Check user has write access
         if ( as->authenticateWriteAccess(user_) ) {
            return true;
         }
         std::string msg = "[ authentication failed ] User ";
         msg += user_;
         msg += " has no *write* access. Please see your administrator.";
         throw std::runtime_error( msg );
      }
      else {
         // read request, and we have read access
         return true;
      }
   }

   std::string msg = "[ authentication failed ] User '";
   msg += user_;
   msg += "' is not allowed any access.";
   throw std::runtime_error( msg );

   return false;
}

void UserCmd::setup_user_authentification()
{
   // Minimise system calls by using static.
   static std::string the_user_name;
   if (the_user_name.empty()) {

      // Get the uid of the running process and use it to get a record from /etc/passwd */
      // getuid() can not fail, but getpwuid can fail.
      errno = 0;
      uid_t real_user_id_of_process = getuid();
      struct passwd * thePassWord = getpwuid ( real_user_id_of_process );
      if (thePassWord == 0 ) {
         if ( errno != 0) {
            std::string theError = strerror(errno);
            throw std::runtime_error("UserCmd::setup_user_authentification: could not determine user name. Because: " + theError);
         }

         std::stringstream ss;
         ss << "UserCmd::setup_user_authentification: could not determine user name for uid " << real_user_id_of_process;
         throw std::runtime_error(ss.str());
      }

      the_user_name = thePassWord->pw_name;  // equivalent to the login name
      if ( the_user_name.empty() ) {
         throw std::runtime_error("UserCmd::setup_user_authentification: could not determine user name. Because: thePassWord->pw_name is empty");
      }
   }

   user_ = the_user_name;
   assert(!user_.empty());
}

void UserCmd::prompt_for_confirmation(const std::string& prompt)
{
   cout << prompt;
   char reply[256];
   cin.getline (reply,256);
   if (reply[0] != 'y' && reply[0] != 'Y') {
      exit(1);
   }
}

std::ostream& UserCmd::user_cmd(std::ostream& os, const std::string& the_cmd) const
{
   return os << the_cmd << " :" << user_;
}

//#define DEBUG_ME 1

void UserCmd::split_args_to_options_and_paths(
          const std::vector<std::string>& args,
          std::vector<std::string>& options,
          std::vector<std::string>& paths)
{
   // ** ECFLOW-137 **, if the trigger expression have a leading '/' then it gets parsed into the paths
   //                   vector and not options
   // This is because boost program options does *NOT* seem to preserve the leading quotes around the
   // trigger/complete expression,  i.e "/suite/t2 == complete"   is read as /suite/t2 == complete
   // However in paths  we do expect to see any spaces

   size_t vec_size = args.size();
   for(size_t i = 0; i < vec_size; i++) {
      if (args[i].empty()) continue;
      if (args[i][0] == '/' && args[i].find(" ") == std::string::npos) {
         paths.push_back(args[i]);
      }
      else {
         options.push_back(args[i]);
      }
   }

#ifdef DEBUG_ME
   std::cout << "split_args_to_options_and_paths\n";
   for(size_t i = 0; i < args.size(); ++i) { std::cout << "args[" << i << "]=" <<args[i] << "\n"; }
   for(size_t i = 0; i < options.size(); ++i) { std::cout << "options[" << i << "]=" << options[i] << "\n"; }
   for(size_t i = 0; i < paths.size(); ++i) { std::cout << "paths[" << i << "]=" << paths[i] << "\n"; }
#endif
}

int UserCmd::time_out_for_load_sync_and_get() { return 600; }

