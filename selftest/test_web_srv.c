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

static char *favicon =
    "GET /favicon.ico HTTP/1.1\r\n"
    "Host: 127.0.0.1:8080\r\n"
    "Connection: keep-alive\r\n"
    "Accept: */*\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.65 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate, sdch\r\n"
    "Accept-Language: en-US,en;q=0.8,sv;q=0.6\r\n"
    "\r\n";

// --------------------------------------------------------------------------

static void test_value(
    test_result *result,
    const char *buff,
    iw_parse_index *value,
    const char *reference)
{
    test(result,
         iw_parse_casecmp(reference, buff, value),
         "Checking value \"%.*s\", expected \"%s\"",
         value->len, buff + value->start, reference);
}

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
         iw_parse_casecmp(name, buff, &hdr->name),
         "Getting header \"%.*s\", expected \"%s\"",
         hdr->name.len, buff + hdr->name.start, name);
    test(result,
         iw_parse_casecmp(value, buff, &hdr->value),
         "Getting value \"%.*s\", expected \"%s\"",
         hdr->value.len, buff + hdr->value.start, value);
}

// --------------------------------------------------------------------------

static void test_req(
    test_result *result,
    const char *buff,
    const char *uri,
    iw_web_req *req)
{
    test_value(result, buff, &req->version, "HTTP/1.1");
    test_value(result, buff, &req->uri, uri);
    test(result, iw_web_req_get_method(req)==IW_WEB_METHOD_GET, "GET method");
    test_hdr(result, buff, req, true, "Host", "127.0.0.1:8080");
    test_hdr(result, buff, req, true, "hOsT", "127.0.0.1:8080");
    test_hdr(result, buff, req, false, "hXsT", NULL);
}

// --------------------------------------------------------------------------
/*
static void test_req_buff(
    test_result *result,
    const char *name,
    const char *buff,
    const char *uri)
{
    iw_web_req req;
    IW_WEB_PARSE retval;

    iw_web_req_init(&req);
    test_display(name);
    retval = iw_web_req_parse(buff, strlen(buff), &req);
    test(result, retval == IW_WEB_PARSE_COMPLETE, "Parse successful");
    test_req(result, buff, uri, &req);
    iw_web_req_free(&req);
}
*/
// --------------------------------------------------------------------------

/// @brief Testing partial parsing of request.
/// This function will call the parse function multiple times before finally
/// calling the parse function with the complete request. This is done to test
/// the ability to handle partially received requests that are not
/// NUL-terminated.
/// @param result The test result structure.
/// @param name The name of the test.
/// @param buff The buffer to parse.
/// @param uri The URI that is expected in this request.
static void test_partial_req_buff(
    test_result *result,
    const char *name,
    const char *buff,
    const char *uri)
{
    iw_web_req req;
    IW_WEB_PARSE retval;

    iw_web_req_init(&req);
    test_display(name);

    unsigned int cnt;
    unsigned int tot_len = strlen(buff);
    // Do a parse call with one byte extra each time.
    for(cnt=1;cnt < tot_len;cnt++) {
        retval = iw_web_req_parse(buff, cnt, &req);
        if(retval != IW_WEB_PARSE_INCOMPLETE) {
            test(result, false, "Failed partial parsing at %d bytes", cnt);
            iw_web_req_free(&req);
            return;
        }
    }
    test(result, true, "Called parse with partial buffer [1-%d] bytes",
         tot_len - 1);

    // Do final step with the whole length (not a multiple of increment to
    // avoid missing a byte due to a rounding error.
    retval = iw_web_req_parse(buff, tot_len, &req);
    test(result, retval == IW_WEB_PARSE_COMPLETE, "Complete parse %d bytes",
         tot_len);
    test_req(result, buff, uri, &req);
    iw_web_req_free(&req);
}

// --------------------------------------------------------------------------

void test_web_srv(test_result *result) {
    test_partial_req_buff(result,
                      "Parsing basic request incrementally", basic_req, "/");
    test_partial_req_buff(result,
                      "Parsing favicon request incrementally", favicon,
                      "/favicon.ico");
}

// --------------------------------------------------------------------------
