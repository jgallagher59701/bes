// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of Hyrax, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2018 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

#ifndef _global_metadata_cache_h
#define _global_metadata_cache_h

#include <string>

#include "BESFileLockingCache.h"

namespace libdap {
class DDS;
class DMR;
}

namespace bes {

/**
 * @brief Cache the results from server functions.
 *
 * @author jhrg
 */

class GlobalMetadataStore: public BESFileLockingCache {
private:
    typedef void (libdap::DDS::*print_method_t)(std::ostream &);

    std::string d_index_entry;  // Built up as info is added, written on success

    static bool d_enabled;
    static GlobalMetadataStore *d_instance;

    // Called by atexit()
    static void delete_instance() {
        delete d_instance;
        d_instance = 0;
    }

    std::string get_hash(const std::string &name);

    bool store_dap2_response(libdap::DDS *dds, print_method_t print_method, const std::string &key);

    // Suppress the automatic generation of these ctors
    GlobalMetadataStore();
    GlobalMetadataStore(const GlobalMetadataStore &src);

    friend class GlobalMetadataStoreTest;

protected:
    /**
     * Protected constructor that calls BESFileLockingCache's constructor;
     * lookup cache directory, item prefix and max cache size in BESKeys
     *
     * @note Use the get_instance() methods to get a pointer to the singleton for
     * this class. Do not use this method except in derived classes. This method
     * either builds a valid object or throws an exception.
     *
     * @param cache_dir key to find cache dir
     * @param prefix key to find the cache prefix
     * @param size key to find the cache size (in MBytes)
     * @throws BESSyntaxUserError if the keys are not set in the BESKeys, if the key
     * are either the empty string or zero, respectively, or if the cache directory does not exist.
     */
    GlobalMetadataStore(const string &cache_dir, const string &prefix, unsigned long long size) :
        BESFileLockingCache(cache_dir, prefix, size)
    {
    }

public:
    static const string PATH_KEY;
    static const string PREFIX_KEY;
    static const string SIZE_KEY;

    static GlobalMetadataStore *get_instance(const string &cache_dir, const string &prefix, unsigned long long size);
    static GlobalMetadataStore *get_instance();

    static string get_cache_dir_from_config();
    static string get_cache_prefix_from_config();
    static unsigned long get_cache_size_from_config();

    virtual ~GlobalMetadataStore()
    {
    }

    virtual bool add_object(libdap::DDS *dds, const std::string &name);

#if 0
    virtual std::string get_dds_response(const std::string &name) { }
    virtual std::string get_das_response(const std::string &name) { }

    virtual bool add_object(libdap::DMR *dmr, const std::string &name) { }

    virtual std::string get_dmr_response(const std::string &name) { }

    // These 'get' methods return null or the empty string if the thing is not in the store.
    virtual libdap::DDS *get_dds_object(const std::string &name) { }
    virtual libdap::DMR *get_dmr_object(const std::string &name) { }
#endif

};

} // namespace bes

#endif // _global_metadata_cache_h
