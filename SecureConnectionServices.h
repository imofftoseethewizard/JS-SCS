/******************************************************************************

  SecureConnectionServices.h

  SecureConnectionServices is the root of the object heirarchy for the
  Javascript API of the plugin. The properties and methods registered in the
  instance constructor will appear on the object created by the <embed> tag in
  the web page.

  ----------------------------------------------------------------------------

  This file is part of JS/SCS.

  JS/SCS is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  JS/SCS is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  JS/SCS.  If not, see <http://www.gnu.org/licenses/>.

  ----------------------------------------------------------------------------

  Copyright 2011 Pat M. Lasswell.

 ******************************************************************************/


#include <string>
#include <sstream>

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "JSAPIAuto.h"
#include "BrowserHost.h"

#include "SecureConnectionServicesPlugin.h"


#ifndef H_SecureConnectionServices
#define H_SecureConnectionServices


FB_FORWARD_PTR(SecureConnectionServices)
FB_FORWARD_PTR(SecureConnection)


class SecureConnectionServices : public FB::JSAPIAuto
{
public:
  SecureConnectionServices(SecureConnectionServicesPluginPtr plugin,
                           const FB::BrowserHostPtr& host);

  virtual ~SecureConnectionServices();

  SecureConnectionServicesPluginPtr getPlugin();

  std::string get_version();

  FB::JSAPIPtr createSecureConnection(const std::string& user,
                                      const std::string& hostName,
                                      boost::optional<unsigned int> port);
private:
  SecureConnectionServicesPluginWeakPtr m_plugin;
  FB::BrowserHostPtr m_host;
};

#endif // H_SecureConnectionServices

// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
