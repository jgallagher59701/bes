// BESDapError.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004,2005 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.edu> and Jose Garcia <jgarcia@ucar.edu>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301
 
// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#include <sstream>

using std::ostringstream;

#include "BESDapError.h"
#include "BESContextManager.h"
#include "BESDapErrorInfo.h"

/** @brief handles exceptions if the error context is set to dap2
 *
 * If the error context from the BESContextManager is set to dap2 then
 * handle all exceptions by returning transmitting them as dap2 error
 * messages.
 *
 * @param e exception to be handled
 * @param dhi structure that holds request and response information
 */
int
BESDapError::handleException( BESError &e, BESDataHandlerInterface &dhi )
{
    // If we are handling errors in a dap2 context, then create a
    // DapErrorInfo object to transmit/print the error as a dap2
    // response.
    bool found = false ;
    string context =
	BESContextManager::TheManager()->get_context( "errors", found ) ;
    if( context == "dap2" )
    {
	ErrorCode ec = unknown_error ;
	BESDapError *de = dynamic_cast<BESDapError*>( &e);
	if( de )
	{
	    ec = de->get_error_code() ;
	}
	dhi.error_info = new BESDapErrorInfo( ec, e.get_message() ) ;

	return e.get_error_type() ;
    }
    else
    {
	// If we are not in a dap2 context and the exception is a dap
	// handler exception, then convert the error message to include the
	// error code. If it is or is not a dap exception, we simply return
	// that the exception was not handled.
	BESDapError *de = dynamic_cast<BESDapError*>( &e);
	if( de )
	{
	    ostringstream s;
	    s << "libdap exception building response"
		<< ": error_code = " << de->get_error_code()
		<< ": " << de->get_message() ;
	    e.set_message( s.str() ) ;
	}
    }
    return 0 ;
}
