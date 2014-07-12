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

static char *ip_addr_port[] = {
    "0.0.0.0:10000",
    "192.168.1.1:1234",
    "10.10.10.10:65535",
    "[2001:db8::1]:1234",
    "[3001:db8::1:2]:10000",
    "[fe80::a00:27ff:febf:2395]:1",
    "[FF01::1]:65535",
    "[::ffff:192.0.2.128]:1000"
};

// --------------------------------------------------------------------------

static char *ip_addr_fail[] = {
    "0.0.0.0.0"
    "192.168.1.a",
    "1.1.1.1:1000", // This conversion is without ports so this should fail
    "abcd",
    "2001:db8:1",
    "2001:db8::x",
    "::ffff:192.168.0.a"
    "2002:db8:::1",
    "2001:db8::a::1",
    "[2001:db8::1]:10000" // This conversion is without ports so this should fail
};

// --------------------------------------------------------------------------

static char *ip_addr_port_fail[] = {
    "0.0.0.0.0:1234"
    "192.168.1.a:1234",
    "1.1.1.1:100000",
    "1.1.1.1:abcd",
    "abcd/1234",
    "2001:db8:1",
    "[2001:db8:1]",
    "2001:db8::x",
    "::ffff:192.168.0.a"
    "[2002:db8:::1]:1000",
    "[2001:db8::a::1#1000",
    "#2001:db8::1]:10000" // This conversion is without ports so this should fail
};

// --------------------------------------------------------------------------

static void test_ip_array(
    test_result *result,
    char *ip_addr[], int max,
    bool do_port)
{
    int cnt;
    char buff[IW_IP_BUFF_LEN];
    iw_ip address;

    // Test canned values by converting from a string to an address and
    // back again.
    for(cnt=0;cnt < max;cnt++) {
        bool retval = false;
        if(iw_ip_str_to_addr(ip_addr[cnt], do_port, &address)) {
            if(iw_ip_addr_to_str(&address, do_port, buff, sizeof(buff)) != NULL) {
                retval = strcasecmp(ip_addr[cnt], buff) == 0;
            }
        }
        test(result, retval,
             "Converting %s to IP address and back?", ip_addr[cnt]);
    }
}

// --------------------------------------------------------------------------

static void test_ip_array_fail(
    test_result *result,
    char *ip_addr[], int max,
    bool do_port)
{
    int cnt;
    iw_ip address;

    // Negative testing.
    for(cnt=0;cnt < max;cnt++) {
        bool retval = iw_ip_str_to_addr(ip_addr[cnt], do_port, &address);
        test(result, !retval,
             "Fail to convert invalid string %s?", ip_addr[cnt]);
    }
}

// --------------------------------------------------------------------------

void test_ip(test_result *result) {
    char buff[IW_IP_BUFF_LEN];
    iw_ip address;

    test_display("Testing valid IP conversions");
    test_ip_array(result, ip_addr, IW_ARR_LEN(ip_addr), false);

    test_display("Testing valid IP conversions with ports");
    test_ip_array(result, ip_addr_port, IW_ARR_LEN(ip_addr_port), true);

    test_display("Testing invalid IP conversions");
    test_ip_array_fail(result, ip_addr_fail, IW_ARR_LEN(ip_addr_fail), false);

    test_display("Testing invalid IP conversions with ports");
    test_ip_array_fail(result, ip_addr_port_fail, IW_ARR_LEN(ip_addr_port_fail), true);

    memset(&address, 0, sizeof(address));
    address.ss_family = AF_INET;

    test_display("Testing IPv4 to string");

    // Test converting from IPv4 known addresses to string
    ((struct sockaddr_in *)&address)->sin_addr.s_addr = htonl(INADDR_ANY);
    iw_ip_addr_to_str(&address, false, buff, sizeof(buff));
    test(result, strcmp(buff, "0.0.0.0") == 0,
         "Converting INADDR_ANY to 0.0.0.0?");
    ((struct sockaddr_in *)&address)->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    iw_ip_addr_to_str(&address, false, buff, sizeof(buff));
    test(result, strcmp(buff, "127.0.0.1") == 0,
         "Converting INADDR_LOOPBACK to 127.0.0.1?");
    ((struct sockaddr_in *)&address)->sin_addr.s_addr = htonl(INADDR_BROADCAST);
    iw_ip_addr_to_str(&address, false, buff, sizeof(buff));
    test(result, strcmp(buff, "255.255.255.255") == 0,
         "Converting INADDR_BROADCAST to 255.255.255.255?");
}

// --------------------------------------------------------------------------
