/******************************************************************************

  FileService.cpp

  Config File

  The config file for FileService is located at .jsscs/config/file.

  Format

  The config file is divided into sections by section headers enclosed in
  brackets. Each section header must be on a line by itself. Whitespace can be
  inserted around the section name or before and after the brackets. Only the
  following sections are valid, all others will be an error: noaccess,
  readonly, writeonly, readwrite, and override.

  Each section can contain one or more specifiers. A specifier begins with an
  optional number of indentation characters, either spaces or tabs. This is
  followed by either a + or a -, optional whitespace, the path the specifier
  refers to, and then optional whitespace before the newline.

  Indentation is significant and is counted as the number of characters, so in
  the interest of sanity for those who read the file, please use either all
  spaces or all tabs, as to mix them would be horribly confusing. Each
  specifier may have an indentation level between 0 and one more than the line
  above it. Greater jumps in indentation are not allowed.

  The path a specifier contains is required to be absolute if the specifier
  has no leading indentation; conversely if the specifier is indented, then it
  must be a relative path.

  Specifiers are arranged into a tree in the natural sense, according to their
  relative indentation levels and vertical positions in the file. Child
  specifiers refine parents, and the last sibling dominates.

  A plus at the start of a specifier indicates that the named file or subtree
  should be added to the set paths possessing the permissions implied by the
  containing section; a minus indicates that the named file or subtree should
  be unaffected. Thus

    [readwrite]
    - /usr/src

  does not mean that /usr/src has neither read nor write permission, but that
  this section does not alter the permissions on /usr/src.  

  [noaccess]
  
  Paths matching the specifiers following a noaccess section heading will be
  available for neither reading nor writing.

  [readonly]

  Paths matching the specifiers following a readonly section heading will be
  available for reading but not writing.

  [writeonly]

  Paths matching the specifiers following a writeonly section heading will be
  available for writing but not reading.

  [readwrite]

  Paths matching the specifiers following a readwrite section heading will be
  available for both writing and reading.

  [override]

  An override section provides a means to create holes in the FileService's
  security architecture.  By default, the access specification in the config
  file is augmented with 

    [noaccess]
    + ~/.*

  since a lot of utilities store configuration information in directories
  named '.*' in the users home directory.  Many of these store sensitive
  information that could be used to upgrade permissions.  To make it easy
  to keep this hidden, by default FileService provides no access. It may,
  however, be useful on a carefully considered basis to provide access to 
  some items in the .* directories. Specifiers following an override section
  header are interpreted as if they were children of the single specifier
  in the default [noaccess] section.  

  Example:

    [readonly]
    + ~

    [override]
    + ~/.jsscs/config/file

  This file is actually interpreted as

    [readonly]
    + ~

    [noaccess]
    + ~/.*
    - ~/.jsscs/config/file

  Multiple [override] sections are appended in the order they appear.

 ******************************************************************************/


#include <map>
#include <errno.h>

#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

#include "variant_list.h"

#include "FileService.h"
#include "AsyncCommand.h"

// Used by get, command execution
#define BUFFER_SIZE 4096



/*-----------------------------------------------------------------------------*

  FileService::FileService

 *-----------------------------------------------------------------------------*/

FileService::FileService(SecureConnectionPtr connection,
			 const std::string& scheme,
			 const std::string& configText)
  : Service(connection, scheme, configText),
    m_sftp(NULL),
    m_home("")
{
  registerMethod("get", make_method(this, &FileService::get));
  registerEvent("onresult");
  registerEvent("onerror");
}


/*-----------------------------------------------------------------------------*

  FileService::~FileService

 *-----------------------------------------------------------------------------*/

FileService::~FileService()
{
  revoke();
}


/*-----------------------------------------------------------------------------*

  FileService::start

  TODO -- refactor command execution out to its own method.

 *-----------------------------------------------------------------------------*/

void FileService::start()
{
  LIBSSH2_SESSION *session = m_connection.lock()->getSession();

  if (!m_sftp && !(m_sftp = libssh2_sftp_init(session)))
    throw FB::script_error("Unable to initialize SFTP channel.");

  LIBSSH2_CHANNEL *channel; // channel for command execution

  if (!(channel = libssh2_channel_open_session(session)))
    throw FB::script_error("Unable to open command channel to remote host.");

  try
  {
    int rc;
    if ((rc = libssh2_channel_exec(channel, "echo $HOME")) != 0)
      throw FB::script_error("Unable to execute command on remote host.");

    char buffer[BUFFER_SIZE];
    std::stringstream result_text;
    while ((rc = libssh2_channel_read(channel, buffer, sizeof(buffer)-1)) > 0)
    {
      buffer[rc] = 0;
      result_text << buffer;
    }

    if (rc < 0)
      throw FB::script_error("Error while receiving command output.");
  
    m_home = result_text.str();
    size_t end = m_home.find("\n");
    if (end > 0)
      m_home.erase(end, m_home.length() - end);

    parseConfig();
  }
  catch (FB::script_error e) 
  {
    if (channel)
    {
      libssh2_channel_close(channel);
      libssh2_channel_free(channel);
      channel = NULL;
    }
  }

}


/*-----------------------------------------------------------------------------*

  FileService::revoke

  Shut down the service.

 *-----------------------------------------------------------------------------*/

void FileService::revoke()
{
  m_enabled = false;

  if (m_sftp)
  {
    libssh2_sftp_shutdown(m_sftp);
    m_sftp = NULL;
  }
}


/*-----------------------------------------------------------------------------*

  FileService::create

 *-----------------------------------------------------------------------------*/

FileServicePtr FileService::create(SecureConnectionPtr connection,
				   const std::string& scheme,
				   const std::string& configText)
{
  FileServicePtr fs = boost::make_shared<FileService>(connection, scheme, configText);
  fs->start();
  return fs;
}


/*-----------------------------------------------------------------------------*

  FileService::reportError

 *-----------------------------------------------------------------------------*/

void FileService::reportError(const FB::script_error& e) 
{
  FireEvent("onerror", FB::variant_list_of(shared_from_this())(e.what()));
}


/*-----------------------------------------------------------------------------*
  
  FileService::re_glob_transform

  This is used to translate the full path indicated by a config specifier into
  a regular expression that matches what it globs to. Sequentially, perform
  the following conversion:

    escape special characters .[{}()\+|^$, but not *?{} ...
    ? --> .
    * --> (\\\\.|[^\\/])*
    {a,b,...} --> (a|b|...)
    ... --> .*
    remove trailing /
 
  Processing braced alternatives is easier if the three syntactic parts --
  '{', ',', and '}'. Hence we iteratively apply a regular expression to select
  the next piece by required transformation:

    ... --> .* ==> (\.{3})
    one of .[{}()\+|^$ --> \_ ==> ([][{}()\+|^$])
    ? --> . ==> (\?)
    * --> (\.|[^\/])* ==> (\*)
    { --> ( ==> (\{)
    , --> | ==> (,)
    } --> ) ==> (\})

 *-----------------------------------------------------------------------------*/

boost::regex FileService::re_glob_transform("(\\.{3})"
					    "|([].[{}()\\+|^$])"
					    "|(\\?)"
					    "|(\\*)"
					    "|(\\{)"
					    "|(,)"
					    "|(\\})"
					    "|(.)");


/*-----------------------------------------------------------------------------*

  FileService::parseConfig

  SecureConnection supplies a service its configuration file contents at
  construction.  This function parses that text and sets options appropriately
  in the FileService instance.

  It is written from the perspective that the remote host is POSIX-based. For
  Windows-based hosts, the syntax of the configuration file's path specifiers
  will have to be slightly different due to the use of backslash as a path
  separator rather than a character escape.

 *-----------------------------------------------------------------------------*/

void FileService::parseConfig() 
{
  // Yet another hand-coded parser.

  std::vector<std::string> lines;
  boost::split(lines, m_configText, boost::is_any_of("\n"));

  // Default policy is to deny access to anything named .* in the users home
  // directory, as these files typically contain sensitive configuration
  // information (such as the keys in .ssh).  This is accomplished by implicitly
  // appending two lines to every config.  This behavior can be modified through
  // the use of [override] sections.
  lines.push_back("[noaccess]");
  lines.push_back("+~/.*");
  
  int count = lines.size();
  std::stringstream msg("FileService configuration error at line ");

  // Used to track the number of leading whitespace characters in specifier
  // lines.  Indentation is limited to no more than 1 greater than in the
  // prior line.
  int column = 0;

  // This is used to make sure that we've seen a section config line before a
  // specifier line.
  int section = false;

  SecureConnectionPtr connection = m_connection.lock();

  // Used to determine the full path of each specifier
  std::vector<boost::filesystem::path> parent_paths;

  // Used to hold the ConfigLine instances during processing.  Later they will
  // be moved to final_config and overrides, and ultimately into the m_config
  // member.
  std::vector<ConfigLine> raw_config;

  for(int i = 0; i < count; i++)
  {
    const std::string& line(lines[i]);
    ConfigLine cl(line);
    
    switch (cl.getType())
    {
    case ConfigLine::BLANK:
      continue;

    case ConfigLine::SECTION:
      section = true;
      column = 0;
      break;

    case ConfigLine::SPECIFIER:
      {
	if (!section)
	{
	  msg << i << ": the first non-blank line must be a section heading, not a specifier: \n";
	  msg << line;
	  connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	  return;
	}

	if (cl.getIndent() > column+1) 
	{
	  msg << i << ": indent too deep, greater than 1 more than the previous: \n";
	  msg << line;
	  connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	  return;
	}

	column = cl.getIndent();
	parent_paths.resize(column+1);

	boost::filesystem::path p(cl.getPath());
	std::string p_str(p.string());
	boost::filesystem::path::iterator p_it = p.begin();

	if (column > 0)
	{
	  // boost::filesystem::path::is_absolute won't work here, since it is
	  // configured for the local machine, not the remote one.  In the
	  // case that the local machine is Windows and the remote POSIX,
	  // /home/users/foobar would not be considered absolute.  The same
	  // applies below for is_relative in a similar test just below.
	  // Hence we have to resort to more primitive means.
	  if (p_str[0] == '/' || p_str[0] == '~')
	  {
	    msg << i << ": child specifier must have a relative path: \n";
	    msg << line;
	    connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	    return;
	  }

	  cl.setPath(boost::filesystem::path(parent_paths[column-1]) /= p);
	}
	else
	{
	  // See the comment in the subsequent clause of "if (column > 0)" above.
	  if (p_str[0] != '/' && p_str[0] != '~')
	  {
	    // Would it be more civil to presume top-level relative paths
	    // imply the home directory as a base?

	    msg << i << ": root specifier must have an absolute path: \n";
	    msg << line;
	    connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	    return;
	  }

	  std::string comp0;

	  if (p_it != p.end() && (comp0 = *p_it)[0] == '~')
	  {
	    if (comp0.length() > 1)
	    {
	      // ~username is not supported, only login user's home directory
	      // TODO: consider what it would take to support ~username
	      //   1. scan config for ~user.
	      //   2. build command using cd and pwd
	      //   3. have results ready before this function starts
	      // ? would this consititute information leakage ?
	      //  probably not, since this file must be edited while logged in
	      //  to the server in question, and the permissions are only 
	      //  implicitly available (request, fail) to the JS client.
	      msg << i << ": ~username is not supported in config specifiers, only ~/: \n";
	      msg << line;
	      connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	      return;
	    }

	    boost::filesystem::path full_path(m_home);
	    for (p_it++; p_it != p.end(); p_it++)
	      full_path /= *p_it;

	    cl.setPath(full_path);
	  }
	}
	p = parent_paths[column] = cl.getPath();
	
	// Normalize path to remove occurrences of /./ and to collapse
	// occurrences of /../ 

	p_it = p.begin();
	boost::filesystem::path::iterator p_end = p.end();
	std::vector<boost::filesystem::path> prefixes;

	while (p_it != p_end)
	{
	  std::string comp(*p_it++);
	  if (comp.compare(".") == 0)
	    continue;

	  if (comp.compare("..") == 0)
	  {
	    // Ensure that root never gets popped.
	    if (prefixes.size() < 2)
	    {
	      msg << i << ": badly formed path, cannot resolve parent of root (/..): \n";
	      msg << line;
	      connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	      return;
	    }
	    prefixes.pop_back();
	  }
	  else
	    prefixes.push_back(boost::filesystem::path((prefixes.size() == 0) 
						       ? "" 
						       : prefixes.back()) /= comp);
	}

	cl.setPath(prefixes.back());

	// Transform (possibly globbed) path into a regular expression that
	// matches the path (or what the path globs to).
	std::string glob_path = cl.getPath().string();
	boost::sregex_iterator it(glob_path.begin(), glob_path.end(), re_glob_transform);
	boost::sregex_iterator end;
	bool within_brace = false;
	std::stringstream re_text;
	for(; it != end; it++)
	{
	  boost::match_results<std::string::const_iterator> what(*it);
	  const char* t = re_text.str().c_str();
	  const char* u = what.str().c_str();
	  if (what[1].matched) // ... --> .*
	    re_text << ".*";

	  else if (what[2].matched) // one of [].{}()\+|^$ --> \_
	    re_text << "\\" << what[2].str();

	  else if (what[3].matched) // ? --> .
	    re_text << ".";

	  else if (what[4].matched) // * --> (\\.|[^\/])*
	    re_text << "(\\\\.|[^\\/])*";

	  else if (what[5].matched) // { --> (
	  {
	    re_text << "(";
	    within_brace = true;
	  }
	  else if (what[6].matched) // , --> , or | -- depending on ?within brace?
	    re_text << (within_brace ? "|" : ",");

	  else if (what[7].matched) // } --> )
	  {
	    re_text << ")";
	    within_brace = false;
	  }
	  else if (what[8].matched) // ordinary character --> itself
	    re_text << what[8].str();
	}
      
	if (within_brace)
	{
	  // There is an unclosed brace in the path.
	  msg << i << ": unterminated {, or nested {}s, in glob: \n";
	  msg << line;
	  connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	  return;
	}

	// The processing above to remove . and .. should also ensure that the
	// path does not end in a trailing /.
	std::string regex_str = re_text.str();
	if (regex_str[regex_str.length()-1] == '/')
	{
	  msg << i << ": internal error, no terminal slash assumption failed: \n";
	  msg << line;
	  connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	  return;
	}

	// Append (/.*)?$ so that directories will match.  This also prevents
	// ~/foo from matching ~/foobar.
	const char *s = re_text.str().c_str();
	s = re_text.str().append("(/.*)?$").c_str();
	cl.setGlobRegex(boost::regex(re_text.str().append("(/.*)?$")));
	break;
      }
    case ConfigLine::SECTION_ERROR:
    case ConfigLine::SPECIFIER_ERROR:
    case ConfigLine::MISC_ERROR:
      {
	msg << i << ": ";
	switch (cl.getType())
	{
	case ConfigLine::SECTION_ERROR:
	  msg << "unrecognized config section: \n";
	  break;
	case ConfigLine::SPECIFIER_ERROR:
	  msg << "error in specifier: \n";
	  break;
	case ConfigLine::MISC_ERROR:
	  msg << "\n";
	  break;
	}

	msg << line;
	connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	return;
      }
    }

    raw_config.push_back(cl);
  }

  // Look through the config lines for specifiers after an override section
  // header.  At this point, there will only be SECTION and SPECIFIER lines.
  // Keep the non-override section headers and the specifiers that follow them
  // in final_config; keep the override specifiers in overrides, while changing
  // their signs as they are stored.  At the end the overrides are appended so
  // that they'll make holes in the default security policy of not allowing
  // access to files at or below ~/.*.

  std::vector<ConfigLine> final_config;
  std::vector<ConfigLine> overrides;
  
  std::vector<ConfigLine>::iterator o_it = raw_config.begin();
  bool in_override = false;
  while (o_it != raw_config.end())
  {
    ConfigLine& cl = *o_it++;
    if (cl.getType() == ConfigLine::SECTION)
      in_override = cl.getSection() == ConfigLine::OVERRIDE;

    if (in_override && cl.getType() == ConfigLine::SPECIFIER)
    {
      cl.setSign(!cl.getSign());
      overrides.push_back(cl);
    }

    if (!in_override)
      final_config.push_back(cl);
  }

  // Final config should end with "[noaccess]" and "+~/.*".  The overrides go
  // right after.  In the loop above, they had their signs changed so that
  // a positive statement (override this path) creates a hole in the noaccess
  // section.

  final_config.insert(final_config.end(), overrides.begin(), overrides.end());

  m_config = final_config;
  m_enabled = true;
}


/*-----------------------------------------------------------------------------*
  
  FileService::ConfigLine::SectionNames

  This map is used to translate the text in section headings with internal 
  constants representing the different section types.

 *-----------------------------------------------------------------------------*/

std::map<std::string,
	 FileService::ConfigLine::ConfigSection,
	 std::greater<std::string> > FileService::ConfigLine::SectionNames =
		 boost::assign::map_list_of
		 ("noaccess", FileService::ConfigLine::NOACCESS)
		 ("readonly", FileService::ConfigLine::READONLY)
		 ("writeonly", FileService::ConfigLine::WRITEONLY)
		 ("readwrite", FileService::ConfigLine::READWRITE)
		 ("override", FileService::ConfigLine::OVERRIDE);


/*-----------------------------------------------------------------------------*
  
  FileService::ConfigLine::re_blank
  FileService::ConfigLine::re_section
  FileService::ConfigLine::re_specifier

  Regular expressions to recognize the different types of config lines.

  TODO -- DRY: re_section should be derived from SectionNames above.

 *-----------------------------------------------------------------------------*/

boost::regex FileService::ConfigLine::re_blank("^\\s*$");
boost::regex FileService::ConfigLine::re_section("^\\[\\s*(noaccess|readonly|writeonly|readwrite|override)\\s*\\]\\s*$", boost::regex::icase);
boost::regex FileService::ConfigLine::re_specifier("^(\\s*)(\\+|-)\\s*(.*[^ ])\\s*$");


// Used to give a more helpful error message for errors in section headers.

boost::regex FileService::ConfigLine::re_crude_section("^\\[");


/*-----------------------------------------------------------------------------*
  
  FileService::ConfigLine::ConfigLine

  FileService's configuration file is line-based. Each line will have one of
  three valid types, or one of three error types. Valid lines can be blank,
  can initiate a new section, or can specify a subtree to be included or
  excluded from the current section. There is an error type for each non-blank
  line and one for catch-all errors. Currently, the SPECIFIER_ERROR type is
  unreachable, as errors in specifiers are caught in FileService::parseConfig,
  not here.

 *-----------------------------------------------------------------------------*/

FileService::ConfigLine::ConfigLine(const std::string& line)
{
  boost::smatch what;

  if (boost::regex_match(line, what, re_blank))
    m_type = BLANK;

  else if (boost::regex_match(line, what, re_section))
  {
    m_type = SECTION;
    std::string target(what[1].str());
    boost::to_lower(target);
    std::map<std::string, ConfigSection>::iterator it = SectionNames.find(target);
    
    if (it == SectionNames.end())
      m_type = SECTION_ERROR;
    else
      m_section = it->second;
  }
  else if (boost::regex_match(line, what, re_specifier))
  {
    m_type = SPECIFIER;

    m_specifier.indent = what[1].str().length();
    m_specifier.sign = (what[2].str().compare("+") == 0) ? 1 : 0;

    m_specifier.path = boost::filesystem::path(what[3].str());
  }
  else if (boost::regex_match(line, what, re_crude_section))
    // found something bracketed, but not recognizable as a section
    m_type = SECTION_ERROR;

  else
    m_type = MISC_ERROR;
}


/*-----------------------------------------------------------------------------*

  FileService::isReadable

  Checks to see if the path can be read.

 *-----------------------------------------------------------------------------*/

bool FileService::isReadable(const std::string& path)
{
  return getPermissions(path).first;
}


/*-----------------------------------------------------------------------------*

  FileService::isWriteable

  Checks to see if the path can be written.

 *-----------------------------------------------------------------------------*/

bool FileService::isWriteable(const std::string& path)
{
  return getPermissions(path).second;
}


/*-----------------------------------------------------------------------------*

  FileService::getPermissions

  Checks the given path against the config permissions.

  parseConfig set up m_config so that all this function need do is evaluate
  each ConfigLine in turn, testing whether or not the contained regex matches
  the given path. If it does, then the appropriate permissions for that
  section are recorded; if not no action is taken. The last result wins. By
  default, all permissions are assumed to be refused, so only positive action
  on the part of the user who edited the config file can allow a file to be
  accessed.

  The pair returned has read permission first, write second.

 *-----------------------------------------------------------------------------*/

std::pair<bool, bool> FileService::getPermissions(const std::string& path)
{
  std::vector<ConfigLine>::iterator it;
  boost::smatch match; 

  // parseConfig requires that a section be prior to all specifiers, so that
  // this is guaranteed to be initialized before a specifier is processed
  // below.
  FileService::ConfigLine::ConfigSection section;

  bool canRead = false;
  bool canWrite = false;

  for (it = m_config.begin(); it != m_config.end(); it++)
  {
    const char *s = (it->getType() == ConfigLine::SPECIFIER) ? it->getGlobRegex().str().c_str() : "";
    if (it->getType() == ConfigLine::SECTION)
      section = it->getSection();

    else if (boost::regex_match(path, match, it->getGlobRegex()))
    {
      switch (section)
      {
      case ConfigLine::NOACCESS:
	canRead = false;
	canWrite = false;
	break;

      case ConfigLine::READWRITE:
	canRead = true;
	canWrite = true;
	break;

      case ConfigLine::READONLY:
	canRead = true;
	canWrite = false;
	break;

      case ConfigLine::WRITEONLY:
	canRead = false;
	canWrite = true;
	break;

      default:
	// This shouldn't happen; errors and overrides should have been
	// filtered out by now.
	break;
      } 
    }
  }
  return std::pair<bool, bool>(canRead, canWrite);
}


/*-----------------------------------------------------------------------------*

  FileService::get

  Get returns a command instance to fetch a file from the remote host.

 *-----------------------------------------------------------------------------*/

FB::JSAPIPtr FileService::get(const std::string& path)
{
  bool enabled = true;

  if (!m_enabled)
  {
    reportError(FB::script_error("Service is disabled."));
    enabled = false;
  }
  else if (!isReadable(path))
  {
    reportError(FB::script_error("Permission denied."));
    enabled = false;
  }

  return boost::make_shared<FileServiceGetCommand>(FB::ptr_cast<FileService>(shared_from_this()), path, enabled);
}


////////////////////////////////////////////////////////////////////////////////


/*-----------------------------------------------------------------------------*

  FileServiceGetCommand::FileServiceGetCommand

 *-----------------------------------------------------------------------------*/

FileServiceGetCommand::FileServiceGetCommand(const FileServicePtr& service,
					     const std::string& path,
					     bool enabled)
  : m_service(service),
    m_path(path),
    m_enabled(enabled)
{
  registerMethod("exec", make_method(this, &FileServiceGetCommand::exec));
  registerEvent("onresult");
  registerEvent("onerror");
}


/*-----------------------------------------------------------------------------*

  FileServiceGetCommand::report

 *-----------------------------------------------------------------------------*/

void FileServiceGetCommand::report(const std::string& event, FB::VariantList args)
{
  FireEvent(event, args);
  args.insert(args.begin(), shared_from_this());
  m_service->FireEvent(event, args);
}


/*-----------------------------------------------------------------------------*

  FileServiceGetCommand::reportResult

 *-----------------------------------------------------------------------------*/

void FileServiceGetCommand::reportResult(FB::VariantList args)
{
  report("onresult", args);
}


/*-----------------------------------------------------------------------------*

  FileServiceGetCommand::reportError

 *-----------------------------------------------------------------------------*/

void FileServiceGetCommand::reportError(const FB::script_error& e) 
{
  report("onerror", FB::variant_list_of(shared_from_this())(e.what()));
}



/*-----------------------------------------------------------------------------*

  FileServiceGetCommand::exec

  This is the meat of the get command, boilerplate sftp file fetch.

 *-----------------------------------------------------------------------------*/

void FileServiceGetCommand::exec(const FB::JSObjectPtr& callback)
{
  if (!m_enabled)
    return;

  LIBSSH2_SFTP_HANDLE *file = NULL;
  
  try
  {
    if (!(file = libssh2_sftp_open(m_service->m_sftp,
				   m_path.c_str(), LIBSSH2_FXF_READ, 0)))
      throw FB::script_error("File not found.");

    // For now, presume contents are ascii.
    // TODO -- encoding
    std::stringstream contents;

    int rc;
    char buffer[BUFFER_SIZE];
    while ((rc = libssh2_sftp_read(file, buffer, sizeof(buffer)-1)) > 0)
    {
      buffer[rc] = 0;
      contents << buffer;
    }

    if (rc < 0) 
      throw FB::script_error(std::string("Error while reading file: ").append(strerror(errno)));

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
