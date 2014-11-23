// --------------------------------------------------------------------------
///
/// @file test_web_srv.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_web_srv.h"

#include "tests.h"

#include <string.h>

static char *basic_req =
    "GET / HTTP/1.1\r\n"
    "Host: 127.0.0.1:8080\r\n"
    "Connection: keep-alive\r\n"
    "Cache-Control: max-age=0\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.65 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate, sdch\r\n"
    "Accept-Language: en-US,en;q=0.8,sv;q=0.6\r\n"
    "\r\n";

static char *favico =
    "GET /favicon.ico HTTP/1.1\r\n"
    "Host: 127.0.0.1:8080\r\n"
    "Connection: keep-alive\r\n"
    "Accept: */*\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.65 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate, sdch\r\n"
    "Accept-Language: en-US,en;q=0.8,sv;q=0.6\r\n"
    "\r\n";

// --------------------------------------------------------------------------

void test_web_srv(test_result *result) {
    iw_web_req req;
    IW_WEB_PARSE retval;

    test_display("Parsing basic request");
    retval = iw_web_srv_parse_request_str(basic_req, &req);
    test(result, retval == IW_WEB_PARSE_COMPLETE, "Parse successful");
    iw_web_srv_free_request(&req);

    test_display("Parsing favico request");
    retval = iw_web_srv_parse_request_str(favico, &req);
    test(result, retval == IW_WEB_PARSE_COMPLETE, "Parse successful");
    iw_web_srv_free_request(&req);
    /*
    test_display("iw_buff_add_data, adding '%s'", test6);
    test(result, !retval, "iw_buff_add_data failed");
    test(result, buff.size == 8 && buff.end == 8,
         "Buffer size is 8 and end is 8");
    test(result, strncmp(buff.buff, "cdefghij", 8) == 0,
         "Buffer contains 'cdefghij'");
         */
}

// --------------------------------------------------------------------------
