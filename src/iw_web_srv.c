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

// TODO: Add glue code to tie web server module to the programs configuration
// settings to allow it to be shown through the web GUI.

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
// Helper functions
//
// --------------------------------------------------------------------------

static bool iw_web_srv_respond(iw_web_srv *srv, iw_web_req *req, FILE *out) {
    char *ptr = NULL;
    size_t size = 0;
    FILE *mem_buf = open_memstream(&ptr, &size);
    if(mem_buf == NULL) {
        return NULL;
    }

    // Let the callback function create the HTML page to send back
    if(!srv->callback(req, mem_buf)) {
        LOG(IW_LOG_WEB, "Sending a response:\n"
                "HTTP/1.1 404 Not found\r\n"
                "\r\n");

        // Create an HTTP response with the HTML content.
        fprintf(out, "HTTP/1.1 404 Not found\r\n"
                "\r\n");

        fclose(mem_buf);
        free(ptr);
        return false;
    }
    fclose(mem_buf);

    LOG(IW_LOG_WEB, "Sending a response:\n"
            "HTTP/1.1 200 Ok\r\n"
            "Content-Length: %ld\r\n"
            "\r\n"
            "%s\r\n",
            (long int)strlen(ptr),
            ptr);

    // Create an HTTP response with the HTML content.
    fprintf(out, "HTTP/1.1 200 Ok\r\n"
            "Content-Length: %ld\r\n"
            "\r\n"
            "%s\r\n",
            (long int)strlen(ptr),
            ptr);

    free(ptr);

    return true;
}

// --------------------------------------------------------------------------

/// @brief Process a client request.
/// @param srv The web server object.
/// @param fd The client's socket file descriptor.
static void iw_web_srv_process_request(iw_web_srv *srv, int fd) {
    iw_web_req req;
    iw_buff buff;
    memset(&req, 0, sizeof(req));
    if(!iw_buff_create(&buff, BUFF_SIZE, 10 * BUFF_SIZE)) {
        LOG(IW_LOG_WEB, "Failed to create command server request buffer");
        goto done;
    }
    FILE *out = fdopen(fd, "r+w+");
    int bytes;
    iw_web_req_init(&req);
    do {
        char *ptr;
        if(!iw_buff_reserve_data(&buff, &ptr, BUFF_SIZE)) {
            LOG(IW_LOG_WEB, "Failed to allocate command server request buffer");
            goto done;
        }
        bytes = recv(fd, ptr, BUFF_SIZE, 0);
        if(bytes == -1) {
            LOG(IW_LOG_WEB, "Request failed (%d:%s)", errno, strerror(errno));
            goto done;
        } else if(bytes == 0) {
            goto done;
        }

        iw_buff_commit_data(&buff, bytes);
        req.buff = buff.buff;
        req.len  = buff.size;
        IW_WEB_PARSE parse = iw_web_req_parse(&req);
        if(parse == IW_WEB_PARSE_ERROR) {
            LOG(IW_LOG_WEB, "Failed to parse request");
            goto done;
        } if(parse == IW_WEB_PARSE_COMPLETE) {
            // Successfully parsed a request. Return from this function.
            iw_buff_remove_data(&buff, req.parse_point);
            goto done;
        }
    } while(bytes > 0);

done:
    iw_web_srv_respond(srv, &req, out);

    iw_web_req_free(&req);

    // Give the client time to close the connection to avoid having the server
    // socket go into a TIME_WAIT state after program termination.
    usleep(100000);
    fclose(out);
    iw_buff_destroy(&buff);
    LOG(IW_LOG_WEB, "Closed a client connection, fd=%d", fd);
}

// --------------------------------------------------------------------------

/// @brief The main web socket thread entry point.
/// @param param The parameter passed by the thread creator.
/// @return Nothing.
static void *iw_web_srv_thread(void *param) {
    int retval;
    iw_web_srv *srv = (iw_web_srv *)param;

    if(srv == NULL) {
        return NULL;
    }

    // Entering web server loop.
    LOG(IW_LOG_WEB, "Entering web server loop");
    while((retval = accept(srv->fd, NULL, 0)) != -1) {
        LOG(IW_LOG_WEB, "Accepted a client connection, fd=%d", retval);
        iw_web_srv_process_request(srv, retval);
    }

    close(srv->fd);
    IW_FREE(srv);

    return NULL;
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

iw_web_srv *iw_web_srv_init(
    iw_ip *address,
    unsigned short port,
    IW_WEB_REQ_FN callback)
{
    iw_web_srv *srv = (iw_web_srv *)IW_CALLOC(1, sizeof(iw_web_srv));
    if(srv == NULL) {
        return NULL;
    }
    srv->callback = callback;

    // Open server socket
    iw_ip tmp;
    if(address == NULL) {
        address = &tmp;
        if(!iw_ip_ipv4_to_addr(INADDR_LOOPBACK, address)) {
            LOG(IW_LOG_WEB, "Failed to open web server socket.");
            return NULL;
        }
    }

    if(port == 0) {
        port = 8080;
    }

    if(!iw_ip_set_port(address, port)  ||
       (srv->fd = iw_ip_open_server_socket(SOCK_STREAM, address, true)) == -1)
    {
        LOG(IW_LOG_WEB, "Failed to open web server socket.");
        return NULL;
    }

    // Create thread to serve the command socket.
    if(!iw_thread_create("Web Server", iw_web_srv_thread, srv)) {
        LOG(IW_LOG_WEB, "Failed to create web server thread");
        close(srv->fd);
        return NULL;
    }

    return srv;
}

// --------------------------------------------------------------------------
