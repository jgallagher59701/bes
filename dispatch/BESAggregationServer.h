// BESAggregationServer.h

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
 
// (c) COPYRIGHT University Corporation for Atmostpheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#ifndef BESAggregationServer_h_
#define BESAggregationServer_h_ 1

#include "BESDataHandlerInterface.h"

/** @brief Abstraction representing mechanism for aggregating data
 */
class BESAggregationServer
{
private:
    string		_name ;
			BESAggregationServer() {}
protected:
    			BESAggregationServer( string name )
			    : _name( name ) {}
public:
    virtual		~BESAggregationServer() {} ;
    /** @brief aggregate the response object
     *
     * @param dhi structure which contains the response object and the
     * aggregation command
     * @throws BESAggregationException if problem aggregating the data
     * @see BESAggregationException
     * @see _BESDataHandlerInterface
     */
    virtual void	aggregate( BESDataHandlerInterface &dhi ) = 0 ;

    virtual string	get_name() { return _name ; }
};

#endif // BESAggregationServer_h_

// $Log: BESAggregationServer.h,v $
// Revision 1.2  2005/03/26 00:33:33  pwest
// fixed aggregation server invoke problems. Other commands use the aggregation command but no aggregation is needed, so set aggregation to empty string when done
//
// Revision 1.1  2005/02/01 17:48:17  pwest
//
// integration of ESG into opendap
//