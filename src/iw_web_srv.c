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
#include "iw_web_req.h"

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

/// @brief Create a response and send it.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully written.
static bool iw_web_srv_construct_response(FILE *out) {
    char *content =
        "<html><head><title>Response2</title></head>\n"
        "<body>\n"
        "<h1>Response</h1>\n"
        "<form action='/submit'>\n"
        "Name: <input type='text' name='name'></input>\n"
        "<input type='submit' value='Submit'></input>\n"
        "</form>\n"
        "</body>\n"
        "</html>";
    fprintf(out, "HTTP/1.1 200 Ok\r\n"
                    "Content-Length: %ld\r\n"
                    "\r\n"
                    "%s\r\n",
                    (long int)strlen(content),
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
    iw_web_req_init(&req);
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
        IW_WEB_PARSE parse = iw_web_req_parse(buff.buff, buff.size, &req);
        if(parse == IW_WEB_PARSE_ERROR) {
            LOG(IW_LOG_IW, "Failed to parse request");
            goto done;
        } if(parse == IW_WEB_PARSE_COMPLETE) {
            // Successfully parsed a request. Return from this function.
            iw_buff_remove_data(&buff, req.parse_point);
            goto done;
        }
    } while(bytes > 0);

done:
    iw_web_srv_construct_response(out);

    iw_web_req_free(&req);

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
