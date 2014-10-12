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
#include "iw_log.h"
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

/// @brief Attempt to parse a client request.
/// @param buff The buffer containing the client request.
/// @param file The file stream to write the response to.
/// @return True if a request was found (whether or not it was a valid request).
static bool iw_web_srv_parse_request(iw_buff *buff, FILE *out) {
    // Search for the newline signifying the end of the request.
    char *end = strstr(buff->buff, "\r\n");
    if(end != NULL) {
        char *content = "<html><head><title>Response</title></head><body><h1>Response</h1></body></html>";
        // There is a request, go ahead and parse it.
        *end = '\0';
        LOG(IW_LOG_IW, "Received request \"%s\"", buff->buff);
        fprintf(out, "HTTP/1.1 200 Ok\r\n"
                        "Content-Length: %d\r\n"
                        "\r\n"
                        "%s\r\n",
                        strlen(content),
                        content);
        int len = end - buff->buff;

        // Remove the request we just processed. Make sure to remove the '\r\n'
        // even though we did not use it for the parsing.
        iw_buff_remove_data(buff, len + 2);
        return true;
    }
    return false;
}

// --------------------------------------------------------------------------

/// @brief Process a client request.
/// @param fd The client's socket file descriptor.
static void iw_web_srv_process_request(int fd) {
    iw_buff buff;
    if(!iw_buff_create(&buff, BUFF_SIZE, 10 * BUFF_SIZE)) {
        LOG(IW_LOG_IW, "Failed to create command server request buffer");
        goto done;
    }
    FILE *out = fdopen(fd, "r+w+");
    int bytes;
    do {
        int remainder = iw_buff_remainder(&buff);
        if(remainder == 0) {
            // Out of buffer space and could not parse request, close socket
            // and return
            LOG(IW_LOG_IW, "Failed to parse request");
            goto done;
        }
        char *ptr;
        if(!iw_buff_reserve_data(&buff, &ptr, remainder)) {
            LOG(IW_LOG_IW, "Failed to allocate command server request buffer");
            goto done;
        }
        bytes = recv(fd, ptr, remainder, 0);
        if(bytes == -1) {
            LOG(IW_LOG_IW, "Request failed (%d:%s)", errno, strerror(errno));
            goto done;
        } else if(bytes == 0) {
            goto done;
        }

        iw_buff_commit_data(&buff, bytes);
        if(iw_web_srv_parse_request(&buff, out)) {
            // Successfully parsed a request (whether the request was valid
            // or not). Return from this function.
            send(fd, "\0", 1, 0);
            goto done;
        }
    } while(bytes > 0);

done:
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
