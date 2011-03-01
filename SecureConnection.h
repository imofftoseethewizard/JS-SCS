/******************************************************************************

  SecureConnection.h

  SecureConnection wraps an SSL connection with a remote host. It reads the
  configuration information below ~/.jshs/config on the remote host and
  offers only those services which are configured. It checks that the url of
  the foreign host is allowed by the same origin policy. It ensures that
  SSL sessions are closed and freed when this object is reclaimed.

  ---------------------------------------------------------------------------

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

  ---------------------------------------------------------------------------

  Copyright 2011 Pat M. Lasswell.

 ******************************************************************************/


#ifndef H_SecureConnection
#define H_SecureConnection

#include <string>

#include <libssh2.h>
#include <libssh2_sftp.h>

#include <boost/weak_ptr.hpp>

#include "JSAPIAuto.h"
#include "HostServices.h"


FB_FORWARD_PTR(Service);
FB_FORWARD_PTR(SecureConnection);

class SecureConnection : public FB::JSAPIAuto
{
 public:
  friend class FileService;

  SecureConnection(HostServicesPtr plugin,
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

  void requestServiceByScheme(const std::string& scheme);
  void releaseService(ServicePtr service);

  void credentialsRequestFilled(const std::string& password);
  void credentialsRequestDenied();

  void closeConnection();

 protected:
  void setReadyState(ReadyState state);
  void reportError(const FB::script_error& e);

  void open();
  void completeOpen();

  bool isOriginAllowed();
  void createSocket();
  void startSession();
  void authenticate();
  void getServiceSchemes();

  void openSftpChannel();
  void grantService(ServicePtr service);
  void revokeAllServices();


 private:
  HostServicesWeakPtr m_hs;
    
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

// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
