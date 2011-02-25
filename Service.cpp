/******************************************************************************

  Service.cpp

  Service is a base class for all provided services, a place to hang common
  code.

  ---------------------------------------------------------------------------

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

  ---------------------------------------------------------------------------

  Copyright 2011 Pat M. Lasswell.

 ******************************************************************************/

#include "Service.h"


Service::Service(SecureConnectionPtr connection,
		 const std::string& scheme,
		 const std::string& configText)
  : m_connection(connection),
    m_scheme(scheme),
    m_configText(configText)
{
  registerAttribute("connection", connection, true);
  registerAttribute("scheme",     scheme,     true);
}


Service::~Service()
{
}


FB::JSAPIPtr Service::get_connection() const
{
  return boost::shared_ptr<SecureConnection>(m_connection);
}


std::string Service::get_scheme() const
{
  return m_scheme;
}


void Service::revoke() 
{
}


// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
