// BESKeys.cc

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

#ifdef __cplusplus
extern "C" {
#include <sys/types.h>
#include "regex.h"
}
#endif

#include <stdio.h>
#include <unistd.h>
#include <iostream>

using std::endl ;
using std::cout ;

#include "BESKeys.h"
#include "BESKeysException.h"

/** @brief default constructor that reads loads key/value pairs from the
 * specified file.
 *
 * This constructor uses the specified file to load key/value pairs.
 * This file holds different key/value pairs for the application, one
 * key/value pair per line separated by an equal (=) sign.
 *
 * key=value
 *
 * Comments are allowed in the file and must begin with a pound (#) sign at
 * the beginning of the line. No comments are allowed at the end of lines.
 *
 * @throws BESKeysException thrown if there is an error reading the
 * initialization file or a syntax error in the file, i.e. a malformed
 * key/value pair.
 */
BESKeys::BESKeys( const string &keys_file_name )
    : _keys_file( 0 ),
      _keys_file_name( keys_file_name ),
      _the_keys( 0 )
{
    _keys_file = new ifstream( _keys_file_name.c_str() ) ;
    if( !(*_keys_file) )
    {
	char path[500] ;
	getcwd( path, sizeof( path ) ) ;
	string s = string("OPeNDAP: fatal, can not open initialization file ")
		   + _keys_file_name + "\n"
		   + "The current working directory is " + path + "\n" ;
	throw BESKeysException( s ) ;
    }

    _the_keys = new map<string,string>;
    try
    {
	load_keys();
    }
    catch(BESKeysException &ex)
    {
	clean();
	throw;
    }
    catch(...)
    {
	clean() ;
	throw BESKeysException("Undefined exception while trying to load keys");
    }
}

/** @brief cleans up the key/value pair mapping
 */
BESKeys::~BESKeys()
{
    clean() ;
}

void
BESKeys::clean()
{
    if( _keys_file )
    {
	_keys_file->close() ;
	delete _keys_file ;
    }
    if( _the_keys )
    {
	delete _the_keys ;
    }
}

inline void
BESKeys::load_keys()
{
    char buffer[255];
    string key,value;
    while(!(*_keys_file).eof())
    {
	if((*_keys_file).getline(buffer,255))
	{
	    if( break_pair( buffer, key, value ) )
	    {
		(*_the_keys)[key]=value;
	    }
	}
    }
}

inline bool
BESKeys::break_pair(const char* b, string& key, string &value)
{
    if((b[0]!='#') && (!only_blanks(b)))//Ignore comments a lines with only spaces
    {
	register size_t l=strlen(b);
	if(l>1)
	{
	    register int how_many_equals=0;
	    int pos=0;
	    for (register size_t j=0;j<l;j++)
	    {
		if(b[j] == '=')
		{
		    how_many_equals++;
		    pos=j;
		}
	    }

	    if(how_many_equals!=1)
	    {
		char howmany[256] ;
		sprintf( howmany, "%d", how_many_equals ) ;
		string s = string( "OPeNDAP: invalid entry " ) + b
		           + "; there are " + howmany
			   + " = characters.\n";
		throw BESKeysException( s );
	    }
	    else
	    {
		string s=b;
		key=s.substr(0,pos);
		value=s.substr(pos+1,s.size());

		return true;
	    }
	}

	return false;
    }

    return false;
}

bool
BESKeys::only_blanks(const char *line)
{
    int val;
    regex_t rx;
    val=regcomp (&rx, "[^[:space:]]",REG_ICASE);

    if(val!=0)
	throw BESKeysException("Regular expression did not compile correclty");
    val=regexec (&rx,line,0,0, REG_NOTBOL);
    if (val==0)
    {
	regfree(&rx);
	return false;
    }
    else
    {
	if (val==REG_NOMATCH)
	{
	    regfree(&rx);
	    return true;
	}
	else if (val==REG_ESPACE)
	    throw BESKeysException("Regular expression out of space");
	else
	    throw BESKeysException("Regular expression has an undefined problem");
    }
}

/** @brief allows the user to set key/value pairs from within the application.
 *
 * This method allows users of BESKeys to set key/value pairs from within the
 * application, such as for testing purposes, key/value pairs from the command
 * line, etc...
 *
 * If the key is already set then this value replaces the value currently held
 * in the keys map.
 *
 * @param key variable name that can be accessed using the get_key method
 * @param val value of the variable returned when get_key is called for this
 * key
 * @return returns the value of the key, empty string if unsuccessful
 */
string
BESKeys::set_key( const string &key, const string &val )
{
    map< string, string >::iterator i ;
    i = _the_keys->find( key ) ;
    if( i == _the_keys->end() )
    {
	(*_the_keys)[key] = val ;
	return val ;
    }
    else
    {
	(*i).second = val ;
	return val ;
    }
    return "" ;
}

/** @brief allows the user to set key/value pairs from within the application.
 *
 * This method allows users of BESKeys to set key/value pairs from within the
 * application, such as for testing purposes, key/value pairs from the command
 * line, etc...
 *
 * If the key is already set then this value replaces the value currently held
 * in the keys map.
 *
 * @param pair the key/value pair passed as key=value
 * @return returns the value for the key, empty string if unsuccessful
 */
string
BESKeys::set_key( const string &pair )
{
    string key ;
    string val ;
    break_pair( pair.c_str(), key, val ) ;
    return set_key( key, val ) ;
}

/** @brief Retrieve the value of a given key, if set.
 *
 * This method allows the user of BESKeys to retrieve the value of the
 * specified key.
 *
 * @param s The key the user is looking for
 * @param found Set to true of the key is set or false if the key is not set.
 * The value of a key can be set to the empty string, which is why this
 * boolean is provided.
 * @return Returns the value of the key, empty string if the key is not set.
 */
string
BESKeys::get_key( const string& s, bool &found ) 
{
    map<string,string>::iterator i;
    i=_the_keys->find(s);
    if(i!=_the_keys->end())
    {
	found = true ;
	return (*i).second;
    }
    else
    {
	found = false ;
	return "";
    }
}

/** @brief displays all key/value pairs defined to standard output.
 *
 * This method allows the user to see all of the key/value pairs that are
 * currently defined. The output looks like:
 *
 * <PRE>
 * key: "key", value: "value"
 * </PRE>
 */
void
BESKeys::show_keys()
{
    map<string,string>::iterator i ;
    for( i= _the_keys->begin(); i != _the_keys->end(); ++i )
    {
	cout << "key: \"" << (*i).first
	     << "\", value: \"" << (*i).second << "\""
	     << endl ;
    }
}
