/**********************************************************\

  Auto-generated SecureConnectionServices.h

\**********************************************************/

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

  // Read/Write property ${PROPERTY.ident}
  std::string get_testString();
  void set_testString(const std::string& val);

  // Read-only property ${PROPERTY.ident}
  std::string get_version();

  // Method echo
  FB::variant echo(const FB::variant& msg);
    
  // Method test-event
  void testEvent(const FB::variant& s);

  FB::JSAPIPtr createSecureConnection(const std::string& user,
				      const std::string& hostName,
				      boost::optional<unsigned int> port);
 private:
  SecureConnectionServicesPluginWeakPtr m_plugin;
  FB::BrowserHostPtr m_host;

  std::string m_testString;
};

#endif // H_SecureConnectionServices

