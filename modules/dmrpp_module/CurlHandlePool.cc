// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of the BES

// Copyright (c) 2018 OPeNDAP, Inc.
// Author: James Gallagher<jgallagher@opendap.org>
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

#include "config.h"

#include <string>
#include <sstream>

#include <curl/curl.h>

#ifdef HAVE_CURL_MULTI_h
#include <curl/multi.h>
#endif

#include "util.h"   // long_to_string()

#include "BESDebug.h"
#include "BESInternalError.h"
#include "BESForbiddenError.h"
#include "WhiteList.h"

#include "DmrppRequestHandler.h"
#include "CurlHandlePool.h"
#include "Chunk.h"

#define MAX_WAIT_MSECS 30*1000 /* Wait max. 30 seconds */

using namespace dmrpp;
using namespace std;
using namespace bes;

Lock::Lock(pthread_mutex_t &lock) : m_mutex(lock)
 {
     int status = pthread_mutex_lock(&m_mutex);
     if (status != 0) throw BESInternalError("Could not lock in CurlHandlePool", __FILE__, __LINE__);
 }

Lock::~Lock()
 {
     int status = pthread_mutex_unlock(&m_mutex);
     if (status != 0) throw BESInternalError("Could not unlock in CurlHandlePool", __FILE__, __LINE__);
 }

dmrpp_easy_handle::dmrpp_easy_handle()
{
    d_handle = curl_easy_init();
    if (!d_handle) throw BESInternalError("Could not allocate CURL handle", __FILE__, __LINE__);

    CURLcode res;

    // Pass all data to the 'write_data' function
    if (CURLE_OK != (res = curl_easy_setopt(d_handle, CURLOPT_WRITEFUNCTION, chunk_write_data)))
        throw BESInternalError(string("CURL Error: ").append(curl_easy_strerror(res)), __FILE__, __LINE__);

    /* enable TCP keep-alive for this transfer */
    if (CURLE_OK != (res = curl_easy_setopt(d_handle, CURLOPT_TCP_KEEPALIVE, 1L)))
        throw string("CURL Error: ").append(curl_easy_strerror(res));

    /* keep-alive idle time to 120 seconds */
    if (CURLE_OK != (res = curl_easy_setopt(d_handle, CURLOPT_TCP_KEEPIDLE, 120L)))
        throw string("CURL Error: ").append(curl_easy_strerror(res));

    /* interval time between keep-alive probes: 60 seconds */
    if (CURLE_OK != (res = curl_easy_setopt(d_handle, CURLOPT_TCP_KEEPINTVL, 60L)))
        throw string("CURL Error: ").append(curl_easy_strerror(res));

    d_in_use = false;
    d_url = "";
    d_chunk = 0;
}

dmrpp_easy_handle::~dmrpp_easy_handle()
{
    curl_easy_cleanup(d_handle);
}

/**
 * Return the HTTP/S status code if the request succeeded; throw an exception
 * on error.
 *
 * @param eh The CURL easy_handle
 */
static void evaluate_curl_response(CURL* eh)
{
    long http_code = 0;
    CURLcode res = curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &http_code);
    if (CURLE_OK != res) {
        throw BESInternalError(string("Error getting HTTP response code: ").append(curl_easy_strerror(res)), __FILE__, __LINE__);
    }

    // Newer Apache servers return 206 for range requests. jhrg 8/8/18
    switch (http_code) {
    case 200: // OK
    case 206: // Partial content - this is to be expected since we use range gets
        // cases 201-205 are things we should probably reject, unless we add more
        // comprehensive HTTP/S processing here. jhrg 8/8/18
        break;

    default: {
        ostringstream oss;
        oss << "HTTP status error: Expected an OK status, but got: ";
        oss << http_code;
        throw BESInternalError(oss.str(), __FILE__, __LINE__);
        break;
    }
    }
}

#ifndef HAVE_CURL_MULTI
static void *easy_handle_read_data(void *handle)
{
    dmrpp_easy_handle *eh = reinterpret_cast<dmrpp_easy_handle*>(handle);

    try {
        eh->read_data();
        pthread_exit(NULL);
    }
    catch (BESError &e) {
        string *error = new string(e.get_message().append(": ").append(e.get_file())
            .append(":").append(libdap::long_to_string(e.get_line())));
        pthread_exit(error);
    }
}
#endif

/**
 * @brief This is the read_data() method for serial transfers
 */
void dmrpp_easy_handle::read_data()
{
    CURL *curl = d_handle;

    // Perform the request
    CURLcode curl_code = curl_easy_perform(curl);
    if (CURLE_OK != curl_code) {
        throw BESInternalError(string("Data transfer error: ").append(curl_easy_strerror(curl_code)), __FILE__, __LINE__);
    }

    // For HTTP, check the return code, for the file protocol, if curl_code is OK, that's good enough
    if (d_url.find("https://") == 0 || d_url.find("http://") == 0) {
        evaluate_curl_response(curl);
    }

    d_chunk->set_is_read(true);
}

/**
 * @brief The read_data() method for parallel transfers
 *
 * This uses either the CURL Multi API or pthreads to read N
 * dmrpp_easy_handle instances in parallel.
 */
void dmrpp_multi_handle::read_data()
{
#ifdef HAVE_CURL_MULTI
    int still_running = 0;
    CURLMcode mres = curl_multi_perform(d_multi, &still_running);
    if (mres != CURLM_OK)
        throw BESInternalError(string("Could not initiate data read: ").append(curl_multi_strerror(mres)), __FILE__,
            __LINE__);

    do {
        int numfds = 0;
        mres = curl_multi_wait(d_multi, NULL, 0, MAX_WAIT_MSECS, &numfds);
        if (mres != CURLM_OK)
            throw BESInternalError(string("Could not wait on data read: ").append(curl_multi_strerror(mres)), __FILE__,
                __LINE__);

        mres = curl_multi_perform(d_multi, &still_running);
        if (mres != CURLM_OK)
            throw BESInternalError(string("Could not iterate data read: ").append(curl_multi_strerror(mres)), __FILE__,
                __LINE__);

    } while (still_running);

    CURLMsg *msg = 0;
    int msgs_left = 0;
    while ((msg = curl_multi_info_read(d_multi, &msgs_left))) {
        if (msg->msg == CURLMSG_DONE) {
            CURL *eh = msg->easy_handle;

            CURLcode res = msg->data.result;
            if (res != CURLE_OK)
                throw BESInternalError(string("Error HTTP: ").append(curl_easy_strerror(res)), __FILE__, __LINE__);

            // Note: 'eh' is the easy handle returned by culr_multi_info_read(),
            // but in it's private field is our dmrpp_easy_handle object. We need
            // both to mark this data read operation as complete.
            dmrpp_easy_handle *dmrpp_easy_handle = 0;
            res = curl_easy_getinfo(eh, CURLINFO_PRIVATE, &dmrpp_easy_handle);
            if (res != CURLE_OK)
                throw BESInternalError(string("Could not access easy handle: ").append(curl_easy_strerror(res)), __FILE__, __LINE__);

            // This code has to work with both http/s: and file: protocols. Here we check the
            // HTTP status code. If the protocol is not HTTP, we assume since msg->data.result
            // returned CURLE_OK, that the transfer worked. jhrg 5/1/18
            if (dmrpp_easy_handle->d_url.find("http://") == 0 || dmrpp_easy_handle->d_url.find("https://") == 0) {
                evaluate_curl_response(eh);
            }

            // If we are here, the request was successful.

            dmrpp_easy_handle->d_chunk->set_is_read(true);  // Set the is_read() property for chunk here.
            DmrppRequestHandler::curl_handle_pool->release_handle(dmrpp_easy_handle);

            mres = curl_multi_remove_handle(d_multi, eh);
            if (mres != CURLM_OK)
                throw BESInternalError(string("Could not remove libcurl handle: ").append(curl_multi_strerror(mres)),  __FILE__, __LINE__);
        }
        else {  // != CURLMSG_DONE
            throw BESInternalError("Error getting HTTP or FILE responses.", __FILE__, __LINE__);
        }
    }
#else
    // Note: It's the responsibility of the caller to make sure that no more than
    // d_max_parallel_transfers are added to the 'multi' handle.

    // Start the processing pipelines
    pthread_t thread[d_multi.size()];
    unsigned int threads = 0;
    for (unsigned int i = 0; i < d_multi.size(); ++i) {
        int status = pthread_create(&thread[i], NULL, easy_handle_read_data, (void*) d_multi[i]);
        if (status == 0) {
            ++threads;
        }
        else {
            ostringstream oss("Could not start process_one_chunk_unconstrained thread for chunk ");
            oss << i << ": " << strerror(status);
            throw BESInternalError(oss.str(), __FILE__, __LINE__);
        }
    }

    // Now join the child threads.
    for (unsigned int i = 0; i < threads; ++i) {
        string *error;
        int status = pthread_join(thread[i], (void**) &error);
        if (status != 0) {
            ostringstream oss("Could not join process_one_chunk_unconstrained thread for chunk ");
            oss << i << ": " << strerror(status);
            throw BESInternalError(oss.str(), __FILE__, __LINE__);
        }
        else if (error != 0) {
            BESInternalError e(*error, __FILE__, __LINE__);
            delete error;
            throw e;
        }
    }

    // Now remove the easy_handles, mimicking the behavior when using the real Multi API
    d_multi.clear();
#endif
}

CurlHandlePool::CurlHandlePool() : d_multi_handle(0)
{
    d_max_easy_handles = DmrppRequestHandler::d_max_parallel_transfers;
    d_multi_handle = new dmrpp_multi_handle();

    for (unsigned int i = 0; i < d_max_easy_handles; ++i) {
        d_easy_handles.push_back(new dmrpp_easy_handle());
    }

    if (pthread_mutex_init(&d_get_easy_handle_mutex, 0) != 0)
        throw BESInternalError("Could not initialize mutex in CurlHandlePool", __FILE__, __LINE__);
}

/**
 * Get a CURL easy handle to transfer data from \arg url into the given \arg chunk.
 *
 * @note This method and release_handle() use the same lock to prevent the handle's
 * chunk pointer from being cleared by another thread after a thread running this
 * method has set it. However, there's no protection against calling this when no
 * more handles are available. If that happens a thread calling release_handle()
 * will block until this code returns (and this code will return NULL).
 *
 * @param chunk Use this Chunk to set a libcurl easy handle so that it
 * will fetch the Chunk's data.
 * @return A CURL easy handle configured to transfer data, or null if
 * there are no more handles in the pool.
 */
dmrpp_easy_handle *
CurlHandlePool::get_easy_handle(Chunk *chunk)
{
    Lock lock(d_get_easy_handle_mutex);

    dmrpp_easy_handle *handle = 0;
    for (vector<dmrpp_easy_handle *>::iterator i = d_easy_handles.begin(), e = d_easy_handles.end(); i != e; ++i) {
        if (!(*i)->d_in_use)
            handle = *i;
    }

    if (handle) {
        // Once here, d_easy_handle holds a CURL* we can use.
        handle->d_in_use = true;
        handle->d_url = chunk->get_data_url();

        // Here we check to make sure that the we are only going to
        // access an approved location with this easy_handle
        if(!WhiteList::get_white_list()->is_white_listed(handle->d_url)){
            string msg = "ERROR!! The chunk url " + handle->d_url + " does not match any white-list rule. ";
            throw BESForbiddenError(msg ,__FILE__,__LINE__);
        }

        handle->d_chunk = chunk;

        CURLcode res = curl_easy_setopt(handle->d_handle, CURLOPT_URL, chunk->get_data_url().c_str());
        if (res != CURLE_OK) throw BESInternalError(string(curl_easy_strerror(res)), __FILE__, __LINE__);

        // get the offset to offset + size bytes
        if (CURLE_OK != (res = curl_easy_setopt(handle->d_handle, CURLOPT_RANGE, chunk->get_curl_range_arg_string().c_str())))
            throw BESInternalError(string("HTTP Error setting Range: ").append(curl_easy_strerror(res)), __FILE__,
            __LINE__);

        // Pass this to write_data as the fourth argument
        if (CURLE_OK != (res = curl_easy_setopt(handle->d_handle, CURLOPT_WRITEDATA, reinterpret_cast<void*>(chunk))))
            throw BESInternalError(string("CURL Error setting chunk as data buffer: ").append(curl_easy_strerror(res)),
            __FILE__, __LINE__);

        // store the easy_handle so that we can call release_handle in multi_handle::read_data()
        if (CURLE_OK != (res = curl_easy_setopt(handle->d_handle, CURLOPT_PRIVATE, reinterpret_cast<void*>(handle))))
            throw BESInternalError(string("CURL Error setting easy_handle as private data: ").append(curl_easy_strerror(res)), __FILE__,
            __LINE__);
    }

    return handle;
}

/**
 * Release a DMR++ easy_handle. This returns the handle to the pool of handles
 * that can be used for serial transfers or, with multi curl, for parallel transfers.
 *
 * @param handle
 */
void CurlHandlePool::release_handle(dmrpp_easy_handle *handle)
{
    // In get_easy_handle, it's possible that d_in_use could be false and d_chunk
    // could not be set to 0 (because a separate thread could be running these
    // methods). In that case, the thread running get_easy_handle could set d_chunk,
    // and then this thread could clear it (... unlikely, but an optimizing compiler is
    // free to reorder statements so long as they don't alter the function's behavior).
    // Timing tests indicate this lock does not cost anything that can be measured.
    // jhrg 8/21/18
    Lock lock(d_get_easy_handle_mutex);

    handle->d_url = "";
    handle->d_chunk = 0;
    handle->d_in_use = false;
}
