// --------------------------------------------------------------------------
///
/// @file iw_web_srv.h
///
/// A web server for handling HTTP requests.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_WEB_SRV_H_
#define _IW_WEB_SRV_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_buff.h"
#include "iw_ip.h"
#include "iw_list.h"
#include "iw_main.h"
#include "iw_web_req.h"

#include <stdbool.h>
#include <stdio.h>

// --------------------------------------------------------------------------

/// @brief A callback function for web requests.
/// Called when an HTTP request has been parsed and should be responded to.
/// @param req The request that was made.
/// @param out The file stream to write the response to.
typedef bool (*IW_WEB_REQ_FN)(iw_web_req *req, FILE *out);

// --------------------------------------------------------------------------

/// @brief The web server object.
/// Used to represent a web server.
typedef struct _iw_web_srv {
    int fd;                     ///< The file descriptor for the server socket.

    IW_WEB_REQ_FN callback;     ///< The callback function for requests.
} iw_web_srv;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Called to create a web server to process client requests.
/// @param address The address to bind to or NULL for local host.
/// @param port The port number to use to serve client requests. If the port
///        is set to zero, the default port of 8080 will be used.
/// @param callback The callback function for handling server requests.
/// @return The web server object for the new web server or NULL for failure.
extern iw_web_srv *iw_web_srv_init(
    iw_ip *address,
    unsigned short port,
    IW_WEB_REQ_FN callback);

// --------------------------------------------------------------------------

/// @brief Called to terminate a web server.
/// All allocated memory for the server will be freed.
/// @param srv The server to terminate.
extern void iw_web_srv_exit(iw_web_srv *srv);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_WEB_SRV_H_

// --------------------------------------------------------------------------
