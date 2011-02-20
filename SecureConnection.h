/**********************************************************\

  SecureConnection.h

\**********************************************************/

#include <string>

#include <libssh2.h>
#include <libssh2_sftp.h>

#include <boost/weak_ptr.hpp>

#include "JSAPIAuto.h"
#include "SecureConnectionServices.h"

#ifndef H_SecureConnection
#define H_SecureConnection

FB_FORWARD_PTR(Service);
FB_FORWARD_PTR(SecureConnection);

class SecureConnection : public FB::JSAPIAuto
{
 public:
  SecureConnection(SecureConnectionServicesPtr plugin,
		   const std::string& user,
		   const std::string& hostName,
		   unsigned int port);

  virtual ~SecureConnection();

  LIBSSH2_SESSION *getSession() const;

  std::string get_user() const;
  std::string get_password() const;
  void set_password(const std::string& password);

  std::string get_hostName() const;
  unsigned int get_port() const;

  enum ReadyState {
    NEW,
    CONNECTING,
    OPEN,
    CLOSING,
    CLOSED
  };

  int get_readyState() const;

  FB::JSObjectPtr get_onreadystatechange() const;
  FB::JSObjectPtr get_onrequest() const;
  FB::JSObjectPtr get_ongrant() const;
  FB::JSObjectPtr get_onerror() const;

  void set_onreadystatechange(const FB::JSObjectPtr& callback);
  void set_onrequest(const FB::JSObjectPtr& callback);
  void set_ongrant(const FB::JSObjectPtr& callback);
  void set_onerror(const FB::JSObjectPtr& callback);

  FB::VariantList get_services() const;
  FB::VariantList get_serviceSchemes() const;

  enum HostCheckResult {
    KNOWN_HOST_CHECK_FAILURE = 1,
    UNKNOWN_HOST,
    KNOWN_HOST_KEY_MISMATCH
  };

  void openAsync();

  void requestServiceByScheme(const std::string& scheme);
  void releaseService(ServicePtr service);

  void closeConnection();

 protected:
  void setReadyState(ReadyState state);
  void reportError(const FB::script_error& e);

  void open();

  bool isOriginAllowed();
  void createSocket();
  void startSession();
  void authenticate();
  void getServiceSchemes();

  void openSftpChannel();
  void grantService(ServicePtr service);
  void revokeAllServices();

  //  void checkKnownHost(boost::optional<const FB::JSObjectPtr&> callbackAdviseUnknownHost);

 private:
  SecureConnectionServicesWeakPtr m_scs;
    
  std::string m_user;
  std::string m_password;
  std::string m_hostName;
  unsigned int m_port;

  std::vector<std::string> m_serviceSchemes;
  std::vector<ServicePtr> m_services;

  int m_readyState;

  FB::JSObjectPtr m_onreadystatechange;
  FB::JSObjectPtr m_onrequest;
  FB::JSObjectPtr m_ongrant;
  FB::JSObjectPtr m_onerror;

  int m_sock;
  LIBSSH2_SESSION *m_session;
  LIBSSH2_SFTP *m_sftp;

};

#endif // H_SecureConnection

