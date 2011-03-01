/******************************************************************************

  RequestPasswordView.cpp

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

#include <gtkmm.h>

#include "RequestPasswordView.h"

// Unclear what the proper value is. A google search turns up a few references
// to 32 being the internal limit.  Given human memory and typing accuracy,
// 15 is probably a practical limit, and 32 provides plenty of headspace.

#define MAX_PASSWORD_LENGTH 32

/*-----------------------------------------------------------------------------*

  RequestPasswordView::RequestPasswordView

  The constructor creates the widgets (or controls) and adds them to the given
  window.  It connects signal/event handlers.  The controls are initially
  disabled by default.
 
 *-----------------------------------------------------------------------------*/

#if FB_X11
RequestPasswordView::RequestPasswordView(FB::PluginWindow *window)
  : m_window(dynamic_cast<FB::PluginWindowX11*>(window))

{
  Gtk::Plug *plug = Glib::wrap(GTK_PLUG(m_window->getContainer()));
  Gtk::VBox *vbox = Gtk::manage(new Gtk::VBox());
  plug->add(*vbox);
  vbox->show();

  m_label = Gtk::manage(new Gtk::Label());
  vbox->pack_start(*m_label);

  m_entry = Gtk::manage(new Gtk::Entry());
  vbox->pack_start(*m_entry);

  // For a password -- hide text
  m_entry->set_visibility(false);
  m_entry.set_max_length(MAX_PASSWORD_LENGTH);

  Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox());
  vbox->pack_start(*hbox);
 
  m_ok_button = Gtk::manage(new Gtk::Button(Gtk::Stock::OK));
  vbox->pack_end(*m_ok_button);
  m_ok_button->show();

  m_cancel_button = Gtk::manage(new Gtk::Button(Gtk::Stock::CANCEL));
  vbox->pack_end(*m_cancel_button);
  m_cancel_button->show();

  m_ok_button.signal_clicked().connect(sigc::mem_fun(*this, &RequestPasswordView::on_ok));
  m_cancel_button.signal_clicked().connect(sigc::mem_fun(*this, &RequestPasswordView::on_cancel));

  disable();
  // TODO -- connect keyboard signals to enter and escape
}
#endif


/*-----------------------------------------------------------------------------*

  RequestPasswordView::~RequestPasswordView

  Removes the controls/widgets created in the constructor.

 *-----------------------------------------------------------------------------*/

RequestPasswordView::~RequestPasswordView()
{
}


#if FB_X11
/*-----------------------------------------------------------------------------*

  RequestPasswordView::on_ok

  The ok button was clicked, or enter was pressed.

 *-----------------------------------------------------------------------------*/

RequestPasswordView::on_ok()
{
  if (m_enabled) ok_signal();
}


/*-----------------------------------------------------------------------------*

  RequestPasswordView::on_cancel

  The cancel button was clicked, or escape was pressed.

 *-----------------------------------------------------------------------------*/

RequestPasswordView::on_cancel()
{
  if (m_enabled) cancel_signal();
}
#endif


/*-----------------------------------------------------------------------------*

  RequestPasswordView::setLabel

  const std::string& label: New label text.
 
  Sets text into the label of the view.  The text should explain for which user
  account, and on which host the password is requested.  Optionally, a port
  could be included, but that at present seems overly specific.

 *-----------------------------------------------------------------------------*/

void RequestPasswordView::setLabel(const std::string& label)
{
  if (m_enabled)
    m_label.set_text(label);
}
  

/*-----------------------------------------------------------------------------*

  RequestPasswordView::getPassword

  --> std::string: The password entered by the user.

 *-----------------------------------------------------------------------------*/

std::string RequestPasswordView::getPassword() const
{
  return m_enabled ? m_entry.get_text() : "";
}

  
/*-----------------------------------------------------------------------------*

  RequestPasswordView::clearEntry

  Clears the password text widget.

 *-----------------------------------------------------------------------------*/

void RequestPasswordView::clearEntry()
{
  if (m_enabled)
    m_entry.set_text("");
}


/*-----------------------------------------------------------------------------*

  RequestPasswordView::enable

  Enables the controls in the view.

 *-----------------------------------------------------------------------------*/

void RequestPasswordView::enable()
{
  m_entry->set_sensitive(true);
  m_ok_button->set_sensitive(true);
  m_cancel_button->set_sensitive(true);
  m_enabled = true;
}


/*-----------------------------------------------------------------------------*

  RequestPasswordView::disable

  Disables the controls in the view.  When the view is disabled, neither onOK nor
  onCancel will be called.

 *-----------------------------------------------------------------------------*/

void RequestPasswordView::disable()
{
  m_entry->set_sensitive(false);
  m_ok_button->set_sensitive(false);
  m_cancel_button->set_sensitive(false);
  m_enabled = false;
}



// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// fill-column: 78
// End:
