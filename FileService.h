/******************************************************************************

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

  FileServiceCreateCommand create(in FileSystemPath path);
  FileServiceDeleteCommand delete(in FileSystemPath path);
  FileServiceCopyCommand copy(in FileSystemPath source, in FileSystemPath destination);
  FileServiceRenameCommand rename(in FileSystemPath source, in FileSystemPath destination);
  FileServiceGetCommand get(in FileSystemPath path);
  FileServicePutCommand put(in FileSystemPath path, in File file);

  FileServiceExistsCommand exists(in FileSystemPath path);
  FileServiceIsFileCommand isFile(in FileSystemPath path);
  FileServiceIsDirectoryCommand isDirectory(in FileSystemPath path);

  FileServiceGetAbsolutePathCommand getAbsolutePath(in FileSystemPath path);
  FileServiceCreateTempNameCommand createTempName(in optional DOMString prefix);
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

Configuration File

FileService's configuration file by default is found at  ~/.jsscc/config/path.

Sections
[noaccess]
[readonly]
[writeonly]
[readwrite]

A line beginning with a '+', a '-', or whitespace /^\b*(-|+)/ is an access
specifier. What follows the '+' or '-' is a path. The initial character
indicates whether the file or subtree indicated by path is included or 
excluded, respectively, from the rights of the including section. If the
line begins with whitespace, then the path is considered relative to the
path of the prior line.  (It must be true both that the prior line is
an access specifier, and that the path in the present is relative.)  E.g,

~/.jsscs/config/file:
[readonly]
+~

[noaccess]
+~/.ssh
+~/.jsscs
+~/.../*~

[readwrite]
+~/src/
 -.../.git/...
 -old

The above specifies first that all files are readonly.  The noaccess section
removes the .ssh and .jsscs subtrees, so that they may neither be read
nor written to.  Also, any file whose last component ends with ~ is excluded.
Finally, read and write is allowed in the src subtree, though that is qualified
to exclude any path that includes a .git component.

Globbing is allowed and extended.  Additionally, FileService interprets '...' to
mean any number of path components. 

Parsing:
1 separate sections
  look for '[' in column 0

2 parse lines ^(\b*)(+|-)(.*)
  boost::regexp

3 parse paths 
  lookup shell glob rules
    ? matches a single character
    * matches any number of characters
    [ ... ] are like regexp character classes
    { , , } specify alternatives
    ... specifies zero or more path components

  backslash quotes

  expand ~
    get $HOME environment variable
      open channel
      command "echo $HOME"
      read result

5 store as a sequence of trees, tagged by section
  boost or stl should have a tree struct
  presume each starts with '-/'

Application

If a specifier begins with blank space, then the path component is relative to 
the prior specifer with one less leading blank.  It is an error for there not to
be one.  Blanks may be either spaces or tabs, but each counts as one, so files
(for readability) should use either tabs or spaces, but not both.

Paths supplied must be resolvable to an absolute path.  (In unix-based OSs, this
means that they must start with either '/' or '~'.)

A path matches an access-specifier if the glob-expanded specifier matches the
path, or if it matches one of the path's supertrees.

A line matches a section if the last matching specifier starts with a '+'.

 ******************************************************************************/

#include <string>

#include <boost/regex.hpp>

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
    void parseConfig();

    class ConfigLine
    {
    public:
	ConfigLine(const std::string& line);
	
	typedef enum { PLUS, MINUS } Sign;
	typedef enum { BLANK, SECTION, SPECIFIER, SECTION_ERROR, SPECIFIER_ERROR, MISC_ERROR } Type;
	typedef enum { NOACCESS, READONLY, WRITEONLY, READWRITE, OVERRIDE } ConfigSection;
	static std::map<std::string, ConfigSection, std::greater<std::string> > SectionNames;

	typedef struct
	{
	    Sign sign;
	    int indent;
	    std::string path;
	    boost::regex re_glob;
	} Specifier;

	typedef boost::shared_ptr<ConfigLine> Ptr;
	typedef boost::weak_ptr<ConfigLine> WeakPtr;

	inline Type getType() { return m_type; };
	inline Sign getSign() { return m_specifier.sign; };
	inline int getIndent() { return m_specifier.indent; };

	inline std::string getPath() { return m_specifier.path; };
	inline void setPath(const std::string &path) { m_specifier.path = path; };
	inline boost::regex getGlobRegex() { return m_specifier.re_glob; };
	inline void setGlobRegex(const boost::regex& re_glob) { m_specifier.re_glob = re_glob; };

    protected:
	static boost::regex re_blank;
	static boost::regex re_section;
	static boost::regex re_specifier;

	// Used to give more helpful error messages
	static boost::regex re_crude_section;
	static boost::regex re_crude_specifier;

    private:
	Type m_type;
	ConfigSection m_section;
	Specifier m_specifier;
    };
    
    static boost::regex re_glob_transform;

    void reportError(const FB::script_error& e);

private:

    LIBSSH2_SFTP *m_sftp; // channel for file transfer
    std::string m_home; // connection's user's home directory on remote host.

    bool m_disabled;
    std::vector<ConfigLine> m_config;
};

#endif // H_FileService
