/******************************************************************************

  HostServicesPlugin.cpp

  HostServicesPlugin is the root of the object heirarchy for the
  plugin.  It is essentially "int main(argc, argv)" for a FireBreath plugin.

  ----------------------------------------------------------------------------

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

  ----------------------------------------------------------------------------

  Copyright 2011 Pat M. Lasswell.

 ******************************************************************************/



#if FB_X11
#include <gtkmm.h>
#include <gtkmm/plug.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include "PluginWindowX11.h"
#endif


#include "HostServices.h"
#include "HostServicesPlugin.h"


/*----------------------------------------------------------------------------*

  @fn HostServicesPlugin::StaticInitialize()
 
  @brief  Called from PluginFactory::globalPluginInitialize()
 
  @see FB::FactoryBase::globalPluginInitialize

 *----------------------------------------------------------------------------*/

void HostServicesPlugin::StaticInitialize()
{
  // Place one-time initialization stuff here; As of FireBreath 1.4 this should only
  // be called once per process

  // Sets up the g type system and the Glib::wrap() table. 
  Gtk::Main::init_gtkmm_internals();
}


/*----------------------------------------------------------------------------*

  @fn HostServicesPlugin::StaticInitialize()
 
  @brief  Called from PluginFactory::globalPluginDeinitialize()
 
  @see FB::FactoryBase::globalPluginDeinitialize

 *----------------------------------------------------------------------------*/

void HostServicesPlugin::StaticDeinitialize()
{
  // Place one-time deinitialization stuff here. As of FireBreath 1.4 this should
  // always be called just before the plugin library is unloaded
}


/*----------------------------------------------------------------------------*

  @brief  HostServicesPlugin constructor.  Note that your API is not available
  at this point, nor the window.  For best results wait to use
  the JSAPI object until the onPluginReady method is called

 *----------------------------------------------------------------------------*/

HostServicesPlugin::HostServicesPlugin()
{
}


/*----------------------------------------------------------------------------*

  @brief  HostServicesPlugin destructor.

 *----------------------------------------------------------------------------*/

HostServicesPlugin::~HostServicesPlugin()
{
  // This is optional, but if you reset m_api (the shared_ptr to your JSAPI
  // root object) and tell the host to free the retained JSAPI objects then
  // unless you are holding another shared_ptr reference to your JSAPI object
  // they will be released here.
  releaseRootJSAPI();
  m_host->freeRetainedObjects();
}


void HostServicesPlugin::onPluginReady()
{
  // When this is called, the BrowserHost is attached, the JSAPI object is
  // created, and we are ready to interact with the page and such.  The
  // PluginWindow may or may not have already fire the AttachedEvent at
  // this point.
  m_view = boost::make_shared<RequestPasswordView>(this, GetWindow());
  m_view->ok_signal.connect(boost::bind(&HostServicesPlugin::passwordRequestFilled, this));
  m_view->cancel_signal.connect(boost::bind(&HostServicesPlugin::passwordRequestDenied, this));
}



/*----------------------------------------------------------------------------*

  @brief  Creates an instance of the JSAPI object that provides your main
  Javascript interface.
 
  Note that m_host is your BrowserHost and shared_ptr returns a
  FB::PluginCorePtr, which can be used to provide a
  boost::weak_ptr<HostServicesPlugin> for your JSAPI class.
 
  Be very careful where you hold a shared_ptr to your plugin class from,
  as it could prevent your plugin class from getting destroyed properly.

 *----------------------------------------------------------------------------*/

FB::JSAPIPtr HostServicesPlugin::createJSAPI()
{
  // m_host is the BrowserHost
  return boost::make_shared<HostServices>(FB::ptr_cast<HostServicesPlugin>(shared_from_this()),
                                                      m_host);
}


bool HostServicesPlugin::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
  //printf("Mouse down at: %d, %d\n", evt->m_x, evt->m_y);
  return false;
}


bool HostServicesPlugin::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
  //printf("Mouse up at: %d, %d\n", evt->m_x, evt->m_y);
  return false;
}


bool HostServicesPlugin::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
  //printf("Mouse move at: %d, %d\n", evt->m_x, evt->m_y);
  return false;
}


bool HostServicesPlugin::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *)
{
  // The window is attached; act appropriately
  return false;
}


bool HostServicesPlugin::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
  // The window is about to be detached; act appropriately
  return false;
}


/*-----------------------------------------------------------------------------*

  HostServicesPlugin::requestCredentials

  userName: name of the user account on the remote host for which credentials
    are being requested.

  hostName: name of the remote host.

  The first three parameters are not interpreted in any way, but are used to
  build a descriptive string for the user explaining which credentials are
  requested an on behalf of whom (what?).

  The callback is a function of two parameters that returns void.  The first
  parameter is a boolean, indicating whether or not the second parameter is
  valid.  In the case that the user cancels the request for credentials, or
  that the request is otherwise aborted, this function will be called with
  false in the first parameter, and an empty string in the second. If the
  first parameter is true, then the second parameter contains the requested
  password.  Unless something very bad happens, this function will always be
  called; it will be called at most once per invocation of requestCredentials.

  requestCredentials is non-blocking and will return almost immediately. The
  callback will be called after an arbitrarily long time.

 *-----------------------------------------------------------------------------*/

void HostServicesPlugin::requestCredentials(SecureConnectionPtr connection)
{
  if (!m_host->isMainThread())
  {
    m_host->ScheduleOnMainThread(shared_ptr(), boost::bind(&HostServicesPlugin::requestCredentials,
                                                           connection));
    return;
  }

  m_host->assertMainThread();
  
  if (connection == m_viewOwner)
    return;

  if (m_viewWaitList.find(connection) != m_viewWaitlist.end())
    return;

  if (m_viewOwner == NULL)
  {
    m_viewOwner = connection;
    m_view->setLabel(connection->description());
    m_view->enable();
  }
  else
    m_viewWaitlist.push_back(connection);
}


/*-----------------------------------------------------------------------------*

  HostServicesPlugin::passwordRequestFilled

 *-----------------------------------------------------------------------------*/

void HostServicesPlugin::passwordRequestFilled()
{
  m_view.disable();

  if (!m_host->isMainThread())
  {
    m_host->ScheduleOnMainThread(shared_ptr(), boost::bind(&HostServicesPlugin::passwordRequestFilled));
    return;
  }

  m_host->assertMainThread();
  
  SecureConnectionPtr connection = m_viewOwner;
  std::string password = m_view->getPassword();
  
  handleNextCredentialsRequest();

  connection->credentialsRequestFilled(password);
}


/*-----------------------------------------------------------------------------*

  HostServicesPlugin::passwordRequestDenied

 *-----------------------------------------------------------------------------*/

void HostServicesPlugin::passwordRequestDenied()
{
  m_view.disable();

  if (!m_host->isMainThread())
  {
    m_host->ScheduleOnMainThread(shared_ptr(), boost::bind(&HostServicesPlugin::passwordRequestDenied));
    return;
  }

  m_host->assertMainThread();

  SecureConnectionPtr connection = m_viewOwner;
  handleNextCredentialsRequest();
  connection->credentialsRequestDenied();
}

/*-----------------------------------------------------------------------------*

  HostServicesPlugin::handleNextCredentialsRequest

 *-----------------------------------------------------------------------------*/

void HostServicesPlugin::handleNextCredentialsRequest()
{
  m_viewOwner = NULL;

  if (m_viewWaitlist.size() > 0)
  {
    m_viewOwner = m_viewWaitlist.front();
    viewWaitlist.pop_front();
    m_view->setLabel(m_viewOwner->description());
    m_view->enable();
  }
}

// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// fill-column: 78
// End:
