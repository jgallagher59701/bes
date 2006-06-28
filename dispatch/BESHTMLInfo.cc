// BESHTMLInfo.cc

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

#ifdef __GNUG__
#pragma implementation
#endif

#include <sstream>

using std::ostringstream ;

#include "BESHTMLInfo.h"

/** @brief constructs an html information response object.
 *
 * Uses the default OPeNDAP.Info.Buffered key in the dods initialization file to
 * determine whether the information should be buffered or not.
 *
 * @see BESInfo
 * @see DODSResponseObject
 */
BESHTMLInfo::BESHTMLInfo( const string &buffer_key, ObjectType otype)
    : BESInfo( buffer_key, otype )
{
}

/** @brief constructs an html information response object.
 *
 * Uses the default OPeNDAP.Info.Buffered key in the dods initialization file to
 * determine whether the information should be buffered or not.
 *
 * @param is_http whether the response is going to a browser
 * @see BESInfo
 * @see DODSResponseObject
 */
BESHTMLInfo::BESHTMLInfo( bool is_http,
                            const string &buffer_key,
			    ObjectType otype )
    : BESInfo( is_http, buffer_key, otype )
{
}

BESHTMLInfo::~BESHTMLInfo()
{
}

/** @brief add data to this informational object. If buffering is not set then
 * the information is output directly to the output stream.
 *
 * @param s information to be added to this response object
 */
void
BESHTMLInfo::add_data( const string &s )
{
    if( !_header && !_buffered && _is_http )
    {
	set_mime_html( stdout, _otype ) ;
	_header = true ;
    }
    BESInfo::add_data( s ) ;
}

/** @brief add exception data to this informational object. If buffering is
 * not set then the information is output directly to the output stream.
 *
 * @param type type of the exception received
 * @param msg the error message
 * @param file file name of where the error was sent
 * @param line line number in the file where the error was sent
 */
void
BESHTMLInfo::add_exception( const string &type, const string &msg,
                             const string &file, int line )
{
    add_data( "Exception Received<BR />\n" ) ;
    add_data( (string)"    Type: " + type + "<BR />\n" ) ;
    add_data( (string)"    Message: " + msg + "<BR />\n" ) ;
    ostringstream s ;
    s << "    Filename: " << file << " LineNumber: " << line << "<BR />\n" ;
    add_data( s.str() ) ;
}

// $Log: BESHTMLInfo.cc,v $
// Revision 1.4  2005/04/19 17:58:52  pwest
// print of an html information object must include the header
//
// Revision 1.3  2004/12/15 17:39:03  pwest
// Added doxygen comments
//
// Revision 1.2  2004/09/09 17:17:12  pwest
// Added copywrite information
//
// Revision 1.1  2004/06/30 20:16:24  pwest
// dods dispatch code, can be used for apache modules or simple cgi script
// invocation or opendap daemon. Built during cedar server development.
//