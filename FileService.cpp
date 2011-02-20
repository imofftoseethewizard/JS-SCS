/**********************************************************\

  FileService.cpp

\**********************************************************/


#include "variant_list.h"

#include "FileService.h"
#include "AsyncCommand.h"


#define BUFFER_SIZE 4096

FileService::FileService(SecureConnectionPtr connection,
			 const std::string& scheme,
			 const std::string& config)
  : Service(connection, scheme, config),
    m_sftp(NULL)
{
  registerMethod("get", make_method(this, &FileService::get));
  registerEvent("onresult");
  registerEvent("onerror");
}


FileService::~FileService()
{
  revoke();
}

void FileService::start()
{
  if (!m_sftp && !(m_sftp = libssh2_sftp_init(m_connection.lock()->getSession())))
    throw FB::script_error("Unable to initialize SFTP channel.");
}


void FileService::revoke()
{
  if (m_sftp)
  {
    libssh2_sftp_shutdown(m_sftp);
    m_sftp = NULL;
  }
}


FileServicePtr FileService::create(SecureConnectionPtr connection,
				   const std::string& scheme,
				   const std::string& config)
{
  FileServicePtr fs = boost::make_shared<FileService>(connection, scheme, config);
  fs->start();
  return fs;
}


void FileService::reportError(const FB::script_error& e) 
{
  FireEvent("onerror", FB::variant_list_of(shared_from_this())(e.what()));
}


FB::JSAPIPtr FileService::get(const std::string& path)
{
  return boost::make_shared<FileServiceGetCommand>(FB::ptr_cast<FileService>(shared_from_this()), path);
}



FileServiceGetCommand::FileServiceGetCommand(const FileServicePtr& service, const std::string& path)
  : m_service(service),
    m_path(path)
{
  registerMethod("exec", make_method(this, &FileServiceGetCommand::exec));
  registerEvent("onresult");
  registerEvent("onerror");
}


void FileServiceGetCommand::report(const std::string& event, FB::VariantList args)
{
  FireEvent(event, args);
  args.insert(args.begin(), shared_from_this());
  m_service->FireEvent(event, args);
}


void FileServiceGetCommand::reportResult(FB::VariantList args)
{
  report("onresult", args);
}


void FileServiceGetCommand::reportError(const FB::script_error& e) 
{
  report("onerror", FB::variant_list_of(shared_from_this())(e.what()));
}



void FileServiceGetCommand::exec(const FB::JSObjectPtr& callback)
{
  FBLOG_INFO("FileServiceGetCommand::exec", m_path);
  LIBSSH2_SFTP_HANDLE *file = NULL;
  
  try
  {
    if (!(file = libssh2_sftp_open(m_service->m_sftp,
				   m_path.c_str(), LIBSSH2_FXF_READ, 0)))
      throw FB::script_error("File not found.");

    //for now, presume contents are ascii.
    std::stringstream contents;

    int rc;
    char buffer[BUFFER_SIZE];
    while ((rc = libssh2_sftp_read(file, buffer, sizeof(buffer)-1)) > 0)
    {
      buffer[rc] = 0;
      contents << buffer;
    }

    if (rc < 0) 
      // TODO -- get error msg for errno
      throw FB::script_error("Error while reading file.");

    libssh2_sftp_close(file);
    file = NULL;
      
    reportResult(FB::variant_list_of(callback->Invoke("", FB::variant_list_of(contents.str()))));
  }
  catch (FB::script_error e) 
  {
    if (file)
      libssh2_sftp_close(file);

    reportError(e);
  }
}
