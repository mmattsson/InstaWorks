// --------------------------------------------------------------------------
///
/// @file iw_cmd_clnt.c
///
/// For the client code, we do not use the logging framework. This is due to
/// the fact that the client will only be 'alive' long enough to open a
/// UNIX domain socket to the server and issue the given request. It will
/// read the response and output this on the terminal and then exit.
///
/// Any output occurring during this operation should be printed on stdout
/// or stderr.
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_cmd_clnt.h"
#include "iw_ip.h"

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

bool iw_cmd_clnt(unsigned short port, int argc, char **argv) {
    // Open client socket
    int sock;
    iw_ip address;
    if(!iw_ip_ipv4_to_addr(INADDR_LOOPBACK, &address) ||
      !iw_ip_set_port(&address, port)  ||
       (sock = iw_ip_open_client_socket(SOCK_STREAM, &address)) == -1)
    {
        fprintf(stderr, "Failed to connect to server\n");
        return false;
    }

    // Write the command line parameters to the server socket as our request.
    int cnt;
    for(cnt=0;cnt < argc;cnt++) {
        if(send(sock, argv[cnt], strlen(argv[cnt]), 0) == -1) {
            fprintf(stderr, "Failed to send request\n");
            goto done;
        }
        if(cnt < argc - 1) {
            if(send(sock, " ", 1, 0) == -1) {
                fprintf(stderr, "Failed to send request\n");
                goto done;
            }
        }
    }
    if(send(sock, "\r\n", 2, 0) == -1) {
        fprintf(stderr, "Failed to send request\n");
        goto done;
    }

    // Read the server response and display the results.
    bool keepGoing = true;
    int bytes = 0;
    char buffer[128];
    while(keepGoing && (bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        // Check the response for a NUL byte which indicates the end of the
        // response. If we see one, then close the socket.
        for(cnt=0;cnt < bytes;cnt++) {
            if(buffer[cnt] == '\0') {
                // Found the end of the response. Do a close on the socket
                // immediately in order to make the client the initiator of
                // the close. This should make the client socket the side
                // that goes into TIME_WAIT instead of the server.
                keepGoing = false;
                close(sock);
                sock = -1;
                break;
            }
        }
        printf("%.*s", bytes, buffer);
    }
    printf("\n");

done:
    // At this point we need to close the socket if we didn't receive the
    // NUL byte from the server.
    if(sock != -1) {
        close(sock);
    }

    return true;
}

// --------------------------------------------------------------------------
