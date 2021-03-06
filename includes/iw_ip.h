// --------------------------------------------------------------------------
///
/// @file iw_ip.h
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
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
/// address and port.
#define IW_IP_BUFF_LEN  (INET6_ADDRSTRLEN + 8)

//---------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Convert a string to a port number.
/// Validates that the port is a number and within the range or a port
/// number.
/// @param str The string to convert.
/// @param port [out] A pointer to the variable to receive the port number.
/// @return True if the port number was successfully converted.
extern bool iw_ip_str_to_port(
    const char *str,
    unsigned short *port);

// --------------------------------------------------------------------------

/// @brief Convert a string to a socket address.
/// If a port number is allowed in the conversion, the port number must
/// be entered with a colon followed by the port number. If the address
/// is an IPv6 address, the address must be enclosed in brackets if a port
/// number is present. For example;
/// 192.168.1.100:1234
/// [2001:db8::1]:1234
/// @param str The string to convert.
/// @param allow_port True if a port number is allowed.
/// @param address [out] A pointer to the address to receive the result.
/// @return True if the address was successfully converted.
extern bool iw_ip_str_to_addr(
    const char *str,
    bool allow_port,
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
/// If include_port is set, and the port number is non-zero, the port number
/// will be added to the string after a colon. If the address is an IPv6 address,
/// the address will be surrounded by brackets if a port is added.
/// For example;
/// 192.168.1.100:1234
/// [2001:db8::1]:1234
/// @param address The address to convert.
/// @param include_port True if the port should be included in the conversion.
/// @param buff [out] The buffer to put the string into. Should be at least IW_IP_BUFF_LEN.
/// @param buff_len The size of the buffer.
/// @return A pointer to buff if successful, NULL otherwise.
extern char *iw_ip_addr_to_str(
    iw_ip *address,
    bool include_port,
    char *buff,
    int buff_len);

// --------------------------------------------------------------------------

/// @brief Return the port number from an address structure.
/// @param address The address to return the port number for.
/// @return The port number.
extern unsigned short iw_ip_get_port(iw_ip *address);

// --------------------------------------------------------------------------

/// @brief Sets the port number for an address structure.
/// @param address The address to set the port number for.
/// @param port The port number to set.
/// @return true if the port was successfully set.
extern bool iw_ip_set_port(iw_ip *address, unsigned short port);

// --------------------------------------------------------------------------

/// @brief Compares two IP addresses and returns true if they are equal.
/// @param addr1 The first address to compare.
/// @param addr2 The second address to compare.
/// @param cmp_port True if the port numbers should be compared as well.
/// @return True if the addresses are equal.
extern bool iw_ip_equal(iw_ip *addr1, iw_ip *addr2, bool cmp_port);

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
