// --------------------------------------------------------------------------
///
/// @file iw_web_req.c
///
/// Module for parsing and handling HTTP requests.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_web_req.h"

#include "iw_log.h"
#include "iw_memory.h"
#include "iw_thread.h"
#include "iw_web_srv.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

iw_web_req_header *iw_web_req_add_header(
    iw_web_req *req,
    iw_parse_index *name,
    iw_parse_index *value)
{
    iw_web_req_header *hdr =
            (iw_web_req_header *)IW_CALLOC(1, sizeof(iw_web_req_header));
    if(hdr == NULL) {
        return NULL;
    }
    hdr->name  = *name;
    hdr->value = *value;
    iw_list_add(&req->headers, (iw_list_node *)hdr);

    return hdr;
}

// --------------------------------------------------------------------------

void iw_web_req_delete_header(iw_list_node *node) {
    iw_web_req_header *hdr = (iw_web_req_header *)node;
    IW_FREE(hdr);
}

// --------------------------------------------------------------------------

void iw_web_req_free(iw_web_req *req) {
    iw_list_destroy(&req->headers, iw_web_req_delete_header);
}

// --------------------------------------------------------------------------

IW_WEB_PARSE iw_web_req_parse_str(const char *str, iw_web_req *req) {
    // We cheat here and cast the string to non-const since the string member
    // of the buffer is not const. It can't be const because other uses of the
    // buffer may not be const, but we know that this use is indeed const.
    iw_buff buff;
    buff.buff = (char *)str;
    buff.end = buff.size = buff.max_size = strlen(str);
    memset(req, 0, sizeof(*req));
    return iw_web_req_parse(&buff, req);
}

// --------------------------------------------------------------------------
// Todo: need to rewrite this to not take a buffer. The parse method should
// take a pointer to a string only. The caller may have the string in a buffer
// and can remove the successfully parsed bytes from the buffer at completion.
IW_WEB_PARSE iw_web_req_parse(const iw_buff *buff, iw_web_req *req) {
    IW_PARSE parse;

    // Search for the newline signifying the end of the request URI or header.
    char *end = strstr(buff->buff, IW_PARSE_CRLF);
    if(end == NULL) {
        // Didn't find another line, just return.
        return IW_WEB_PARSE_INCOMPLETE;
    }

    if(req->method == IW_WEB_METHOD_NONE) {
        // No method set yet, try to parse request line. We should have
        // received a whole line since we passed the test above.
        iw_parse_index idx;
        parse = iw_parse_read_token(buff->buff, &req->parse_point,
                                    IW_PARSE_SPACE, true, &idx);
        if(parse != IW_PARSE_MATCH) {
            // We received a line so we should have a separator.
            return IW_WEB_PARSE_ERROR;
        }
        if(strncmp(buff->buff + idx.start, "GET", idx.len) == 0) {
            req->method = IW_WEB_METHOD_GET;
        } else if(strncmp(buff->buff + idx.start, "POST", idx.len) == 0) {
            req->method = IW_WEB_METHOD_POST;
        } else {
            // Unsupported method
            return IW_WEB_PARSE_ERROR;
        }

        // Parse the request URI
        parse = iw_parse_read_token(buff->buff, &req->parse_point,
                                    IW_PARSE_SPACE, true, &req->uri);
        if(parse != IW_PARSE_MATCH) {
            // We received a line so we should have a separator.
            return IW_WEB_PARSE_ERROR;
        }

        // Parse the protocol version
        parse = iw_parse_read_token(buff->buff, &req->parse_point,
                                    IW_PARSE_CRLF, true, &req->version);
        if(parse != IW_PARSE_MATCH) {
            // We recieved a line so we should have a separator.
            return IW_WEB_PARSE_ERROR;
        }
    }

    // Now try to find headers until we have an empty line.
    while(!req->headers_complete) {
        parse = iw_parse_is_token(buff->buff, &req->parse_point, IW_PARSE_CRLF);
        if(parse == IW_PARSE_MATCH) {
            // An empty line, we've completed parsing the headers.
            req->headers_complete = true;
            break;
        }

        // Get the name of the header
        iw_parse_index name;
        parse = iw_parse_read_token(buff->buff, &req->parse_point,
                                    ":", true, &name);
        if(parse != IW_PARSE_MATCH) {
            // We've received a line, we should have a proper header.
            return IW_WEB_PARSE_ERROR;
        }

        // Get the value of the header
        iw_parse_index value;
        parse = iw_parse_read_token(buff->buff, &req->parse_point,
                                    IW_PARSE_CRLF, true, &value);
        if(parse != IW_PARSE_MATCH) {
            // We've received a line, we should have a proper header.
            return IW_WEB_PARSE_ERROR;
        }

        // Create a header index for this header
        iw_web_req_add_header(req, &name, &value);
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
        iw_web_req_method_str(req->method),
        buff->buff);
    iw_list_node *node = req->headers.head;
    while(node != NULL) {
        iw_web_req_header *hdr = (iw_web_req_header *)node;
        node = node->next;
        LOG(IW_LOG_IW, "HDR: \"%.*s\" -> \"%.*s\"",
            hdr->name.len, buff->buff + hdr->name.start,
            hdr->value.len, buff->buff + hdr->value.start);
    }

    req->complete = true;

    return IW_WEB_PARSE_COMPLETE;
}

// --------------------------------------------------------------------------
//
// Attribute access API
//
// --------------------------------------------------------------------------

IW_WEB_METHOD iw_web_req_get_method(const iw_web_req *req) {
    return req->complete ? req->method : IW_WEB_METHOD_NONE;
}

// --------------------------------------------------------------------------

char *iw_web_req_method_str(IW_WEB_METHOD method) {
    switch(method) {
    case IW_WEB_METHOD_GET  : return "GET";
    case IW_WEB_METHOD_POST : return "POST";
    case IW_WEB_METHOD_NONE : return "Not Set";
    default : return "UNSUPPORTED";
    }
}

// --------------------------------------------------------------------------

iw_web_req_header *iw_web_req_get_header(
    const char *buff,
    iw_web_req *req,
    const char *name)
{
    iw_list_node *node = req->headers.head;
    while(node != NULL) {
        iw_web_req_header *hdr = (iw_web_req_header *)node;
        if(strncasecmp(buff + hdr->name.start, name, hdr->name.len) == 0 &&
           strlen(name) == hdr->name.len)
        {
            return hdr;
        }
        node = node->next;
    }
    return NULL;
}

// --------------------------------------------------------------------------

iw_web_req_header *iw_web_req_get_next_header(
    const char *buff,
    iw_web_req *req,
    const char *name,
    const iw_web_req_header *hdr)
{
    iw_list_node *node = (iw_list_node *)hdr;
    while(node != NULL) {
        iw_web_req_header *hdr = (iw_web_req_header *)node;
        if(strncasecmp(buff + hdr->name.start, name, hdr->name.len) == 0 &&
           strlen(name) == hdr->name.len)
        {
            return hdr;
        }
        node = node->next;
    }
    return NULL;
}

// --------------------------------------------------------------------------
