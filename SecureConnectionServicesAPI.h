/**********************************************************\

  Auto-generated SecureConnectionServicesAPI.h

\**********************************************************/

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "SecureConnectionServices.h"

#ifndef H_SecureConnectionServicesAPI
#define H_SecureConnectionServicesAPI

class SecureConnectionServicesAPI : public FB::JSAPIAuto
{
public:
    SecureConnectionServicesAPI(const SecureConnectionServicesPtr& plugin, const FB::BrowserHostPtr& host);
    virtual ~SecureConnectionServicesAPI();

    SecureConnectionServicesPtr getPlugin();

    // Read/Write property ${PROPERTY.ident}
    std::string get_testString();
    void set_testString(const std::string& val);

    // Read-only property ${PROPERTY.ident}
    std::string get_version();

    // Method echo
    FB::variant echo(const FB::variant& msg);
    
    // Method test-event
    void testEvent(const FB::variant& s);

private:
    SecureConnectionServicesWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

    std::string m_testString;
};

#endif // H_SecureConnectionServicesAPI

