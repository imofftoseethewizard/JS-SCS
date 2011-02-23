/**********************************************************\

  SecureConnection.cpp

\**********************************************************/

#define DIR_BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 4096
#define CONFIG_DIR ".jsscs/config"

#include <cstdio>
#include <string>

#include <libssh2.h>
#include <libssh2_sftp.h>

#include <sys/stat.h>

/*
#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
*/
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <boost/bind.hpp>
#include <boost/functional.hpp>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

#include "variant_list.h"
#include "JSAPIAuto.h"
#include "JSExceptions.h"


#include "SecureConnectionServices.h"
#include "SecureConnection.h"
#include "Service.h"

// TODO -- registration of components, then remove this
#include "FileService.h"

/*****************************************************************************

  IDL:
    interface SecureConnection {
      readonly attribute DOMString user;
      writeonly attribute DOMString password;
      readonly attribute DOMString host;
      readonly attribute unsigned short port;

      // ready state
      const unsigned short NEW = 0;
      const unsigned short CONNECTING = 1;
      const unsigned short OPEN = 2;
      const unsigned short CLOSING = 3;
      const unsigned short CLOSED = 4;
      readonly attribute unsigned short readyState;

      readonly attribute ServiceCollection services;

      // callbacks
	       attribute Function onreadystatechange;
	       attribute Function onrequest;
	       attribute Function ongrant;
	       attribute Function onerror;

      void open();
 
      Service requestServiceByName(in DOMString name);
      void releaseService(in Service service);

      void close();
    };

 In ~/.jsscs/policies.

 *****************************************************************************/

SecureConnection::SecureConnection(SecureConnectionServicesPtr scs,
				   const std::string& user,
				   const std::string& hostName,
				   unsigned int port) 
  : m_scs(scs),
    m_user(user),
    m_hostName(hostName),
    m_port(port),
    m_readyState(SecureConnection::NEW),
    m_sock(-1),
    m_session(NULL),
    m_sftp(NULL)
{
  registerProperty("user", make_property(this, &SecureConnection::get_user));
  registerProperty("hostName", make_property(this, &SecureConnection::get_hostName));
  registerProperty("port", make_property(this, &SecureConnection::get_port));

  registerAttribute("NEW",        static_cast<int>(NEW),        true);
  registerAttribute("CONNECTING", static_cast<int>(CONNECTING), true);
  registerAttribute("OPEN",       static_cast<int>(OPEN),       true);
  registerAttribute("CLOSING",    static_cast<int>(CLOSING),    true);
  registerAttribute("CLOSED",     static_cast<int>(CLOSED),     true);

  registerProperty("readyState", make_property(this, &SecureConnection::get_readyState));

  registerProperty("password", make_property(this,
					     &SecureConnection::get_password,
					     &SecureConnection::set_password));

  registerProperty("onreadystatechange", make_property(this,
						       &SecureConnection::get_onreadystatechange,
						       &SecureConnection::set_onreadystatechange));

  registerProperty("onrequest",          make_property(this,
						       &SecureConnection::get_onrequest,
						       &SecureConnection::set_onrequest));

  registerProperty("ongrant",            make_property(this,
						       &SecureConnection::get_ongrant,
						       &SecureConnection::set_ongrant));

  registerProperty("onerror",            make_property(this,
						       &SecureConnection::get_onerror,
						       &SecureConnection::set_onerror));

  registerProperty("servicesOffered", make_property(this, &SecureConnection::get_serviceSchemes));
  registerProperty("services", make_property(this, &SecureConnection::get_services));

  registerMethod("open", make_method(this, &SecureConnection::openAsync));
  
  registerMethod("requestServiceByScheme", make_method(this, &SecureConnection::requestServiceByScheme));
  registerMethod("releaseService", make_method(this, &SecureConnection::releaseService));

  registerMethod("close", make_method(this, &SecureConnection::closeConnection));

  registerEvent("onreadystatechange");
  registerEvent("onrequest");
  registerEvent("ongrant");
  registerEvent("onerror");

  registerAttribute("PID", getpid(), true);
}


SecureConnection::~SecureConnection()
{
  closeConnection();
}


LIBSSH2_SESSION *SecureConnection::getSession() const
{
  return m_session;
}


std::string SecureConnection::get_user() const
{
  return m_user;
}


std::string SecureConnection::get_password() const
{
  return "";
}


void SecureConnection::set_password(const std::string& password)
{
  m_password = password;
}


std::string SecureConnection::get_hostName() const
{
  return m_hostName;
}


unsigned int SecureConnection::get_port() const
{
  return m_port;
}


int SecureConnection::get_readyState() const
{
  return m_readyState;
}


FB::JSObjectPtr SecureConnection::get_onreadystatechange() const
{
  return m_onreadystatechange;
}


FB::JSObjectPtr SecureConnection::get_onrequest() const
{
  return m_onrequest;
}


FB::JSObjectPtr SecureConnection::get_ongrant() const
{
  return m_ongrant;
}


FB::JSObjectPtr SecureConnection::get_onerror() const
{
  return m_onerror;
}


void SecureConnection::set_onreadystatechange(const FB::JSObjectPtr& callback)
{
  m_onreadystatechange = callback;
}


void SecureConnection::set_onrequest(const FB::JSObjectPtr& callback)
{
  m_onrequest = callback;
}


void SecureConnection::set_ongrant(const FB::JSObjectPtr& callback)
{
  m_ongrant = callback;
}


void SecureConnection::set_onerror(const FB::JSObjectPtr& callback)
{
  m_onerror = callback;
}


FB::VariantList SecureConnection::get_serviceSchemes() const
{
  return FB::make_variant_list(m_serviceSchemes);
}


FB::VariantList SecureConnection::get_services() const
{
  
  return FB::make_variant_list(m_services);
}


void SecureConnection::openAsync()
{
  boost::thread(boost::bind(&SecureConnection::open, ref(this)));
}

void SecureConnection::open()
{
  if (m_readyState != NEW && m_readyState != CLOSED) 
    reportError(FB::script_error("SecureConnection is already open."));

  else if (!isOriginAllowed())
    reportError(FB::script_error("Host access would violate same origin policy."));

  else
  {
    try 
    {
      setReadyState(CONNECTING);

      createSocket();
      startSession();
      authenticate();
      getServiceSchemes();

      setReadyState(OPEN);
    }
    catch (FB::script_error e)
    {
      closeConnection();
      reportError(e);
    }
  }
}

bool predicate(const std::string& s1, const std::string& s2)
{
  return !s1.compare(s2);
}


void SecureConnection::requestServiceByScheme(const std::string& scheme)
{
  if (m_readyState != OPEN)
    throw FB::script_error("Connection is not open.");

  LIBSSH2_SFTP_HANDLE *config = NULL;
  std::stringstream config_text;

  try {
    if (m_serviceSchemes.size() == 0)
      getServiceSchemes();

    if (m_serviceSchemes.end() == std::find_if(m_serviceSchemes.begin(),
					       m_serviceSchemes.end(),
					       boost::bind(predicate, scheme, _1)))
      throw FB::script_error("Service not available.");

    std::string filename(CONFIG_DIR);
    filename.append("/");
    filename.append(scheme);

    if (!(config = libssh2_sftp_open(m_sftp, filename.c_str(), LIBSSH2_FXF_READ, 0)))
      throw FB::script_error("Unable to open service policy.");

    char buffer[FILE_BUFFER_SIZE];
    int rc;
    while ((rc = libssh2_sftp_read(config, buffer, sizeof(buffer)-1)) > 0)
    {
      buffer[rc] = 0;
      config_text << buffer;
    }

    if (rc < 0)
      throw FB::script_error("Unable to read service policy.");

    libssh2_sftp_close(config);
    config = NULL;

  }
  catch (FB::script_error e) 
  {
    if (config)
      libssh2_sftp_close(config);
    reportError(e);
  }
  // TODO scheme->constructor registry
  grantService(FileService::create(FB::ptr_cast<SecureConnection>(shared_from_this()),
				   scheme, config_text.str()));
}


void SecureConnection::grantService(ServicePtr service)
{
  m_services.push_back(service);
  FireEvent("ongrant", FB::variant_list_of(shared_from_this())(service));
}


void SecureConnection::releaseService(ServicePtr service)
{
  // check that services is in m_services, remove
  service->revoke();
}


void SecureConnection::revokeAllServices()
{
  while (m_services.size() > 0) {
    m_services.back()->revoke();
    m_services.pop_back();
  }
}


void SecureConnection::closeConnection()
{
  setReadyState(CLOSING);
  
  revokeAllServices();

  if (m_sftp)
  {
    libssh2_sftp_shutdown(m_sftp);
    m_sftp = NULL;
  }

  if (m_session)
  {
    libssh2_session_disconnect(m_session, "Finished.");
    libssh2_session_free(m_session);
    m_session = NULL;
  }

  if (m_sock != -1)
  {
#ifdef WIN32
    closesocket(m_sock);
#else
    close(m_sock);
#endif
    m_sock = -1;
  }

  setReadyState(CLOSED);
}


void SecureConnection::setReadyState(SecureConnection::ReadyState state)
{
  if (m_readyState != state)
  {
    m_readyState = state;
    FireEvent("onreadystatechange", FB::variant_list_of(shared_from_this())(m_readyState));
  }
}


void SecureConnection::reportError(const FB::script_error& e) 
{
  FireEvent("onerror", FB::variant_list_of(shared_from_this())(e.what()));
}


bool SecureConnection::isOriginAllowed()
{
  // TODO implement same-origin policy for hostName
  // use either window.location.origin, or read the permissions
  // from the manifest for an app.
  return true;
}

void SecureConnection::createSocket()
{
  struct addrinfo hints;
  struct addrinfo *result, *p;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; 
  hints.ai_flags = 0;
  hints.ai_protocol = 0;          /* Any protocol */

  std::stringstream port;
  port << m_port;
  
  int rc;
  if ((rc = getaddrinfo(m_hostName.c_str(), port.str().c_str(), &hints, &result)) != 0)
  {
    std::stringstream msg;
    msg << "Cannot resolve remote host: " << gai_strerror(rc);

    throw FB::script_error(msg.str());
  }

  for (p = result, m_sock = -1; p != NULL && m_sock == -1; p = p->ai_next)
  {
    if ((m_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
      continue;

    if (connect(m_sock, p->ai_addr, p->ai_addrlen) != 0)
    {
      close(m_sock); 
      m_sock = -1;
      continue;
    }
  }

  if (m_sock == -1)
    throw FB::script_error("Unable to connect to remote host.");
}


void SecureConnection::startSession()
{
  m_session = libssh2_session_init();
  if (!m_session)
    throw FB::script_error("Cannot initialize secure session.");

  int rc;
  if ((rc = libssh2_session_startup(m_session, m_sock)))
  { 
    std::stringstream msg;
    msg << "Cannot start session with host: return code: " << rc;

    throw FB::script_error(msg.str());
  }
}

/*
void SecureConnection::checkKnownHost(boost::optional<const FB::JSObjectPtr&> callbackAdviseUnknownHost)
{
  // unable to open known hosts file at all, even with fopen.
  // all attempts return EACCES, yet I can find no reason why this should be.
  // So, for now, this is disabled.
  return;

  LIBSSH2_KNOWNHOSTS *hosts = libssh2_knownhost_init(m_session);

  if(!hosts)
    throw FB::script_error("Unable to access known hosts list.");

  try
  {
    std::string hosts_file(getenv("HOME"));
    hosts_file.append("/.ssh/known_hosts");

    int rc;
    if (0 > (rc = libssh2_knownhost_readfile(hosts, hosts_file.c_str(), LIBSSH2_KNOWNHOST_FILE_OPENSSH)))
    {
      std::stringstream msg;
      msg << "Unable to read known hosts file [" << hosts_file.c_str() << "]: " << rc;

      throw FB::script_error(msg.str());
    }
      
    size_t len;
    int type;
    const char *key = libssh2_session_hostkey(m_session, &len, &type);
    if (!key) 
    {
      libssh2_knownhost_free(hosts);
      throw FB::script_error("Unable to obtain host key.");
    }
    
    int check = libssh2_knownhost_check(hosts, m_hostName.c_str(), key, len,
					LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW,
					NULL);

    if (check != LIBSSH2_KNOWNHOST_CHECK_MATCH)
    {
      // TODO check policy to see that unknown hosts may be added.
      HostCheckResult failure;
      switch (check)
      {
      case LIBSSH2_KNOWNHOST_CHECK_FAILURE:
	failure = KNOWN_HOST_CHECK_FAILURE;
	break;

      case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND:
	failure = UNKNOWN_HOST;
	break;

      case LIBSSH2_KNOWNHOST_CHECK_MISMATCH:
	failure = KNOWN_HOST_KEY_MISMATCH;
	break;

      default:
	throw FB::script_error("Unknown failure mode in known host check.");
      }

      if (!callbackAdviseUnknownHost)
	throw FB::script_error("Unknown host; not added to known hosts.");

      FB::variant result = callbackAdviseUnknownHost.get()->Invoke("", FB::variant_list_of(failure));
      if (!result.convert_cast<bool>())
	throw FB::script_error("Unknown host; not added to known hosts.");
      
      int typemask = LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW;
      
      if (type == LIBSSH2_HOSTKEY_TYPE_RSA)
	typemask |= LIBSSH2_KNOWNHOST_KEY_SSHRSA;

      else if (type == LIBSSH2_HOSTKEY_TYPE_DSS)
	typemask |= LIBSSH2_KNOWNHOST_KEY_SSHDSS;

      struct libssh2_knownhost *store;
      if ((rc = libssh2_knownhost_add(hosts, m_hostName.c_str(), 0, (char *)key, len,
				      typemask, &store)))
      {
	std::stringstream msg;
	msg << "Failed to add unknown host to known hosts: return code: " << rc;

	throw FB::script_error(msg.str());
      }
      
      if ((rc = libssh2_knownhost_writefile(hosts, hosts_file.c_str(), LIBSSH2_KNOWNHOST_FILE_OPENSSH)))
      {
	std::stringstream msg;
	msg << "Failed to write updated known hosts file: return code: " << rc;

	throw FB::script_error(msg.str());
      }
    }

    libssh2_knownhost_free(hosts);
  }
  catch (FB::script_error e)
  {
    libssh2_knownhost_free(hosts);
    throw e;
  }
  catch (std::exception e)
  {
    libssh2_knownhost_free(hosts);
    throw e;
  }
}

*/
void SecureConnection::authenticate()
{
  // TODO default UI when requestCredentials is null
  if (m_password.length() == 0)
    throw FB::script_error("No password supplied.");

  int rc;
  if ((rc = libssh2_userauth_password(m_session, m_user.c_str(), m_password.c_str())))
    throw FB::script_error("Invalid username or password");

  m_password = "";
}

void SecureConnection::openSftpChannel()
{
  if (!m_sftp && !(m_sftp = libssh2_sftp_init(m_session)))
    throw FB::script_error("Unable to initialize SFTP channel.");
}


void SecureConnection::getServiceSchemes()
{
  openSftpChannel();

  LIBSSH2_SFTP_HANDLE *config_dir = NULL;

  try
  {
    if ((config_dir = libssh2_sftp_opendir(m_sftp, CONFIG_DIR)))
    {
      int rc;
      char buffer[DIR_BUFFER_SIZE];
      LIBSSH2_SFTP_ATTRIBUTES attrs;
      while ((rc = libssh2_sftp_readdir(config_dir, buffer, sizeof(buffer), &attrs)) > 0)
	if (strncmp(buffer, ".", rc) && strncmp(buffer, "..", rc))
	  m_serviceSchemes.push_back(std::string(buffer));

      if (rc < 0) 
	throw FB::script_error("Error while reading services' policies.");

      libssh2_sftp_closedir(config_dir);
      config_dir = NULL;
    }
  }
  catch (FB::script_error e) 
  {
    if (config_dir)
      libssh2_sftp_closedir(config_dir);

    throw e;
  }
}
