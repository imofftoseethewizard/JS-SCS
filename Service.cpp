/**********************************************************\

  Service.cpp

\**********************************************************/

#include "Service.h"


Service::Service(SecureConnectionPtr connection,
		 const std::string& scheme,
		 const std::string& configText)
  : m_connection(connection),
    m_scheme(scheme),
    m_configText(configText)
{
  registerAttribute("connection", connection, true);
  registerAttribute("scheme",     scheme, true);
}


Service::~Service()
{
}

FB::JSAPIPtr Service::get_connection() const
{
  return boost::shared_ptr<SecureConnection>(m_connection);
}

std::string Service::get_scheme() const
{
  return m_scheme;
}


void Service::revoke() 
{
}
