/**********************************************************\

  Auto-generated SecureConnectionServices.cpp

\**********************************************************/

#include <string>
#include <sstream>

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"

#include "SecureConnectionServices.h"
#include "SecureConnection.h"

///////////////////////////////////////////////////////////////////////////////
/// @fn SecureConnectionServices::SecureConnectionServices(const SecureConnectionServicesPluginPtr& plugin, const FB::BrowserHostPtr host)
///
/// @brief  Constructor for your JSAPI object.  You should register your methods, properties, and events
///         that should be accessible to Javascript from here.
///
/// @see FB::JSAPIAuto::registerMethod
/// @see FB::JSAPIAuto::registerProperty
/// @see FB::JSAPIAuto::registerEvent
///////////////////////////////////////////////////////////////////////////////
SecureConnectionServices::SecureConnectionServices(boost::shared_ptr<SecureConnectionServicesPlugin> plugin,
						   const FB::BrowserHostPtr& host)
  : m_plugin(plugin),
    m_host(host)
{
    registerMethod("echo",      make_method(this, &SecureConnectionServices::echo));
    registerMethod("testEvent", make_method(this, &SecureConnectionServices::testEvent));
    registerMethod("SecureConnection", make_method(this, &SecureConnectionServices::createSecureConnection));

    // Read-write property
    registerProperty("testString",
                     make_property(this,
                        &SecureConnectionServices::get_testString,
                        &SecureConnectionServices::set_testString));

    // Read-only property
    registerProperty("version",
                     make_property(this,
                        &SecureConnectionServices::get_version));
    
    
    registerEvent("onfired");    
}

///////////////////////////////////////////////////////////////////////////////
/// @fn SecureConnectionServices::~SecureConnectionServices()
///
/// @brief  Destructor.  Remember that this object will not be released until
///         the browser is done with it; this will almost definitely be after
///         the plugin is released.
///////////////////////////////////////////////////////////////////////////////
SecureConnectionServices::~SecureConnectionServices()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @fn SecureConnectionServicesPluginPtr SecureConnectionServices::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
SecureConnectionServicesPluginPtr SecureConnectionServices::getPlugin()
{
    SecureConnectionServicesPluginPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}



// Read/Write property testString
std::string SecureConnectionServices::get_testString()
{
    return m_testString;
}
void SecureConnectionServices::set_testString(const std::string& val)
{
    m_testString = val;
}

// Read-only property version
std::string SecureConnectionServices::get_version()
{
    return "0.01.01";
}

// Method echo
FB::variant SecureConnectionServices::echo(const FB::variant& msg)
{
    return msg;
}

void SecureConnectionServices::testEvent(const FB::variant& var)
{
    FireEvent("onfired", FB::variant_list_of(var)(true)(1));
}

// SecureConnection (JS)constructor 

FB::JSAPIPtr
SecureConnectionServices::createSecureConnection(const std::string& user,
						 const std::string& hostName,
						 boost::optional<unsigned int> port)
{
  return boost::make_shared<SecureConnection>(FB::ptr_cast<SecureConnectionServices>(shared_from_this()),
					      user, hostName, port.get_value_or(22));
}
