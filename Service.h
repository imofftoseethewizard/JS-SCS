/**********************************************************\

  Service.h

\**********************************************************/

#include <string.h>

#include "JSAPIAuto.h"

#include "SecureConnection.h"

#ifndef H_Service
#define H_Service

class Service : public FB::JSAPIAuto
{
 public:
  Service();
  Service(SecureConnectionPtr connection,
	  const std::string& scheme,
	  const std::string& configText);

  virtual ~Service();

  FB::JSAPIPtr get_connection() const;
  std::string get_scheme() const;

  virtual void revoke();

 protected:
  std::string m_scheme;
  std::string m_configText;
  SecureConnectionWeakPtr m_connection;
  
 private:
};

typedef ServicePtr *(ServiceFactory)(SecureConnectionPtr connection,
				     const std::string& scheme,
				     const std::string& configText);

#endif // H_Service
