// --------------------------------------------------------------------------
///
/// @file test_opts.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_ip.h"
#include "iw_util.h"

#include "tests.h"

#include <stdio.h>
#include <string.h>

// --------------------------------------------------------------------------

static char *ip_addr[] = {
    "0.0.0.0",
    "192.168.1.1",
    "10.10.10.10",
    "2001:db8::1",
    "3001:db8::1:2",
    "fe80::a00:27ff:febf:2395",
    "FF01::1",
    "::ffff:192.0.2.128"
};

// --------------------------------------------------------------------------

void test_ip(test_result *result) {
    char buff[IW_IP_BUFF_LEN];
    iw_ip address;
    int cnt;

    // Test some canned values by converting from a string to an address and
    // back again.
    for(cnt=0;cnt < IW_ARR_LEN(ip_addr);cnt++) {
        iw_ip_str_to_addr(ip_addr[cnt], &address);
        iw_ip_addr_to_str(&address, buff, sizeof(buff));
        test(result, strcasecmp(ip_addr[cnt], buff) == 0,
             "Converting %s to IP address and back?", ip_addr[cnt]);
    }

    memset(&address, 0, sizeof(address));
    address.ss_family = AF_INET;

    // Test converting from IPv4 known addresses to string
    ((struct sockaddr_in *)&address)->sin_addr.s_addr = htonl(INADDR_ANY);
    iw_ip_addr_to_str(&address, buff, sizeof(buff));
    test(result, strcmp(buff, "0.0.0.0") == 0,
         "Converting INADDR_ANY to 0.0.0.0?");
    ((struct sockaddr_in *)&address)->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    iw_ip_addr_to_str(&address, buff, sizeof(buff));
    test(result, strcmp(buff, "127.0.0.1") == 0,
         "Converting INADDR_LOOPBACK to 127.0.0.1?");
    ((struct sockaddr_in *)&address)->sin_addr.s_addr = htonl(INADDR_BROADCAST);
    iw_ip_addr_to_str(&address, buff, sizeof(buff));
    test(result, strcmp(buff, "255.255.255.255") == 0,
         "Converting INADDR_BROADCAST to 255.255.255.255?");
}

// --------------------------------------------------------------------------
