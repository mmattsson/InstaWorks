// --------------------------------------------------------------------------
///
/// @file test_web_srv.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_web_req.h"
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

static void test_hdr(
    test_result *result,
    const char *buff,
    iw_web_req *req,
    bool present,
    const char *name,
    const char *value)
{
    iw_web_req_header *hdr;
    hdr = iw_web_req_get_header(buff, req, name);
    if(hdr == NULL) {
        if(present) {
            // The header should be present, the fact that we didn't find it
            // means this test failed.
            test(result, false, "Failed to get header \"%s\"", name);
        } else {
            // The header should not be present. The fact that we didn't find it
            // means that this test passed.
            test(result, true,
                 "Could not get non-existent header \"%s\"", name);
        }
        return;
    }
    test(result,
         strncasecmp(buff + hdr->name.start, name, hdr->name.len) == 0,
         "Getting header \"%.*s\", expected \"%s\"",
         hdr->name.len, buff + hdr->name.start, name);
    test(result,
         strncasecmp(buff + hdr->value.start, value, hdr->value.len) == 0,
         "Getting value \"%.*s\", expected \"%s\"",
         hdr->value.len, buff + hdr->value.start, value);
}

// --------------------------------------------------------------------------

void test_web_srv(test_result *result) {
    iw_web_req req;
    IW_WEB_PARSE retval;

    test_display("Parsing basic request");
    retval = iw_web_req_parse_str(basic_req, &req);
    test(result, retval == IW_WEB_PARSE_COMPLETE, "Parse successful");
    test(result, iw_web_req_get_method(&req) == IW_WEB_METHOD_GET, "GET method");
    test_hdr(result, basic_req, &req, true, "Host", "127.0.0.1:8080");
    test_hdr(result, basic_req, &req, true, "hOsT", "127.0.0.1:8080");
    test_hdr(result, basic_req, &req, false, "hXsT", NULL);
    iw_web_req_free(&req);

    test_display("Parsing favico request");
    retval = iw_web_req_parse_str(favico, &req);
    test(result, retval == IW_WEB_PARSE_COMPLETE, "Parse successful");
    test(result, iw_web_req_get_method(&req) == IW_WEB_METHOD_GET, "GET method");
    test_hdr(result, favico, &req, true, "Host", "127.0.0.1:8080");
    iw_web_req_free(&req);
}

// --------------------------------------------------------------------------
