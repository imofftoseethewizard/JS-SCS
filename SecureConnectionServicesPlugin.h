/******************************************************************************

  SecureConnectionServicesPlugin.h

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

 ******************************************************************************/

#ifndef H_SecureConnectionServicesPLUGIN
#define H_SecureConnectionServicesPLUGIN

#include "PluginWindow.h"
#include "PluginEvents/MouseEvents.h"
#include "PluginEvents/AttachedEvent.h"

#include "PluginCore.h"


FB_FORWARD_PTR(SecureConnectionServicesPlugin)
class SecureConnectionServicesPlugin : public FB::PluginCore
{
 public:
  static void StaticInitialize();
  static void StaticDeinitialize();

 public:
  SecureConnectionServicesPlugin();
  virtual ~SecureConnectionServicesPlugin();

 public:
  void onPluginReady();
  virtual FB::JSAPIPtr createJSAPI();
  // If you want your plugin to always be windowless, set this to true
  // If you want your plugin to be optionally windowless based on the
  // value of the "windowless" param tag, remove this method or return
  // FB::PluginCore::isWindowless()
  virtual bool isWindowless() { return false; }

  BEGIN_PLUGIN_EVENT_MAP()
    EVENTTYPE_CASE(FB::MouseDownEvent, onMouseDown, FB::PluginWindow)
    EVENTTYPE_CASE(FB::MouseUpEvent, onMouseUp, FB::PluginWindow)
    EVENTTYPE_CASE(FB::MouseMoveEvent, onMouseMove, FB::PluginWindow)
    EVENTTYPE_CASE(FB::MouseMoveEvent, onMouseMove, FB::PluginWindow)
    EVENTTYPE_CASE(FB::AttachedEvent, onWindowAttached, FB::PluginWindow)
    EVENTTYPE_CASE(FB::DetachedEvent, onWindowDetached, FB::PluginWindow)
  END_PLUGIN_EVENT_MAP()

    /** BEGIN EVENTDEF -- DON'T CHANGE THIS LINE **/
  virtual bool onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *);
  virtual bool onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *);
  virtual bool onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *);
  virtual bool onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *);
  virtual bool onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *);
  /** END EVENTDEF -- DON'T CHANGE THIS LINE **/
};


#endif

// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
