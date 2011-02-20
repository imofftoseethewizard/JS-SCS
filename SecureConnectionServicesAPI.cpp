/**********************************************************\

  Auto-generated SecureConnectionServicesAPI.cpp

\**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"

#include "SecureConnectionServicesAPI.h"

///////////////////////////////////////////////////////////////////////////////
/// @fn SecureConnectionServicesAPI::SecureConnectionServicesAPI(const SecureConnectionServicesPtr& plugin, const FB::BrowserHostPtr host)
///
/// @brief  Constructor for your JSAPI object.  You should register your methods, properties, and events
///         that should be accessible to Javascript from here.
///
/// @see FB::JSAPIAuto::registerMethod
/// @see FB::JSAPIAuto::registerProperty
/// @see FB::JSAPIAuto::registerEvent
///////////////////////////////////////////////////////////////////////////////
SecureConnectionServicesAPI::SecureConnectionServicesAPI(const SecureConnectionServicesPtr& plugin, const FB::BrowserHostPtr& host) : m_plugin(plugin), m_host(host)
{
    registerMethod("echo",      make_method(this, &SecureConnectionServicesAPI::echo));
    registerMethod("testEvent", make_method(this, &SecureConnectionServicesAPI::testEvent));

    // Read-write property
    registerProperty("testString",
                     make_property(this,
                        &SecureConnectionServicesAPI::get_testString,
                        &SecureConnectionServicesAPI::set_testString));

    // Read-only property
    registerProperty("version",
                     make_property(this,
                        &SecureConnectionServicesAPI::get_version));
    
    
    registerEvent("onfired");    
}

///////////////////////////////////////////////////////////////////////////////
/// @fn SecureConnectionServicesAPI::~SecureConnectionServicesAPI()
///
/// @brief  Destructor.  Remember that this object will not be released until
///         the browser is done with it; this will almost definitely be after
///         the plugin is released.
///////////////////////////////////////////////////////////////////////////////
SecureConnectionServicesAPI::~SecureConnectionServicesAPI()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @fn SecureConnectionServicesPtr SecureConnectionServicesAPI::getPlugin()
///
/// @brief  Gets a reference to the plugin that was passed in when the object
///         was created.  If the plugin has already been released then this
///         will throw a FB::script_error that will be translated into a
///         javascript exception in the page.
///////////////////////////////////////////////////////////////////////////////
SecureConnectionServicesPtr SecureConnectionServicesAPI::getPlugin()
{
    SecureConnectionServicesPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}



// Read/Write property testString
std::string SecureConnectionServicesAPI::get_testString()
{
    return m_testString;
}
void SecureConnectionServicesAPI::set_testString(const std::string& val)
{
    m_testString = val;
}

// Read-only property version
std::string SecureConnectionServicesAPI::get_version()
{
    return "CURRENT_VERSION";
}

// Method echo
FB::variant SecureConnectionServicesAPI::echo(const FB::variant& msg)
{
    return msg;
}

void SecureConnectionServicesAPI::testEvent(const FB::variant& var)
{
    FireEvent("onfired", FB::variant_list_of(var)(true)(1));
}

