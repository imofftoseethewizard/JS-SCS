/******************************************************************************

  AsyncCommand.h

    Asyncronous command support. 


  interface AsyncCommand {
    void exec(in optional Function callback);
  };

  AsyncCommand implements EventTarget;

  Javascript provides a single-threaded programming model.  Long operations
  are necessarily broken up into short initiation sequences and callbacks
  which handle the eventuals results and errors.  In the interest of 
  developing an indiom in which this factorization appears more like
  traditional programming, AsyncCommand encapsulates this mechanism so that,
  we could write, e.g, 

    function getEmacsInit() {
      openSecureConnection("patl", "foobar.com").then(function (connection) {
	connection.addEventListener("error", function(msg) {
	  alert(msg);
	});
	connection.requestServiceByScheme("file").then(function (service) {
          service.addEventListener("error", function(msg) {
	    alert(msg);
	  });
	  service.get("~/.emacs.d/init.el").then(function(file) {
	    document.body.appendChild(document.createTextNode(file.contents));
	  });
	});
      });
    }

  where openSecureConnection returns an AsyncCommand instance.  

  A more standard factorization would be 

    function getEmacsInit() {
      openSecureConnection("patl", "foobar.com", function (connection) {
	connection.addEventListener("error", function(msg) {
	  alert(msg);
	});
	connection.requestServiceByScheme("file", function (service) {
          service.addEventListener("error", function(msg) {
	    alert(msg);
	  });
	  service.get("~/.emacs.d/init.el", function(file) {
	    document.body.appendChild(document.createTextNode(file.contents));
	  });
	});
      });
    }

  This does not, however, admit the addition of event listeners on
  individual commands, nor provide any way to know precisely which command
  caused the error, as in this idiom:

    function getEmacsInit() {
      openSecureConnection("patl", "foobar.com").exec(function (connection) {
        var service, request;
         
	connection.addEventListener("error", function(cmd, msg) {
          if (cmd === service) {
            ...
          } else if (cmd == request) {
            ...
          }
	});

       (request = connection.requestServiceByScheme("file")).exec(function (service) {
          (service = service.get("~/.emacs.d/init.el")).exec(function(file) {
	    document.body.appendChild(document.createTextNode(file.contents));
	  });
	});
      });
    }

  
  
******************************************************************************/
/*
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "JSAPIAuto.h"

#ifndef H_AsyncCommand
#define H_AsyncCommand


template <class Action>
class AsyncCommand : FB::JSAPIAuto 
{
public:
  AsyncCommand(Action action);
  AsyncCommand(const FB::JSAPIPtr &provider, Action action);

  void exec(const FB::JSObjectPtr& callback);

  FB::JSObjectPtr get_onerror() const;
  void set_onerror(const FB::JSObjectPtr& callback);

  void reportError(FB::script_error e);

  typedef boost::shared_ptr<AsyncCommand<Action> > Ptr;
  typedef boost::weak_ptr<AsyncCommand<Action> > WeakPtr;

private:
  FB::JSAPIPtr m_provider;
  Action m_action;

  FB::JSObjectPtr m_onerror;
};


#endif // H_AsyncCommand
*/
