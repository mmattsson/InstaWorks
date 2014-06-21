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
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#include "iw_cmd_clnt.h"

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
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        fprintf(stderr,
                "Failed to open command client socket (%d:%s)\n", 
                errno, strerror(errno));
        return false;
    }

    // Connect to the server socket.
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    if(connect(sock, (struct sockaddr *)&address, sizeof(address)) != 0) {
        fprintf(stderr,
                "Failed to connect to server (%d:%s)\n", errno, strerror(errno));
        close(sock);
        return false;
    }

    // Write the command line parameters to the server socket as our request.
    int cnt;
    for(cnt=0;cnt < argc;cnt++) {
        send(sock, argv[cnt], strlen(argv[cnt]), 0);
        if(cnt < argc - 1) {
            send(sock, " ", 1, 0);
        }
    }
    send(sock, "\r\n", 2, 0);

    // Read the server response and display the results.
    int bytes = 0;
    char buffer[128];
    while((bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        printf("%.*s", bytes, buffer);
    }
    printf("\n");

    return true;
}

// --------------------------------------------------------------------------
