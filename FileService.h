/**********************************************************\

  FileService.h

  FileService exposes much of the functionality of sftp.

  var fs;
  remote.requestServiceByScheme("file")
        .exec(function(service) {
                fs = service;
                // do something
              });

  ...
  
  fs.get("./emacs.d/init.el")
    .exec(function (file) {
      
    });
  function (file) {
  }.
  fs.get("./emacs.d/init.el", function(file) { init_el = file });
  
interface FileService : Service {
  readonly attribute SecureConnection connection;

  AsyncCommand<>                 create(in FileSystemPath path);
  AsyncCommand<>                 delete(in FileSystemPath path);
  AsyncCommand<>                 copy(in FileSystemPath source, in FileSystemPath destination);
  AsyncCommand<>                 rename(in FileSystemPath source, in FileSystemPath destination);
  AsyncCommand<FileSystemObject> get(in FileSystemPath path);
  AsyncCommand<>                 put(in FileSystemPath path, in File file);

  AsyncCommand<boolean>          exists(in FileSystemPath path);
  AsyncCommand<boolean>          isFile(in FileSystemPath path);
  AsyncCommand<boolean>          isDirectory(in FileSystemPath path);

  AsyncCommand<FileSystemPath>   getAbsolutePath(in FileSystemPath path);
  AsyncCommand<FileSystemPath>   createTempName(in optional DOMString prefix);
};

FileService implements EventTarget;

FileSystemObject
  name
  type
  path
  owner
  group
  permissions
  FileSystemObjectContents

FileSystemObjectContents

FileContents
  contents

DirectoryContents
  contained file names

FileService implements Service;

\**********************************************************/

#include <string>

#include "JSAPIAuto.h"

#include "SecureConnection.h"
#include "Service.h"


#ifndef H_FileService
#define H_FileService

FB_FORWARD_PTR(FileService)

class FileServiceGetCommand : public FB::JSAPIAuto
{
public:
    FileServiceGetCommand(const FileServicePtr& service, const std::string& path);
    void exec(const FB::JSObjectPtr& callback);

protected:
    void report(const std::string& event, FB::VariantList args);
    void reportResult(FB::VariantList args);
    void reportError(const FB::script_error& e) ;

private:
    FileServicePtr m_service;
    std::string m_path;
};


class FileService : public Service
{
    friend class FileServiceGetCommand;

public:
    FileService(SecureConnectionPtr connection,
		const std::string& scheme,
		const std::string& config);

    virtual ~FileService();

    virtual void start();
    virtual void revoke();

    static boost::shared_ptr<FileService> create(SecureConnectionPtr connection,
						 const std::string& scheme,
						 const std::string& config);

    FB::JSAPIPtr get(const std::string &path);


protected:
    void reportError(const FB::script_error& e);

private:
    LIBSSH2_SFTP *m_sftp;
};

#endif // H_FileService
