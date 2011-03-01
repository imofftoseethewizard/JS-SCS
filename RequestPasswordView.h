/******************************************************************************

  RequestPasswordView.h

  This is a view onto the plugin. It implements the UI for requesting a password
  from the user.

  ---------------------------------------------------------------------------

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

  ---------------------------------------------------------------------------

  Copyright 2011 Pat M. Lasswell.

 ******************************************************************************/

#ifndef H_REQUESTPASSWORDVIEW
#define H_REQUESTPASSWORDVIEW

#include <string>

#include <boost/signal.hpp>

#include "PluginWindow.h"

class RequestPasswordView
{
public:
  RequestPasswordView(FB::PluginWindow *window);

  ~RequestPasswordView();

  // Sets the label above the text entry widget, eg.
  //  'Password for pat@example.com:'
  void setLabel(const std::string& label);
  
  // Returns the value in the text entry widget.
  std::string getPassword() const;
  
  // Clears the text entry widget.
  void clearEntry();

  // Enables the buttons and the text entry widget.
  void enable();

  // Disables the buttons and the text entry widget.
  void disable();

  boost::signal<void ()> ok_signal;
  boost::signal<void ()> cancel_signal;

private:
#ifdef FB_X11  
  // The window in which the view is displayed.
  FB::PluginWindowX11 *m_window;

  // Widgets
  Gtk::Label *m_label;
  Gtk::Entry *m_entry;
  Gtk::Button *m_ok_button;
  Gtk::Button *m_cancel_button;

  // Ok button signal handler
  void on_ok();

  // Cancel button signal handler
  void on_cancel();
#endif
};

#endif // H_REQUESTPASSWORDVIEW


// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// fill-column: 78
// End:
