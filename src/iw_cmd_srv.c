// --------------------------------------------------------------------------
///
/// @file iw_cmd_srv.c
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_cmd_srv.h"

#include "iw_buff.h"
#include "iw_cmds_int.h"
#include "iw_common.h"
#include "iw_ip.h"
#include "iw_log.h"
#include "iw_thread_int.h"
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

static int s_cmd_sock = -1;

static pthread_t s_cmd_srv_tid = 0;

// --------------------------------------------------------------------------
//
// Helper functions
//
// --------------------------------------------------------------------------

/// @brief Attempt to parse a client request.
/// @param buff The buffer containing the client request.
/// @param file The file stream to write the response to.
/// @return True if a request was found (whether or not it was a valid request).
static bool iw_cmd_srv_parse_request(iw_buff *buff, FILE *out) {
    // Search for the newline signifying the end of the request.
    char *end = strstr(buff->buff, "\r\n");
    if(end != NULL) {
        // There is a request, go ahead and parse it.
        *end = '\0';
        LOG(IW_LOG_IW, "Received request \"%s\"", buff->buff);
        fprintf(out, "Received request: %s\n", buff->buff);
        int len = end - buff->buff;
        iw_cmd_parse_info info = { buff->buff, NULL };
        info.token = strtok_r(buff->buff, " ", &(info.saveptr));
        iw_cmds_process(&info, out);
        fflush(out);

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
static void iw_cmd_srv_process_request(int fd) {
    iw_buff buff;
    if(!iw_buff_create(&buff, BUFF_SIZE, BUFF_SIZE)) {
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
        if(iw_cmd_srv_parse_request(&buff, out)) {
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

/// @brief The main command socket thread entry point.
/// @param param The parameter passed by the thread creator.
/// @return Nothing.
static void *iw_cmd_srv_thread(void *param) {
    UNUSED(param);

    int retval;

    // Entering command server loop.
    LOG(IW_LOG_IW, "Entering command server loop");
    while((retval = accept(s_cmd_sock, NULL, 0)) != -1) {
        LOG(IW_LOG_IW, "Accepted a client connection, fd=%d", retval);
        iw_cmd_srv_process_request(retval);
    }

    return NULL;
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

bool iw_cmd_srv(unsigned short port) {
    // Open command socket
    iw_ip address;
    if(!iw_ip_ipv4_to_addr(INADDR_LOOPBACK, &address) ||
      !iw_ip_set_port(&address, port)  ||
       (s_cmd_sock = iw_ip_open_server_socket(SOCK_STREAM, &address, true)) == -1)
    {
        LOG(IW_LOG_IW, "Failed to open command server socket.");
        return false;
    }

    // Create thread to serve the command socket.
    if(!iw_thread_create_int(&s_cmd_srv_tid, "CMD Server", iw_cmd_srv_thread, false, NULL)) {
        LOG(IW_LOG_IW, "Failed to create command server thread");
        close(s_cmd_sock);
        return false;
    }

   return true;
}

// --------------------------------------------------------------------------

void iw_cmd_srv_exit() {
    LOG(IW_LOG_IW, "Terminating command server");
    if(s_cmd_srv_tid != 0) {
        shutdown(s_cmd_sock, SHUT_RDWR);
        pthread_join(s_cmd_srv_tid, NULL);
    }
    LOG(IW_LOG_IW, "Command server successfully terminated");
}

// --------------------------------------------------------------------------
