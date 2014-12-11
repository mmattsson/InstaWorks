// --------------------------------------------------------------------------
///
/// @file iw_web_req.h
///
/// Module for processing web requests.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_WEB_REQ_H_
#define _IW_WEB_REQ_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_buff.h"
#include "iw_ip.h"
#include "iw_list.h"
#include "iw_main.h"
#include "iw_parse.h"

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Typedefs
//
// --------------------------------------------------------------------------

/// The HTTP method used for the request.
typedef enum {
    IW_WEB_METHOD_NONE,
    IW_WEB_METHOD_GET,
    IW_WEB_METHOD_POST
} IW_WEB_METHOD;

// --------------------------------------------------------------------------

/// The parse return value
typedef enum _IW_WEB_PARSE {
    IW_WEB_PARSE_COMPLETE,
    IW_WEB_PARSE_INCOMPLETE,
    IW_WEB_PARSE_ERROR
} IW_WEB_PARSE;

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

    /// True if the request has been successfully parsed.
    bool complete;

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

/// @brief Initialize a web request object.
/// @param req The request to initialize.
extern void iw_web_req_init(iw_web_req *req);

// --------------------------------------------------------------------------

/// @brief Create an HTTP header object.
/// Adds an HTTP header to the request object. The header object points into
/// the HTTP request memory to track where the header name and value is. It
/// does not allocate new memory to represent the header and the values pointed
/// to by the header object should therefore not be free'd. Also, since the
/// memory pointed to by the header object is part of the original HTTP request
/// and will not be altered, the values pointed to are not NUL terminated and
/// only the given length bytes for the name or the value should be used.
/// @param req The request to add the header to.
/// @param name The name of the header.
/// @param value The value of the header.
extern iw_web_req_header *iw_web_req_add_header(
    iw_web_req *req,
    iw_parse_index *name,
    iw_parse_index *value);

// --------------------------------------------------------------------------

/// @brief Delete a header object.
/// This deletes the header object memory allocatedd to represent the header.
/// It does not delete the memory pointed to by the header object.
/// @param node The header node to delete.
extern void iw_web_req_delete_header(iw_list_node *node);

// --------------------------------------------------------------------------

/// @brief Attempt to parse a client request.
/// The string being parsed does not have to be NUL-terminated.
/// @param str The string to parse.
/// @param len The length of the string.
/// @param req The request parse information.
/// @return The parsing result.
extern IW_WEB_PARSE iw_web_req_parse(
    const char *str,
    unsigned int len,
    iw_web_req *req);

// --------------------------------------------------------------------------

/// @brief Free all memory allocated by a web request.
/// This does not free the request pointer itself, only the memory allocated
/// as part of parsing the request.
/// @param req The request to free.
extern void iw_web_req_free(iw_web_req *req);

// --------------------------------------------------------------------------
//
// Attribute access API
//
// --------------------------------------------------------------------------

/// @brief Get the method string for the given method.
/// @param method The method to return the string for.
/// @return The string representing this method.
extern char *iw_web_req_method_str(IW_WEB_METHOD method);

// --------------------------------------------------------------------------

/// @brief Get the method of a request.
/// @param req The request to get the method for.
/// @return The method for the request.
extern IW_WEB_METHOD iw_web_req_get_method(const iw_web_req *req);

// --------------------------------------------------------------------------

/// @brief Get the first header with the given name.
/// If no header matches the given name, NULL is returned. If several headers
/// match, the first header matching the name is returned. If no name is given,
/// The first header in the request is returned.
/// @param buff The buffer of the request.
/// @param req The request to find the header in.
/// @param name The name of the header to return or NULL for the first header.
/// @return The request header or NULL for no match.
extern iw_web_req_header *iw_web_req_get_header(
    const char *buff,
    iw_web_req *req,
    const char *name);

// --------------------------------------------------------------------------

/// @brief Get the next header with the given name.
/// Returns the next header with the given name or NULL if no more matches
/// were found.
/// @param buff The buffer of the request.
/// @param req The request to find the header in.
/// @param name The name of the header to return or NULL for all headers.
/// @param hdr The previously found header.
/// @return The next matching header or NULL for no more matches.
extern iw_web_req_header *iw_web_req_get_next_header(
    const char *buff,
    iw_web_req *req,
    const char *name,
    const iw_web_req_header *hdr);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_WEB_REQ_H_

// --------------------------------------------------------------------------
