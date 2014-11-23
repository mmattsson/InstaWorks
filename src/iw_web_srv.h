// --------------------------------------------------------------------------
///
/// @file iw_web_srv.h
///
/// A web server to process client requests. This allows a process to
/// provide a web based query and configuration interface.
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

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Typedefs
//
// --------------------------------------------------------------------------

/// The parse return value
typedef enum _PARSE {
    IW_WEB_PARSE_COMPLETE,
    IW_WEB_PARSE_INCOMPLETE,
    IW_WEB_PARSE_ERROR
} IW_WEB_PARSE;

// --------------------------------------------------------------------------

/// The HTTP method used for the request.
typedef enum {
    IW_WEB_METHOD_NONE,
    IW_WEB_METHOD_GET,
    IW_WEB_METHOD_POST
} IW_WEB_METHOD;

// --------------------------------------------------------------------------

/// An index into the request for a particular structure.
typedef struct _iw_parse_index {
    const char *start;  ///< The start of the structure.
    int   len;          ///< The length of the structure.
} iw_parse_index;

// --------------------------------------------------------------------------

/// An HTTP request header.
typedef struct _iw_web_req_header {
    iw_list_node   node;  ///< The list node.
    iw_parse_index name;  ///< The name of the header.
    iw_parse_index value; ///< The value of the header.
} iw_web_req_header;

// --------------------------------------------------------------------------

/// The HTTP request parse structure.
typedef struct _iw_web_req {
    // ----------------------------------------------------------------------
    //
    // Transient data on the progress of the parsing.
    //
    // ----------------------------------------------------------------------

    /// The point of parsing, used for sub-sequent calls to the parse function.
    unsigned int parse_point;

    // ----------------------------------------------------------------------
    //
    // Information about the parsed request, used to process the request.
    //
    // ----------------------------------------------------------------------

    /// The method of the request.
    IW_WEB_METHOD   method;

    /// The protocol version.
    iw_parse_index version;

    /// The URI being requested.
    iw_parse_index uri;

    /// The headers of the request.
    iw_list  headers;

    /// True if all request headers have been parsed.
    bool     headers_complete;

    /// The length of the content of the body (or zero if no body).
    int      content_length;

    /// The content of the request (if any).
    iw_parse_index content;

} iw_web_req;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Free all memory allocated by a web request.
/// This does not free the request pointer itself, only the memory allocated
/// as part of parsing the request.
/// @param req The request to free.
extern void iw_web_srv_free_request(iw_web_req *req);

// --------------------------------------------------------------------------

/// @brief Attempt to parse a client request.
/// @param str The string to parse.
/// @param req The request parse information.
/// @return The parsing result.
extern IW_WEB_PARSE iw_web_srv_parse_request_str(
    const char *str,
    iw_web_req *req);

// --------------------------------------------------------------------------

/// @brief Attempt to parse a client request.
/// This function may be called multiple times. The \a req parameter must be
/// initialized for the first call.
/// @param buff The buffer to parse.
/// @param req The request parse information.
/// @return The parsing result.
extern IW_WEB_PARSE iw_web_srv_parse_request(
    const iw_buff *buff,
    iw_web_req *req);

// --------------------------------------------------------------------------

/// @brief Called to create a web server to process client requests.
/// @param address The address to bind to or NULL for local host.
/// @param port The port number to use to serve client requests. If the port
///        is set to zero, the default port of 8080 will be used.
extern bool iw_web_srv(
    iw_ip *address,
    unsigned short port);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_WEB_SRV_H_

// --------------------------------------------------------------------------
