// --------------------------------------------------------------------------
///
/// @file iw_ip.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// directory.
///
// --------------------------------------------------------------------------

#include "iw_ip.h"

#include "iw_log.h"

#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

bool iw_ip_str_to_addr(
    char *str,
    iw_ip *address)
{
    struct addrinfo hints;
    struct addrinfo *result;
    int retval;

    if(address == NULL) {
        return false;
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags  = AI_NUMERICHOST;

    retval = getaddrinfo(str, NULL, &hints, &result);
    if(retval != 0) {
        LOG(IW_LOG_IW, "Failed to convert string to address (%d:%s)",
            retval, gai_strerror(retval));
        return false;
    }

    if(result == NULL || result->ai_addrlen > sizeof(*address)) {
        LOG(IW_LOG_IW, "Failed to convert string to address");
        return false;
    }

    memcpy(address, result->ai_addr, result->ai_addrlen);

    freeaddrinfo(result);

    return true;
}

// --------------------------------------------------------------------------

bool iw_ip_addr_to_str(
    iw_ip *address,
    char *buff,
    unsigned int buff_len)
{
    if(address->ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)address;
        return inet_ntop(AF_INET, &s->sin_addr, buff, buff_len) == NULL;
    } else if(address->ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)address;
        return inet_ntop(AF_INET6, &s->sin6_addr, buff, buff_len) == NULL;
    }
    return false;
}

// --------------------------------------------------------------------------

unsigned short iw_ip_get_port(iw_ip *address) {
    switch(address->ss_family) {
    case AF_INET  : return ntohs(((struct sockaddr_in *)address)->sin_port);
    case AF_INET6 : return ntohs(((struct sockaddr_in6 *)address)->sin6_port);
    default : return 0;
    }
}

// --------------------------------------------------------------------------

bool iw_ip_set_port(iw_ip *address, unsigned short port) {
    switch(address->ss_family) {
    case AF_INET  :
        ((struct sockaddr_in *)address)->sin_port = htons(port);
        break;
    case AF_INET6 :
        ((struct sockaddr_in6 *)address)->sin6_port = htons(port);
        break;
    }
    return true;
}

// --------------------------------------------------------------------------

int iw_ip_open_client_socket(int type, iw_ip *address) {
    int sock = socket(address->ss_family, type, 0);
    if(sock == -1) {
        LOG(IW_LOG_IW, "Failed to open client socket (%d:%s)",
            errno, strerror(errno));
        return -1;
    }

    if(connect(sock, (struct sockaddr *)address, sizeof(*address)) != 0) {
        LOG(IW_LOG_IW, "Failed to connect to server (%d:%s)",
            errno, strerror(errno));
        return -1;
    }

    return sock;
}

// --------------------------------------------------------------------------

int iw_ip_open_server_socket(int type, iw_ip *address) {
    int sock = socket(address->ss_family, type, 0);
    if(sock == -1) {
        LOG(IW_LOG_IW, "Failed to open server socket (%d:%s)",
            errno, strerror(errno));
        return -1;
    }

    if(bind(sock, (struct sockaddr *)address, sizeof(*address)) != 0) {
        LOG(IW_LOG_IW, "Failed to bind server socket (%d:%s)",
            errno, strerror(errno));
        close(sock);
        return -1;
    }

    if(listen(sock, 5) != 0) {
        LOG(IW_LOG_IW, "Failed to listen on server socket (%d:%s)",
            errno, strerror(errno));
        close(sock);
        return -1;
    }

    LOG(IW_LOG_IW, "Opened server socket on port %d!", iw_ip_get_port(address));
    return sock;
}

// --------------------------------------------------------------------------
