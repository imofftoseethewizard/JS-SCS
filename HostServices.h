/******************************************************************************

  HostServices.h

  HostServices is the root of the object heirarchy for the
  Javascript API of the plugin. The properties and methods registered in the
  instance constructor will appear on the object created by the <embed> tag in
  the web page.

  ----------------------------------------------------------------------------

  This file is part of JS/HS.

  JS/HS is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  JS/HS is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  JS/HS.  If not, see <http://www.gnu.org/licenses/>.

  ----------------------------------------------------------------------------

  Copyright 2011 Pat M. Lasswell.

 ******************************************************************************/


#include <string>
#include <sstream>

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "JSAPIAuto.h"
#include "BrowserHost.h"

#include "HostServicesPlugin.h"


#ifndef H_HOSTSERVICES
#define H_HOSTSERVICES


FB_FORWARD_PTR(HostServices)
FB_FORWARD_PTR(SecureConnection)


class HostServices : public FB::JSAPIAuto
{
public:
  HostServices(HostServicesPluginPtr plugin,
                           const FB::BrowserHostPtr& host);

  virtual ~HostServices();

  HostServicesPluginPtr plugin();

  std::string get_version();

  FB::JSAPIPtr createSecureConnection(const std::string& user,
                                      const std::string& hostName,
                                      boost::optional<unsigned int> port);
private:
  HostServicesPluginWeakPtr m_plugin;
  FB::BrowserHostPtr m_host;
};

#endif // H_HOSTSERVICES

// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
