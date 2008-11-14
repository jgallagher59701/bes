// BuildTInterface.cc

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

#include <iostream>
#include <sstream>

using std::cout ;
using std::endl ;
using std::stringstream ;

#include "BuildTInterface.h"
#include "BESDataNames.h"

/** @brief Instantiate a BuildTInterface object
 */
BuildTInterface::BuildTInterface()
    : BESXMLInterface( "", &cout )
{
}

BuildTInterface::~BuildTInterface()
{
}

/** @brief this will test the build_request_plan of BESXMLInterface
 *
 * @param requestDoc request XML document
 */
void
BuildTInterface::run( const string &requestDoc )
{
    _dhi->data[DATA_REQUEST] = "xml Document" ;
    _dhi->data["XMLDoc"] = requestDoc ;
    BESXMLInterface::build_data_request_plan() ;
}

/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance
 *
 * @param strm C++ i/o stream to dump the information to
 */
void
BuildTInterface::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "BuildTInterface::dump - ("
			     << (void *)this << ")" << endl ;
    BESIndent::Indent() ;
    BESXMLInterface::dump( strm ) ;
    BESIndent::UnIndent() ;


}

