// --------------------------------------------------------------------------
///
/// @file iw_web_srv.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_web_srv.h"

#include "iw_buff.h"
#include "iw_list.h"
#include "iw_log.h"
#include "iw_memory.h"
#include "iw_thread.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// --------------------------------------------------------------------------
//
// Defines
//
// --------------------------------------------------------------------------

/// The default command server buffer size. Used to receive client requests
/// for parsing.
#define BUFF_SIZE   1024

// --------------------------------------------------------------------------
//
// Variables and structures
//
// --------------------------------------------------------------------------

static int s_web_sock = -1;

// --------------------------------------------------------------------------

/// The parse return value
typedef enum _PARSE {
    PARSE_COMPLETE,
    PARSE_INCOMPLETE,
    PARSE_ERROR
} PARSE;

// --------------------------------------------------------------------------

/// The HTTP method used for the request.
typedef enum {
    METHOD_NONE,
    METHOD_GET,
    METHOD_POST
} METHOD;

// --------------------------------------------------------------------------

/// An index into the request for a particular structure.
typedef struct _iw_index {
    const char *start;  ///< The start of the structure.
    int   len;          ///< The length of the structure.
} iw_index;

// --------------------------------------------------------------------------

/// An HTTP request header.
typedef struct _iw_header {
    iw_list_node node;  ///< The list node.
    iw_index     name;  ///< The name of the header.
    iw_index     value; ///< The value of the header.
} iw_header;

// --------------------------------------------------------------------------

/// The HTTP request parse structure.
typedef struct _web_req {
    // ----------------------------------------------------------------------
    //
    // Transient data on the progress of the parsing.
    //
    // ----------------------------------------------------------------------

    /// The request buffer to parse.
    iw_buff buff;

    /// The point of parsing, used for sub-sequent calls to the parse function.
    char   *parse_point;

    // ----------------------------------------------------------------------
    //
    // Information about the parsed request, used to process the request.
    //
    // ----------------------------------------------------------------------

    /// The method of the request.
    METHOD   method;

    /// The protocol version.
    iw_index version;

    /// The URI being requested.
    iw_index uri;

    /// The headers of the request.
    iw_list  headers;

    /// True if all request headers have been parsed.
    bool     headers_complete;

    /// The length of the content of the body (or zero if no body).
    int      content_length;

    /// The content of the request (if any).
    iw_index content;

} web_req;

// --------------------------------------------------------------------------
//
// Helper functions
//
// --------------------------------------------------------------------------

/// @brief Create an HTTP header object.
static iw_header *iw_add_header(
    const char *name,
    int name_len,
    const char *value,
    int value_len)
{
    iw_header *hdr = (iw_header *)IW_CALLOC(1, sizeof(iw_header));
    hdr->name.start  = name;
    hdr->name.len    = name_len;
    hdr->value.start = value;
    hdr->value.len   = value_len;
    return hdr;
}

// --------------------------------------------------------------------------

/// @brief Delete an HTTP header object.
/// @param node The header object to delete.
static void iw_delete_header(iw_list_node *node) {
    iw_header *hdr = (iw_header *)node;
    IW_FREE(hdr);
}

// --------------------------------------------------------------------------

static char *iw_method_str(METHOD method) {
    switch(method) {
    case METHOD_GET  : return "GET";
    case METHOD_POST : return "POST";
    case METHOD_NONE : return "Not Set";
    default : return "UNSUPPORTED";
    }
}

// --------------------------------------------------------------------------

/// @brief Create a response and send it.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully written.
static bool iw_web_srv_construct_response(FILE *out) {
    char *content = "<html><head><title>Response2</title></head><body><h1>Response2</h1></body></html>";
    fprintf(out, "HTTP/1.1 200 Ok\r\n"
                    "Content-Length: %ld\r\n"
                    "\r\n"
                    "%s\r\n",
                    strlen(content),
                    content);

    return true;
}

// --------------------------------------------------------------------------

/// @brief Attempt to parse a client request.
/// @param req The request parse information.
/// @return The parsing result.
static PARSE iw_web_srv_parse_request(web_req *req) {
    char *start;
    int len;

    // Search for the newline signifying the end of the request URI or header.
    char *end = strstr(req->buff.buff, "\r\n");
    if(end == NULL) {
        // Didn't find another line, just return.
        return PARSE_INCOMPLETE;
    }

    if(req->method == METHOD_NONE) {
        // No method set yet, try to parse request line. We should have
        // received a whole line since we passed the test above.
        char *sep = strstr(req->buff.buff, " ");
        if(sep == NULL) {
            // We received a line so we should have a separator.
            return PARSE_ERROR;
        }
        start = req->buff.buff;
        len = sep - start;
        if(strncmp(start, "GET", len) == 0) {
            req->method = METHOD_GET;
        } else if(strncmp(start, "POST", len) == 0) {
            req->method = METHOD_POST;
        } else {
            // Unsupported method
            return PARSE_ERROR;
        }
        req->parse_point = sep + 1;

        // Parse the request URI
        sep = strstr(req->parse_point, " ");
        if(sep == NULL) {
            // We received a line so we should have a separator.
            return PARSE_ERROR;
        }
        req->uri.start = req->parse_point;
        req->uri.len   = sep - req->uri.start;
        req->parse_point = sep + 1;

        // Parse the protocol version
        sep = strstr(req->parse_point, "\r\n");
        if(sep == NULL) {
            // We recieved a line so we should have a separator.
            return PARSE_ERROR;
        }
        req->version.start = req->parse_point;
        req->version.len   = sep - req->version.start;
        req->parse_point = sep + 1;
    }

    // Now try to find headers until we have an empty line.
    while(!req->headers_complete) {
        if(strncmp(req->parse_point, "\r\n", 2) == 0) {
            // An empty line, we've completed parsing the headers.
            req->parse_point += 2;
            req->headers_complete = true;
            break;
        }
        char *sep = strstr(req->parse_point, ":");
        if(sep == NULL) {
            // We've received a line, we should have a proper header.
            return PARSE_ERROR;
        }

        // Create a header index for this header

        // Move to the next line
        req->parse_point = strstr(sep, "\r\n");
    }

    // The presence of an empty line signifies the end of the request header.
    // Either the request is complete, or if a Content-Length header was
    // received with a non-zero value, we know how much data there is in the
    // body.
    if(req->content_length > 0) {
        int buff_len = iw_buff_remainder(&req->buff);
        if(buff_len - (req->parse_point - req->buff.buff) < req->content_length) {
            // We do not have enough data to complete the request
            return PARSE_INCOMPLETE;
        }
    }

    LOG(IW_LOG_IW, "Received %s method, data=\"%s\"",
        iw_method_str(req->method),
        req->buff.buff);

    // Remove the request we just processed.
//    iw_buff_remove_data(&req->buff, len);

    return PARSE_COMPLETE;
}

// --------------------------------------------------------------------------

/// @brief Process a client request.
/// @param fd The client's socket file descriptor.
static void iw_web_srv_process_request(int fd) {
    web_req req;
    memset(&req, 0, sizeof(req));
    if(!iw_buff_create(&req.buff, BUFF_SIZE, 10 * BUFF_SIZE)) {
        LOG(IW_LOG_IW, "Failed to create command server request buffer");
        goto done;
    }
    FILE *out = fdopen(fd, "r+w+");
    int bytes;
    do {
        char *ptr;
        if(!iw_buff_reserve_data(&req.buff, &ptr, BUFF_SIZE)) {
            LOG(IW_LOG_IW, "Failed to allocate command server request buffer");
            goto done;
        }
        bytes = recv(fd, ptr, BUFF_SIZE, 0);
        if(bytes == -1) {
            LOG(IW_LOG_IW, "Request failed (%d:%s)", errno, strerror(errno));
            goto done;
        } else if(bytes == 0) {
            goto done;
        }

        iw_buff_commit_data(&req.buff, bytes);
        PARSE parse = iw_web_srv_parse_request(&req);
        if(parse == PARSE_ERROR) {
            LOG(IW_LOG_IW, "Failed to parse request");
            goto done;
        } if(parse == PARSE_COMPLETE) {
            // Successfully parsed a request. Return from this function.
            goto done;
        }
    } while(bytes > 0);

done:
    iw_web_srv_construct_response(out);

    // Give the client time to close the connection to avoid having the server
    // socket go into a TIME_WAIT state after program termination.
    usleep(100000);
    fclose(out);
    iw_buff_destroy(&req.buff);
    LOG(IW_LOG_IW, "Closed a client connection, fd=%d", fd);
}

// --------------------------------------------------------------------------

/// @brief The main web socket thread entry point.
/// @param param The parameter passed by the thread creator.
/// @return Nothing.
static void *iw_web_srv_thread(void *param) {
    int retval;

    // Entering web server loop.
    LOG(IW_LOG_IW, "Entering web server loop");
    while((retval = accept(s_web_sock, NULL, 0)) != -1) {
        LOG(IW_LOG_IW, "Accepted a client connection, fd=%d", retval);
        iw_web_srv_process_request(retval);
    }

    return NULL;
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

bool iw_web_srv(
    iw_ip *address,
    unsigned short port)
{
    // Open server socket
    iw_ip tmp;
    if(address == NULL) {
        address = &tmp;
        if(!iw_ip_ipv4_to_addr(INADDR_LOOPBACK, address)) {
            LOG(IW_LOG_IW, "Failed to open web server socket.");
            return false;
        }
    }

    if(port == 0) {
        port = 8080;
    }

    if(!iw_ip_set_port(address, port)  ||
       (s_web_sock = iw_ip_open_server_socket(SOCK_STREAM, address, true)) == -1)
    {
        LOG(IW_LOG_IW, "Failed to open web server socket.");
        return false;
    }

    // Create thread to serve the command socket.
    if(!iw_thread_create("Web Server", iw_web_srv_thread, NULL)) {
        LOG(IW_LOG_IW, "Failed to create web server thread");
        close(s_web_sock);
        return false;
    }

    return true;
}

// --------------------------------------------------------------------------
