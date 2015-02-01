// --------------------------------------------------------------------------
///
/// @file iw_ip.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// directory.
///
// --------------------------------------------------------------------------

#include "iw_ip.h"

#include "iw_log.h"
#include "iw_util.h"

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

bool iw_ip_str_to_port(
    char *str,
    unsigned short *port)
{
    long long int tmp;
    if(!iw_strtoll(str, &tmp, 10) ||
       tmp < 0 || tmp > 65535)
    {
        return false;
    }
    *port = tmp;
    return true;
}

// --------------------------------------------------------------------------

bool iw_ip_str_to_addr(
    char *str,
    bool allow_port,
    iw_ip *address)
{
    struct addrinfo hints;
    struct addrinfo *result;
    int retval;
    unsigned short port = 0;
    char buff[IW_IP_BUFF_LEN];

    if(address == NULL || str == NULL) {
        return false;
    }

    if(allow_port) {
        char *addr_end = NULL;
        char *port_start = NULL;
        if(*str == '[') {
            // A bracket, this must be an IPv6 address with or without a port.
            addr_end = port_start = strrchr(str, ']');
            if(port_start != NULL && *++port_start == ':') {
                port_start++;
            }
            // Also update the address start to go beyond the bracket.
            str++;
        } else {
            // Check for an IPv4 address with a port, be careful to not
            // mistake an IPv6 address with colons for a port marker
            addr_end = port_start = strrchr(str, ':');
            if(port_start != NULL &&
               strspn(str, ".1234567890") == port_start - str)
            {
                // There are just numbers and/or dots leading up to the last
                // colon. This could be an IPv4 address. Let's use the port
                // we found. The address validity will be confirmed in the
                // conversion below.
                port_start++;
            } else {
                // Not an IPv4 address with port, just reset port_start.
                port_start = NULL;
            }
        }
        if(port_start != NULL) {
            long long int tmp;
            if(iw_strtoll(port_start, &tmp, 10) && tmp >= 0 && tmp <= 65535) {
                port = tmp;
            } else {
                return false;
            }

            // Need to copy address into buffer since getaddrinfo() will not
            // accept a non-NUL terminated address.
            snprintf(buff, sizeof(buff), "%.*s", (int)(addr_end - str), str);
            str = buff;
        }
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

    if(port != 0) {
        iw_ip_set_port(address, port);
    }

    freeaddrinfo(result);

    return true;
}

// --------------------------------------------------------------------------

bool iw_ip_ipv4_to_addr(
    unsigned int ip,
    iw_ip *address)
{
    memset(address, 0, sizeof(*address));
    struct sockaddr_in *ss = (struct sockaddr_in *)address;
    ss->sin_family = AF_INET;
    ss->sin_addr.s_addr = htonl(ip);
    return true;
}

// --------------------------------------------------------------------------

bool iw_ip_ipv6_to_addr(
    struct in6_addr ip,
    iw_ip *address)
{
    memset(address, 0, sizeof(*address));
    struct sockaddr_in6 *ss = (struct sockaddr_in6 *)address;
    ss->sin6_family = AF_INET6;
    ss->sin6_addr   = ip;
    return true;
}

// --------------------------------------------------------------------------

char *iw_ip_addr_to_str(
    iw_ip *address,
    bool include_port,
    char *buff,
    int buff_len)
{
    bool retval = false;
    char *ptr = buff;
    unsigned short port = iw_ip_get_port(address);
    if(address->ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)address;
        retval = inet_ntop(AF_INET, &s->sin_addr, buff, buff_len) != NULL;
        if(include_port && port != 0) {
            int len = strlen(buff);
            buff += len;
            buff_len -= len;
            snprintf(buff, buff_len, ":%d", port);
        }
    } else if(address->ss_family == AF_INET6) {
        if(include_port && port != 0) {
            *buff++ = '[';
            buff_len--;
        }
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)address;
        retval = inet_ntop(AF_INET6, &s->sin6_addr, buff, buff_len) != NULL;
        if(include_port && port != 0) {
            int len = strlen(buff);
            buff += len;
            buff_len -= len;
            snprintf(buff, buff_len, "]:%d", port);
        }
    }
    return retval ? ptr : NULL;
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

bool iw_ip_equal(iw_ip *addr1, iw_ip *addr2, bool cmp_port) {
    if(addr1->ss_family != addr2->ss_family) {
        return false;
    }
    switch(addr1->ss_family) {
    case AF_INET : {
        struct sockaddr_in *a1 = (struct sockaddr_in *)addr1;
        struct sockaddr_in *a2 = (struct sockaddr_in *)addr2;
        return (!cmp_port || a1->sin_port == a2->sin_port) &&
               (a1->sin_addr.s_addr == a2->sin_addr.s_addr);
        }
    case AF_INET6 : {
        struct sockaddr_in6 *a1 = (struct sockaddr_in6 *)addr1;
        struct sockaddr_in6 *a2 = (struct sockaddr_in6 *)addr2;
        return (!cmp_port || a1->sin6_port == a2->sin6_port) &&
               (memcmp(&a1->sin6_addr, &a2->sin6_addr, sizeof(a1->sin6_addr)) == 0);
        }
    default :
        return false;
    }
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

int iw_ip_open_server_socket(int type, iw_ip *address, bool set_reuse) {
    int sock = socket(address->ss_family, type, 0);
    if(sock == -1) {
        LOG(IW_LOG_IW, "Failed to open server socket (%d:%s)",
            errno, strerror(errno));
        return -1;
    }

    if(set_reuse) {
        int enable = 1;
        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                      &enable, sizeof(enable)) != 0)
        {
            LOG(IW_LOG_IW, "Failed to set SO_REUSEADDR (%d:%s)",
                errno, strerror(errno));
            // No need to return for this failure, worst case is we cannot
            // bind the socket in which case we will return below.
        }
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
