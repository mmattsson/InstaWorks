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
//
// Helper functions
//
// --------------------------------------------------------------------------

/// @brief Create an HTTP header object.
static iw_web_req_header *iw_add_header(
    iw_web_req *req,
    const char *name,
    int name_len,
    const char *value,
    int value_len)
{
    iw_web_req_header *hdr =
            (iw_web_req_header *)IW_CALLOC(1, sizeof(iw_web_req_header));
    if(hdr == NULL) {
        return NULL;
    }
    hdr->name.start  = name;
    hdr->name.len    = name_len;
    hdr->value.start = value;
    hdr->value.len   = value_len;
    iw_list_add(&req->headers, (iw_list_node *)hdr);
    return hdr;
}

// --------------------------------------------------------------------------

/// @brief Delete an HTTP header object.
/// @param node The header object to delete.
static void iw_delete_header(iw_list_node *node) {
    iw_web_req_header *hdr = (iw_web_req_header *)node;
    IW_FREE(hdr);
}

// --------------------------------------------------------------------------

static char *iw_method_str(IW_WEB_METHOD method) {
    switch(method) {
    case IW_WEB_METHOD_GET  : return "GET";
    case IW_WEB_METHOD_POST : return "POST";
    case IW_WEB_METHOD_NONE : return "Not Set";
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
                    "Content-Length: %d\r\n"
                    "\r\n"
                    "%s\r\n",
                    strlen(content),
                    content);

    return true;
}

// --------------------------------------------------------------------------


/// @brief Process a client request.
/// @param fd The client's socket file descriptor.
static void iw_web_srv_process_request(int fd) {
    iw_web_req req;
    iw_buff buff;
    memset(&req, 0, sizeof(req));
    if(!iw_buff_create(&buff, BUFF_SIZE, 10 * BUFF_SIZE)) {
        LOG(IW_LOG_IW, "Failed to create command server request buffer");
        goto done;
    }
    FILE *out = fdopen(fd, "r+w+");
    int bytes;
    do {
        char *ptr;
        if(!iw_buff_reserve_data(&buff, &ptr, BUFF_SIZE)) {
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

        iw_buff_commit_data(&buff, bytes);
        IW_WEB_PARSE parse = iw_web_srv_parse_request(&buff, &req);
        if(parse == IW_WEB_PARSE_ERROR) {
            LOG(IW_LOG_IW, "Failed to parse request");
            goto done;
        } if(parse == IW_WEB_PARSE_COMPLETE) {
            // Successfully parsed a request. Return from this function.
            goto done;
        }
    } while(bytes > 0);

done:
    iw_web_srv_construct_response(out);

    iw_web_srv_free_request(&req);

    // Give the client time to close the connection to avoid having the server
    // socket go into a TIME_WAIT state after program termination.
    usleep(100000);
    fclose(out);
    iw_buff_destroy(&buff);
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

void iw_web_srv_free_request(iw_web_req *req) {
    iw_list_destroy(&req->headers, iw_delete_header);
}

// --------------------------------------------------------------------------

IW_WEB_PARSE iw_web_srv_parse_request_str(const char *str, iw_web_req *req) {
    // We cheat here and cast the string to non-const since the string member
    // of the buffer is not const. It can't be const because other uses of the
    // buffer may not be const, but we know that this use is indeed const.
    iw_buff buff;
    buff.buff = (char *)str;
    buff.end = buff.size = buff.max_size = strlen(str);
    memset(req, 0, sizeof(*req));
    return iw_web_srv_parse_request(&buff, req);
}

// --------------------------------------------------------------------------

IW_WEB_PARSE iw_web_srv_parse_request(const iw_buff *buff, iw_web_req *req) {
    char *start;
    int len;

    // Search for the newline signifying the end of the request URI or header.
    char *end = strstr(buff->buff, "\r\n");
    if(end == NULL) {
        // Didn't find another line, just return.
        return IW_WEB_PARSE_INCOMPLETE;
    }

    if(req->method == IW_WEB_METHOD_NONE) {
        // No method set yet, try to parse request line. We should have
        // received a whole line since we passed the test above.
        char *sep = strstr(buff->buff, " ");
        if(sep == NULL) {
            // We received a line so we should have a separator.
            return IW_WEB_PARSE_ERROR;
        }
        start = buff->buff;
        len = sep - start;
        if(strncmp(start, "GET", len) == 0) {
            req->method = IW_WEB_METHOD_GET;
        } else if(strncmp(start, "POST", len) == 0) {
            req->method = IW_WEB_METHOD_POST;
        } else {
            // Unsupported method
            return IW_WEB_PARSE_ERROR;
        }
        req->parse_point = sep - buff->buff + 1;

        // Parse the request URI
        sep = strstr(buff->buff + req->parse_point, " ");
        if(sep == NULL) {
            // We received a line so we should have a separator.
            return IW_WEB_PARSE_ERROR;
        }
        req->uri.start = buff->buff + req->parse_point;
        req->uri.len   = sep - req->uri.start;
        req->parse_point = sep + 1 - buff->buff;

        // Parse the protocol version
        sep = strstr(buff->buff + req->parse_point, "\r\n");
        if(sep == NULL) {
            // We recieved a line so we should have a separator.
            return IW_WEB_PARSE_ERROR;
        }
        req->version.start = req->parse_point + buff->buff;
        req->version.len   = sep - req->version.start;
        req->parse_point = sep + 2 - buff->buff;
    }

    // Now try to find headers until we have an empty line.
    while(!req->headers_complete) {
        if(strncmp(buff->buff + req->parse_point, "\r\n", 2) == 0) {
            // An empty line, we've completed parsing the headers.
            req->parse_point += 2;
            req->headers_complete = true;
            break;
        }
        char *sep = strstr(buff->buff + req->parse_point, ":");
        if(sep == NULL) {
            // We've received a line, we should have a proper header.
            return IW_WEB_PARSE_ERROR;
        }

        // Move to the next line
        end = strstr(sep, "\r\n");

        // Create a header index for this header
        iw_add_header(req,
                      buff->buff + req->parse_point,
                      sep - buff->buff - req->parse_point,
                      sep + 1, end - sep - 1);

        req->parse_point = end - buff->buff + 2;
    }

    // The presence of an empty line signifies the end of the request header.
    // Either the request is complete, or if a Content-Length header was
    // received with a non-zero value, we know how much data there is in the
    // body.
    if(req->content_length > 0) {
        int buff_len = iw_buff_remainder(buff);
        if(buff_len - req->parse_point < req->content_length) {
            // We do not have enough data to complete the request
            return IW_WEB_PARSE_INCOMPLETE;
        }
    }

    // Debug log the request we just received
    LOG(IW_LOG_IW, "Received %s method, data=\"%s\"",
        iw_method_str(req->method),
        buff->buff);
    iw_list_node *node = req->headers.head;
    while(node != NULL) {
        iw_web_req_header *hdr = (iw_web_req_header *)node;
        node = node->next;
        LOG(IW_LOG_IW, "HDR: \"%.*s\" -> \"%.*s\"",
            hdr->name.len, hdr->name.start,
            hdr->value.len, hdr->value.start);
    }

    // Remove the request we just processed.
//    iw_buff_remove_data(&req->buff, len);

    return IW_WEB_PARSE_COMPLETE;
}

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
