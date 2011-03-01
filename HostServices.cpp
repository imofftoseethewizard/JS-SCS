/******************************************************************************

  HostServices.cpp

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


#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"

#include "HostServices.h"
#include "SecureConnection.h"


///////////////////////////////////////////////////////////////////////////////
/// @fn HostServices::HostServices(const HostServicesPluginPtr& plugin, const FB::BrowserHostPtr host)
///
/// @brief  Constructor for your JSAPI object.  You should register your methods, properties, and events
///         that should be accessible to Javascript from here.
///
/// @see FB::JSAPIAuto::registerMethod
/// @see FB::JSAPIAuto::registerProperty
/// @see FB::JSAPIAuto::registerEvent
///////////////////////////////////////////////////////////////////////////////
HostServices::HostServices(boost::shared_ptr<HostServicesPlugin> plugin,
						   const FB::BrowserHostPtr& host)
  : m_plugin(plugin),
    m_host(host)
{
    registerMethod("SecureConnection", make_method(this, &HostServices::createSecureConnection));
    registerProperty("version", make_property(this, &HostServices::get_version));
}

///////////////////////////////////////////////////////////////////////////////
/// @fn HostServices::~HostServices()
///
/// @brief  Destructor.  Remember that this object will not be released until
///         the browser is done with it; this will almost definitely be after
///         the plugin is released.
///////////////////////////////////////////////////////////////////////////////
HostServices::~HostServices()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @fn HostServicesPluginPtr HostServices::plugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
HostServicesPluginPtr HostServices::plugin()
{
    HostServicesPluginPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}



// Read-only property version
std::string HostServices::get_version()
{
    return "0.01.01";
}



// SecureConnection (JS)constructor 

FB::JSAPIPtr
HostServices::createSecureConnection(const std::string& user,
						 const std::string& hostName,
						 boost::optional<unsigned int> port)
{
  return boost::make_shared<SecureConnection>(FB::ptr_cast<HostServices>(shared_from_this()),
					      user, hostName, port.get_value_or(22));
}

// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
