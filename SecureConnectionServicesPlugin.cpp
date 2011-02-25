/******************************************************************************

  SecureConnectionServicesPlugin.cpp

  SecureConnectionServicesPlugin is the root of the object heirarchy for the
  plugin.  It is essentially "int main(argc, argv)" for a FireBreath plugin.

  ----------------------------------------------------------------------------

  This file is part of JS/SCS.

  JS/SCS is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  JS/SCS is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  JS/SCS.  If not, see <http://www.gnu.org/licenses/>.

  ----------------------------------------------------------------------------

  Copyright 2011 Pat M. Lasswell.

 ******************************************************************************/


#include "SecureConnectionServices.h"
#include "SecureConnectionServicesPlugin.h"


/*----------------------------------------------------------------------------*

  @fn SecureConnectionServicesPlugin::StaticInitialize()
 
  @brief  Called from PluginFactory::globalPluginInitialize()
 
  @see FB::FactoryBase::globalPluginInitialize

  *----------------------------------------------------------------------------*/

void SecureConnectionServicesPlugin::StaticInitialize()
{
  // Place one-time initialization stuff here; As of FireBreath 1.4 this should only
  // be called once per process
}


/*----------------------------------------------------------------------------*

  @fn SecureConnectionServicesPlugin::StaticInitialize()
 
  @brief  Called from PluginFactory::globalPluginDeinitialize()
 
  @see FB::FactoryBase::globalPluginDeinitialize

  *----------------------------------------------------------------------------*/

void SecureConnectionServicesPlugin::StaticDeinitialize()
{
  // Place one-time deinitialization stuff here. As of FireBreath 1.4 this should
  // always be called just before the plugin library is unloaded
}


/*----------------------------------------------------------------------------*

  @brief  SecureConnectionServicesPlugin constructor.  Note that your API is not available
  at this point, nor the window.  For best results wait to use
  the JSAPI object until the onPluginReady method is called

  *----------------------------------------------------------------------------*/

SecureConnectionServicesPlugin::SecureConnectionServicesPlugin()
{
}


/*----------------------------------------------------------------------------*

  @brief  SecureConnectionServicesPlugin destructor.

  *----------------------------------------------------------------------------*/

SecureConnectionServicesPlugin::~SecureConnectionServicesPlugin()
{
  // This is optional, but if you reset m_api (the shared_ptr to your JSAPI
  // root object) and tell the host to free the retained JSAPI objects then
  // unless you are holding another shared_ptr reference to your JSAPI object
  // they will be released here.
  releaseRootJSAPI();
  m_host->freeRetainedObjects();
}


void SecureConnectionServicesPlugin::onPluginReady()
{
  // When this is called, the BrowserHost is attached, the JSAPI object is
  // created, and we are ready to interact with the page and such.  The
  // PluginWindow may or may not have already fire the AttachedEvent at
  // this point.
}


/*----------------------------------------------------------------------------*

  @brief  Creates an instance of the JSAPI object that provides your main
  Javascript interface.
 
  Note that m_host is your BrowserHost and shared_ptr returns a
  FB::PluginCorePtr, which can be used to provide a
  boost::weak_ptr<SecureConnectionServicesPlugin> for your JSAPI class.
 
  Be very careful where you hold a shared_ptr to your plugin class from,
  as it could prevent your plugin class from getting destroyed properly.

  *----------------------------------------------------------------------------*/

FB::JSAPIPtr SecureConnectionServicesPlugin::createJSAPI()
{
  // m_host is the BrowserHost
  return boost::make_shared<SecureConnectionServices>(FB::ptr_cast<SecureConnectionServicesPlugin>(shared_from_this()), m_host);
}


bool SecureConnectionServicesPlugin::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
  //printf("Mouse down at: %d, %d\n", evt->m_x, evt->m_y);
  return false;
}


bool SecureConnectionServicesPlugin::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
  //printf("Mouse up at: %d, %d\n", evt->m_x, evt->m_y);
  return false;
}


bool SecureConnectionServicesPlugin::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
  //printf("Mouse move at: %d, %d\n", evt->m_x, evt->m_y);
  return false;
}


bool SecureConnectionServicesPlugin::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *)
{
  // The window is attached; act appropriately
  return false;
}


bool SecureConnectionServicesPlugin::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
  // The window is about to be detached; act appropriately
  return false;
}

// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
