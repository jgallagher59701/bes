// ContainerStorageCatalog.h

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004,2005 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.org>
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

#ifndef ContainerStorageCatalog_h_
#define ContainerStorageCatalog_h_ 1

#include <map>
#include <string>

using std::map ;
using std::string ;

#include "ContainerStorageVolatile.h"

/** @brief implementation of ContainerStorage that represents a
 * regular expression means of determining a data type.
 *
 * When a container is added to this container storage, the file extension
 * is used to determine the type of data using a set of regular expressions.
 * The regular expressions are retrieved from the opendap initialization
 * file using TheDODSKeys. It also gets the root directory for where the
 * files exist. This way, the user need not know the root directory or the
 * type of data represented by the file.
 *
 * Catalog.&lt;name&gt;.RootDirectory is the key
 * representing the base directory where the files are physically located.
 * The real_name of the container is determined by concatenating the file
 * name to the base directory.
 *
 * Catalog.&lt;name&gt;.TypeMatch is the key
 * representing the regular expressions. This key is formatted as follows:
 *
 * &lt;data type&gt;:&lt;reg exp&gt;;&lt;data type&gt;:&lt;reg exp&gt;;
 *
 * For example: cedar:cedar\/.*\.cbf;cdf:cdf\/.*\.cdf;
 *
 * The first would match anything that might look like: cedar/datfile01.cbf
 *
 * &lt;name&gt; is the name of this container storage, so you could have
 * multiple container stores using regular expressions.
 *
 * The containers are stored in a volatile list.
 *
 * @see ContainerStorage
 * @see DODSContainer
 * @see DODSKeys
 */
class ContainerStorageCatalog : public ContainerStorageVolatile
{
private:
    map< string, string > _match_list ;
    typedef map< string, string >::const_iterator Match_list_citer ;

    string			_root_dir ;

public:
    				ContainerStorageCatalog( const string &n ) ;
    virtual			~ContainerStorageCatalog() ;

    virtual void		add_container( const string &s_name,
                                               const string &r_name,
					       const string &type ) ;
};

#endif // ContainerStorageCatalog_h_

// $Log: ContainerStorageCatalog.h,v $
// Revision 1.7  2005/03/17 19:23:58  pwest
// deleting the container in rem_container instead of returning the removed container, returning true if successfully removed and false otherwise
//
// Revision 1.6  2005/03/15 19:55:36  pwest
// show containers and show definitions
//
// Revision 1.5  2005/02/02 00:03:13  pwest
// ability to replace containers and definitions
//
// Revision 1.4  2005/02/01 17:48:17  pwest
//
// integration of ESG into opendap
//
// Revision 1.3  2004/12/15 17:36:01  pwest
//
// Changed the way in which the parser retrieves container information, going
// instead to ThePersistenceList, which goes through the list of container
// persistence instances it has.
//
// Revision 1.2  2004/09/09 17:17:12  pwest
// Added copywrite information
//
// Revision 1.1  2004/06/30 20:16:24  pwest
// dods dispatch code, can be used for apache modules or simple cgi script
// invocation or opendap daemon. Built during cedar server development.
//