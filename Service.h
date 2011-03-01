/******************************************************************************

  Service.h

  Service is a base class for all provided services, a place to hang common
  code.

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


#include <string.h>

#include "JSAPIAuto.h"

#include "SecureConnection.h"

#ifndef H_Service
#define H_Service

class Service : public FB::JSAPIAuto
{
 public:
  Service();
  Service(SecureConnectionPtr connection,
	  const std::string& scheme,
	  const std::string& configText);

  virtual ~Service();

  FB::JSAPIPtr get_connection() const;
  std::string get_scheme() const;

  virtual void revoke();

 protected:
  std::string m_scheme;
  std::string m_configText;
  SecureConnectionWeakPtr m_connection;
  
 private:
};

typedef ServicePtr *(ServiceFactory)(SecureConnectionPtr connection,
				     const std::string& scheme,
				     const std::string& configText);

#endif // H_Service


// Local Variables:
// mode: c++
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
