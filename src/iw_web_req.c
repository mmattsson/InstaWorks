// --------------------------------------------------------------------------
///
/// @file iw_web_req.c
///
/// Module for parsing and handling HTTP requests.
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_web_req.h"

#include "iw_log.h"
#include "iw_memory.h"
#include "iw_thread.h"
#include "iw_util.h"
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

char *iw_web_req_urldecode(const char *str, unsigned int len) {
    int str_idx, copy_idx;
    char buff[3] = { 0 };
    char *copy = IW_MALLOC(len + 1);
    if(copy == NULL) {
        return NULL;
    }

    for(str_idx=0, copy_idx=0;str_idx < len;copy_idx++) {
        if(*(str + str_idx) == '%') {
            buff[0] = *(str + str_idx + 1);
            buff[1] = *(str + str_idx + 2);
            long long int ascii;
            if(!iw_strtoll(buff, &ascii, 16)) {
                IW_FREE(copy);
                return NULL;
            }
            *(copy + copy_idx) = ascii;
            str_idx += 3;
        } else if(*(str + str_idx) == '+') {
            *(copy + copy_idx) = ' ';
            str_idx++;
        } else {
            *(copy + copy_idx) = *(str + str_idx);
            str_idx++;
        }
    }
    *(copy + copy_idx) = '\0';
    return copy;
}

// --------------------------------------------------------------------------

void iw_web_req_init(iw_web_req *req) {
    memset(req, 0, sizeof(*req));
}

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
    if(value != NULL) {
        hdr->value = *value;
    } else {
        memset(&hdr->value, 0, sizeof(hdr->value));
    }
    iw_list_add(&req->headers, (iw_list_node *)hdr);
    return hdr;
}

// --------------------------------------------------------------------------

iw_web_req_parameter *iw_web_req_add_parameter(
    iw_web_req *req,
    iw_parse_index *name,
    iw_parse_index *value)
{
    iw_web_req_parameter *param =
            (iw_web_req_parameter *)IW_CALLOC(1, sizeof(iw_web_req_parameter));
    if(param == NULL) {
        return NULL;
    }
    param->name  = iw_web_req_urldecode(req->buff + name->start, name->len);
    param->value = iw_web_req_urldecode(req->buff + value->start, value->len);
    iw_list_add(&req->parameters, (iw_list_node *)param);
    return param;
}

// --------------------------------------------------------------------------

void iw_web_req_delete_header(iw_list_node *node) {
    iw_web_req_header *hdr = (iw_web_req_header *)node;
    IW_FREE(hdr);
}

// --------------------------------------------------------------------------

void iw_web_req_delete_parameter(iw_list_node *node) {
    iw_web_req_parameter *param = (iw_web_req_parameter *)node;
    IW_FREE(param->name);
    IW_FREE(param->value);
    IW_FREE(param);
}

// --------------------------------------------------------------------------

void iw_web_req_free(iw_web_req *req) {
    iw_list_destroy(&req->headers, iw_web_req_delete_header);
    iw_list_destroy(&req->parameters, iw_web_req_delete_parameter);
}

// --------------------------------------------------------------------------

/// @brief Parse an HTTP query.
/// This function will parse the query and add parameters to the request object.
/// @param req The HTTP request to parse.
/// @param start The start of the query to parse.
/// @param end The end of the query to parse.
static void iw_web_req_parse_query(
    iw_web_req *req,
    unsigned int start,
    unsigned int end)
{
    unsigned int offset = start;
    iw_parse_index name;
    iw_parse_index value;
    IW_PARSE parse = iw_parse_read_to_token(req->buff, end, &offset,
                                            IW_PARSE_EQUAL, false, &name);
    while(parse == IW_PARSE_MATCH) {
        parse = iw_parse_read_to_token(req->buff, end,
                                        &offset, IW_PARSE_AMPERSAND,
                                        false, &value);
        if(parse == IW_PARSE_MATCH) {
            // We found a name/value pair, let's add that
            // Create a header index for this header
            iw_web_req_add_parameter(req, &name, &value);
        } else if(offset < req->parse_point) {
            // We couldn't find another parameter but there are more
            // characters after this equal sign. This means that this
            // is the last parameter and we take the remaining data.
            value.start = offset;
            value.len   = end - offset;
            iw_web_req_add_parameter(req, &name, &value);
        } else {
            // We found a name without a value, let's add that
            // Create a header index for this header
             iw_web_req_add_parameter(req, &name, NULL);
       }
        parse = iw_parse_read_to_token(req->buff, end,
                                        &offset, IW_PARSE_EQUAL,
                                        false, &name);
    }
}

// --------------------------------------------------------------------------

IW_WEB_PARSE iw_web_req_parse(iw_web_req *req) {
    IW_PARSE parse;
    unsigned int offset;

    if(req->method == IW_WEB_METHOD_NONE) {
        // Search for the newline signifying the end of the request
        // URI or header.
        offset = 0;
        parse = iw_parse_find_token(req->buff, req->len, &offset,
                                    IW_PARSE_CRLF);
        if(parse != IW_PARSE_MATCH) {
            // Didn't find another line, just return.
            return IW_WEB_PARSE_INCOMPLETE;
        }

        // No method set yet, try to parse request line. We should have
        // received a whole line since we passed the test above.
        iw_parse_index idx;
        parse = iw_parse_read_to_token(req->buff, req->len, &req->parse_point,
                                       IW_PARSE_SPACE, true, &idx);
        if(parse != IW_PARSE_MATCH) {
            // We received a line so we should have a separator.
            return IW_WEB_PARSE_ERROR;
        }
        if(iw_parse_cmp("GET", req->buff, &idx)) {
            req->method = IW_WEB_METHOD_GET;
        } else if(iw_parse_cmp("HEAD", req->buff, &idx)) {
            req->method = IW_WEB_METHOD_HEAD;
        } else if(iw_parse_cmp("POST", req->buff, &idx)) {
            req->method = IW_WEB_METHOD_POST;
        } else if(iw_parse_cmp("PUT", req->buff, &idx)) {
            req->method = IW_WEB_METHOD_PUT;
        } else if(iw_parse_cmp("DELETE", req->buff, &idx)) {
            req->method = IW_WEB_METHOD_DELETE;
        } else if(iw_parse_cmp("TRACE", req->buff, &idx)) {
            req->method = IW_WEB_METHOD_TRACE;
        } else if(iw_parse_cmp("CONNECT", req->buff, &idx)) {
            req->method = IW_WEB_METHOD_CONNECT;
        } else {
            // Unsupported method
            return IW_WEB_PARSE_ERROR;
        }

        // Parse the request URI
        offset = req->parse_point;
        parse = iw_parse_read_to_token(req->buff, req->len, &req->parse_point,
                                       IW_PARSE_SPACE, true, &req->uri);
        if(parse != IW_PARSE_MATCH) {
            // We received a line so we should have a separator.
            return IW_WEB_PARSE_ERROR;
        }

        // Now go back and break down the URI into the path and the parameters.
        // Note that we use the req->uri end as the end of the data for parsing
        // of the URI.
        int end = req->uri.start + req->uri.len;
        parse = iw_parse_read_to_token(req->buff, end, &offset,
                                        IW_PARSE_QUERY, false, &req->path);
        if(parse != IW_PARSE_MATCH) {
            // We received a line so if the '?' is not present, there are no
            // parameters in the request URI. In this case, we set the path to
            // the whole URI.
            req->path = req->uri;
        } else {
            // There is a '?' present. Collect each parameter.
            iw_web_req_parse_query(req, offset, end);
        }

        // Parse the protocol version
        parse = iw_parse_read_to_token(req->buff, req->len, &req->parse_point,
                                       IW_PARSE_CRLF, true, &req->version);
        if(parse != IW_PARSE_MATCH) {
            // We recieved a line so we should have a separator.
            return IW_WEB_PARSE_ERROR;
        }
    }

    // Now try to find headers until we have an empty line.
    while(!req->headers_complete) {
        parse = iw_parse_is_token(req->buff, req->len, &req->parse_point,
                                  IW_PARSE_CRLF);
        if(parse == IW_PARSE_MATCH) {
            // An empty line, we've completed parsing the headers.
            req->headers_complete = true;
            break;
        }

        // Find a new-line to make sure we have enough data to parse a header.
        // However, don't update the parse point until we've read the header.
        offset = req->parse_point;
        parse = iw_parse_find_token(req->buff, req->len, &offset,
                                    IW_PARSE_CRLF);
        if(parse != IW_PARSE_MATCH) {
            // Didn't find another line, just return.
            return IW_WEB_PARSE_INCOMPLETE;
        }

        // Get the name of the header
        iw_parse_index name;
        parse = iw_parse_read_to_token(req->buff, req->len, &req->parse_point,
                                       IW_PARSE_COLON, true, &name);
        if(parse != IW_PARSE_MATCH) {
            // We've received a line, we should have a proper header.
            return IW_WEB_PARSE_ERROR;
        }

        // Get the value of the header
        iw_parse_index value;
        parse = iw_parse_read_to_token(req->buff, req->len, &req->parse_point,
                                       IW_PARSE_CRLF, true, &value);
        if(parse != IW_PARSE_MATCH) {
            // We've received a line, we should have a proper header.
            return IW_WEB_PARSE_ERROR;
        }

        // If this is the Content-Length header, then set the content-length.
        if(iw_parse_casecmp("Content-Length", req->buff, &name)) {
            long long int content_length;
            if(iw_strtoll(req->buff + value.start, &content_length, 10)) {
                req->content_length = content_length;
            }
        }

        // Create a header index for this header
        iw_web_req_add_header(req, &name, &value);
    }

    // The presence of an empty line signifies the end of the request header.
    // Either the request is complete, or if a Content-Length header was
    // received with a non-zero value, we know how much data there is in the
    // body.
    if(req->content_length > 0) {
        if(req->len - req->parse_point < req->content_length) {
            // We do not have enough data to complete the request
            return IW_WEB_PARSE_INCOMPLETE;
        }
        req->content.start = req->parse_point;
        req->content.len   = req->content_length;
        req->parse_point += req->content_length;
    }

    // If this was a POST request and the Content-Type is
    // 'application/x-www-form-urlencoded' then we should try to parse the
    // Content as a query-string.
    iw_web_req_header *hdr = iw_web_req_get_header(req, "Content-Type");
    if(req->method == IW_WEB_METHOD_POST &&
       hdr != NULL &&
       iw_parse_casecmp("application/x-www-form-urlencoded",
                        req->buff, &hdr->value))
    {
        // Parse the body
        iw_web_req_parse_query(req, req->content.start,
                               req->content.start + req->content.len);
    }

    // Debug log the request we just received
    if(DO_LOG(IW_LOG_WEB)) {
        LOG(IW_LOG_WEB, "Received %s method, data=\n\"%s\"",
            iw_web_req_method_str(req->method),
            req->buff);
        LOG(IW_LOG_WEB, "URI=\"%.*s\"",
            req->uri.len, req->buff + req->uri.start);
        LOG(IW_LOG_WEB, "PATH=\"%.*s\"",
            req->path.len, req->buff + req->path.start);
        iw_list_node *node = req->headers.head;
        while(node != NULL) {
            iw_web_req_header *hdr = (iw_web_req_header *)node;
            node = node->next;
            LOG(IW_LOG_WEB, "HDR: \"%.*s\" -> \"%.*s\"",
                hdr->name.len, req->buff + hdr->name.start,
                hdr->value.len, req->buff + hdr->value.start);
        }
        node = req->parameters.head;
        while(node != NULL) {
            iw_web_req_parameter *param = (iw_web_req_parameter *)node;
            node = node->next;
            LOG(IW_LOG_WEB, "PRM: \"%s\" -> \"%s\"",
                param->name, param->value);
        }
        LOG(IW_LOG_WEB, "Content (%d bytes):\n\"%.*s\"",
            req->content.len, req->content.len, req->buff + req->content.start);
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
    case IW_WEB_METHOD_NONE   : return "Not Set";
    case IW_WEB_METHOD_GET    : return "GET";
    case IW_WEB_METHOD_HEAD   : return "HEAD";
    case IW_WEB_METHOD_POST   : return "POST";
    case IW_WEB_METHOD_PUT    : return "PUT";
    case IW_WEB_METHOD_DELETE : return "DELETE";
    case IW_WEB_METHOD_TRACE  : return "TRACE";
    case IW_WEB_METHOD_CONNECT: return "CONNECT";
    }
    return "Unsupported";
}

// --------------------------------------------------------------------------

iw_web_req_header *iw_web_req_get_header(
    iw_web_req *req,
    const char *name)
{
    iw_list_node *node = (iw_list_node *)req->headers.head;
    while(node != NULL) {
        iw_web_req_header *hdr = (iw_web_req_header *)node;
        if(name == NULL || iw_parse_casecmp(name, req->buff, &hdr->name)) {
            return hdr;
        }
        node = node->next;
    }
    return NULL;
}

// --------------------------------------------------------------------------

iw_web_req_header *iw_web_req_get_next_header(
    iw_web_req *req,
    const char *name,
    const iw_web_req_header *hdr)
{
    if(hdr == NULL) {
        return NULL;
    }
    iw_list_node *node = ((iw_list_node *)hdr)->next;
    while(node != NULL) {
        iw_web_req_header *hdr = (iw_web_req_header *)node;
        if(name == NULL || iw_parse_casecmp(name, req->buff, &hdr->name)) {
            return hdr;
        }
        node = node->next;
    }
    return NULL;
}

// --------------------------------------------------------------------------

iw_web_req_parameter *iw_web_req_get_parameter(
    iw_web_req *req,
    const char *name)
{
    iw_list_node *node = (iw_list_node *)req->parameters.head;
    while(node != NULL) {
        iw_web_req_parameter *param = (iw_web_req_parameter *)node;
        if(name == NULL || strcasecmp(name, param->name) == 0) {
            return param;
        }
        node = node->next;
    }
    return NULL;
}

// --------------------------------------------------------------------------

iw_web_req_parameter *iw_web_req_get_next_parameter(
    iw_web_req *req,
    const char *name,
    const iw_web_req_parameter *param)
{
    if(param == NULL) {
        return NULL;
    }
    iw_list_node *node = ((iw_list_node *)param)->next;
    while(node != NULL) {
        iw_web_req_parameter *param = (iw_web_req_parameter *)node;
        if(name == NULL || strcasecmp(name, param->name) == 0) {
            return param;
        }
        node = node->next;
    }
    return NULL;
}

// --------------------------------------------------------------------------
