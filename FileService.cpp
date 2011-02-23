/******************************************************************************

  FileService.cpp

  TODO:
  function to check requested path vs config permissions
  normalize specifiers for double slashes, and single and double dots
  
  give regexes for the config lines a suffix to capture all subtree elements
    + /?.*$

  use (re = cl.getGlobRegex()) with regex_match on the target

 ******************************************************************************/


#include <map>

#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

#include "variant_list.h"

#include "FileService.h"
#include "AsyncCommand.h"

// Used by get, command execution
#define BUFFER_SIZE 4096


FileService::FileService(SecureConnectionPtr connection,
			 const std::string& scheme,
			 const std::string& configText)
  : Service(connection, scheme, configText),
    m_sftp(NULL),
    m_home(""),
    m_disabled(true)
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
				   const std::string& configText)
{
  FileServicePtr fs = boost::make_shared<FileService>(connection, scheme, configText);
  fs->start();
  return fs;
}

void FileService::parseConfig() 
{
  // Yet another hand-coded parser.

  std::vector<std::string> lines;
  boost::split(lines, m_configText, boost::is_any_of("\n"));
  
  int count = lines.size();
  std::stringstream msg("FileService configuration error at line ");

  // Used to track the number of leading whitespace characters in specifier
  // lines.  Indentation is limited to no more than 1 greater than in the
  // prior line.  
  int column = 0;

  // Used to determine if a specifier line is nugatory, i.e. has the
  // same polarity as its parent and therefore adding no information.
  // Parity is computed as the sum of the column (0-based) of the
  // first non-space character (the + or -) and 1, if that character
  // is a +, or simply the column, if that character is a -, modulo 2.
  // This variable is set such that computed parity + parity (the
  // variable) is zero or not according to whether the specifier is
  // worthwhile or not.  Thus if column 0 has a plus, it is most
  // certainly useful, hence parity must be chosen so that (1 + 0) % 2
  // + parity = 0 (% 2).  Clearly parity must be 1.  It follows
  // therefore + charaters in even columns will be deemed useful, and
  // - characters in odd.  
  int parity = 0;

  // This is used to make sure that we've seen a section config line
  // before a specifier line.
  int section = false;

  SecureConnectionPtr connection = m_connection.lock();
  std::vector<std::string> parent_paths;

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
	  msg << i << ": the first non-blank line must be a section, not a specifier: \n";
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

	if (column > 0)
	{
	  char initial = cl.getPath()[0];
	  if (initial == '~' || initial == '/')
	  {
	    msg << i << ": child specifier must have a relative path: \n";
	    msg << line;
	    connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	    return;
	  }

	  std::string full_path(parent_paths[column-1]);
	  if (full_path[full_path.length()-1] != '/')
	    full_path.append("/");

	  full_path.append(cl.getPath());
	  cl.setPath(full_path);
	}
	else
	{
	  char initial = cl.getPath()[0];
	  if (initial != '~' && initial != '/')
	  {
	    msg << i << ": root specifier must have an absolute path: \n";
	    msg << line;
	    connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	    return;
	  }

	  if (initial == '~')
	  {
	    std::string full_path(cl.getPath());
	    if (full_path.length() > 1 && full_path[1] != '/')
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
	    full_path.erase(0, 1);
	    full_path.insert(0, m_home);
	    cl.setPath(full_path);
	  }

	  parity = cl.getSign() == ConfigLine::PLUS ? 1 : 0;
	}
	parent_paths[column] = cl.getPath();
	
	// Even though this line is not really useful, we still need to have
	// its path in the stack to ensure that its non-nugatory children
	// will be correctly interpreted.  Hence the absolute path still needs
	// to be computed and this test come afterwards.
	if (((cl.getSign() == ConfigLine::PLUS ? 1 : 0) + column + parity) % 2)
	  continue;

	// Transform globbed path into a regular expression that matches what
	// the original path globs to.
	std::string glob_path = cl.getPath();
	boost::sregex_iterator it(glob_path.begin(), glob_path.end(), re_glob_transform);
	boost::sregex_iterator end;
	bool within_brace = false;
	std::stringstream re_text;
	for(; it != end; it++)
	{
	  boost::match_results<std::string::const_iterator> what(*it);
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
	  else if (what[5].matched) // , --> , or | -- depending on ?within brace?
	    re_text << (within_brace ? "|" : ",");

	  else if (what[6].matched) // } --> )
	  {
	    re_text << ")";
	    within_brace = false;
	  }
	  else if (what[7].matched) // ordinary character --> itself
	    re_text << what[7].str();
	}
      
	if (within_brace)
	{
	  // open brace in glob 
	  msg << i << ": unterminated {, or nested {}s, in glob: \n";
	  msg << line;
	  connection->FireEvent("onerror", FB::variant_list_of(connection)(this)(msg.str()));
	  return;
	}

	cl.setGlobRegex(boost::regex(re_text.str()));
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
	m_disabled = true;
	return;
      }
    }

    m_config.push_back(cl);
  }
}


// These need to be in the same order as the ConfigSection enum
std::map<std::string,
	 FileService::ConfigLine::ConfigSection,
	 std::greater<std::string> > FileService::ConfigLine::SectionNames =
		 boost::assign::map_list_of
		 ("noaccess", FileService::ConfigLine::NOACCESS)
		 ("readonly", FileService::ConfigLine::READONLY)
		 ("writeonly", FileService::ConfigLine::WRITEONLY)
		 ("readwrite", FileService::ConfigLine::READWRITE)
		 ("override", FileService::ConfigLine::OVERRIDE);

boost::regex FileService::ConfigLine::re_blank("^\\s*$");
boost::regex FileService::ConfigLine::re_section("^\\[\\s*(noaccess|readonly|writeonly|readwrite|override)\\s*\\]\\s*$", boost::regex::icase);
//boost::regex FileService::ConfigLine::re_specifier("^(\\s*)(\\+|-)\\s*((~?/)?(([^/\\]|(\\\\)|(\\/))*)(/([^/\\]|(\\\\)|(\\/))*)*)\\s*$");
boost::regex FileService::ConfigLine::re_specifier("^(\\s*)(\\+|-)\\s*(.*[^ ])\\s*$");
    // Used to give more helpful error messages
boost::regex FileService::ConfigLine::re_crude_section("^\\[");
boost::regex FileService::ConfigLine::re_crude_specifier("^\\s*(\\+|-)");

// This is used to translate the full path indicated by a config
// specifier into a regular expression that matches what it globs
// to. Sequentially, perform the following conversion:
//   escape special characters .[{}()\+|^$, but not *?{} ...
//   ? --> .
//   * --> (\\\\.|[^\\/])*
//   {a,b,...} --> (a|b|...)
//   ... --> .*
//   remove trailing /
// 
// Iteratively apply a regular expression to select the next piece by 
// required transformation:
//   ... --> .* ==> (\\.{3})
//   one of .[{}()\+|^$ --> \_ ==> ([][{}()\\+|^$])
//   ? --> , ==> (\\?)
//   * --> (\\\\.|[^\\/])* ==> (\\*)
//   {a,b,...} --> (a|b|...) ==> (\\{(?:\\\\.|[^\\{])*\\}) == (\{(?:\\.|[^\{])*\})

boost::regex FileService::re_glob_transform("(\\.{3})|([].[{}()\\+|^$])|(\\?)|(\\*)|(\\{)|(,)|(\\})|(.)");


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
    m_specifier.sign = (what[2].str().compare("+") == 0) ? PLUS : MINUS;

    m_specifier.path = what[3].str();
  }
  else if (boost::regex_match(line, what, re_crude_section))
    m_type = SECTION_ERROR;

  else if (boost::regex_match(line, what, re_crude_specifier))
    m_type = SPECIFIER_ERROR;

  else
    m_type = MISC_ERROR;
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
