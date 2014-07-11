// --------------------------------------------------------------------------
///
/// @file iw_ip.h
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_IP_H_
#define _IW_IP_H_
#ifdef _cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------

#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>

//---------------------------------------------------------------------------

/// @brief Socket address storage structure
/// Typedef to allow us to refer to a sockaddr_storage structure with a
/// shorter name.
typedef struct sockaddr_storage iw_ip;

/// The size needed for a buffer to contain a string representation of an IP
/// address.
#define IW_IP_BUFF_LEN  INET6_ADDRSTRLEN

//---------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Convert a string to a socket address.
/// @param str The string to convert.
/// @param address [out] A pointer to the address to receive the result.
/// @return True if the address was successfully converted.
extern bool iw_ip_str_to_addr(
    char *str,
    iw_ip *address);

// --------------------------------------------------------------------------

/// @brief Convert an IPv4 address to a socket address.
/// @param ip The IPv4 address to convert.
/// @param address [out] A pointer to the address to receive the result.
/// @return True if the address was successfully converted.
extern bool iw_ip_ipv4_to_addr(
    unsigned int ip,
    iw_ip *address);

// --------------------------------------------------------------------------

/// @brief Convert an IPv6 address to a socket address.
/// @param ip The IPv6 address to convert.
/// @param address [out] A pointer to the address to receive the result.
/// @return True if the address was successfully converted.
extern bool iw_ip_ipv6_to_addr(
    struct in6_addr ip,
    iw_ip *address);

// --------------------------------------------------------------------------

/// @brief Convert a socket address to a string representation.
/// @param address The address to convert.
/// @param buff [out] The buffer to put the string into. Should be at least IW_IP_BUFF_LEN.
/// @param buff_len The size of the buffer.
/// @return True if the address was successfully converted.
extern bool iw_ip_addr_to_str(
    iw_ip *address,
    char *buff,
    unsigned int buff_len);

// --------------------------------------------------------------------------

/// @brief Return the port number from an address structure.
/// @param address The address to return the port number for.
/// @return The port number.
extern unsigned short iw_ip_get_port(iw_ip *address);

// --------------------------------------------------------------------------

/// @brief Sets the port number for an address structure.
/// @param address The address to set the port number for.
/// @param port The port number to set.
extern bool iw_ip_set_port(iw_ip *address, unsigned short port);

// --------------------------------------------------------------------------

/// @brief Opens a client socket and connects to the given server.
/// @param type The type of socket, SOCK_DGRAM or SOCK_STREAM.
/// @param address The address to connect to.
/// @return The resulting socket file descriptor or -1 for errors.
extern int iw_ip_open_client_socket(int type, iw_ip *address);

// --------------------------------------------------------------------------

/// @brief Open a server socket.
/// Opens and binds a server socket of the given address-family and type
/// and binds it to the given address.
/// @param type The type of socket, SOCK_DGRAM or SOCK_STREAM.
/// @param address The address to bind the socket to.
/// @param set_reuse True if SO_REUSEADDR should be set for the socket.
/// @return The resulting socket file descriptor or -1 for errors.
extern int iw_ip_open_server_socket(int type, iw_ip *address, bool set_reuse);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_IP_H_

// --------------------------------------------------------------------------
