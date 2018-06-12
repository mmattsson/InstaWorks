// --------------------------------------------------------------------------
///
/// @file test_web_srv.c
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_util.h"
#include "iw_web_req.h"
#include "iw_web_srv.h"

#include "tests.h"

#include <string.h>

// --------------------------------------------------------------------------
//
// HTTP request test structure.
//
// --------------------------------------------------------------------------

/// @brief The HTTP request test structure.
/// Used to define a test for a given HTTP request.
typedef struct _req_test {
    const char   *req;      ///< The request to parse.
    IW_WEB_METHOD method;   ///< The expected method.
    const char   *path;     ///< The expected request path.
    const char   *values[]; ///< The header and parameter name/value pairs.
} req_test;

// --------------------------------------------------------------------------
//
// Sample HTTP requests for testing.
//
// --------------------------------------------------------------------------

static req_test req_uri_1 = {
    "POST /?%24a=1&%24b=2 HTTP/1.1\r\n"
    "\r\n",
    IW_WEB_METHOD_POST,
    "/",
    {
    NULL, NULL,
    "$a", "1",
    "$b", "2",
    NULL, NULL,
    }
};

// --------------------------------------------------------------------------

static req_test req_basic = {
    "GET / HTTP/1.1\r\n"
    "Host: 127.0.0.1:8080\r\n"
    "Connection: keep-alive\r\n"
    "Cache-Control: max-age=0\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.65 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate, sdch\r\n"
    "Accept-Language: en-US,en;q=0.8,sv;q=0.6\r\n"
    "\r\n",
    IW_WEB_METHOD_GET,
    "/",
    {
    "hOsT", "127.0.0.1:8080", // Testing different capitalization
    "hxst", NULL, // The header is not present
    "Connection", "keep-alive",
    "Cache-Control", "max-age=0",
    "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
    "User-agent", "Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.65 Safari/537.36",
    "Accept-Encoding", "gzip, deflate, sdch",
    "Accept-Language", "en-US,en;q=0.8,sv;q=0.6",
    NULL, NULL,
    NULL, NULL
    }
};

// --------------------------------------------------------------------------

static req_test req_favicon = {
    "GET /favicon.ico HTTP/1.1\r\n"
    "Host: 127.0.0.1:8080\r\n"
    "Connection: keep-alive\r\n"
    "Accept: */*\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.65 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate, sdch\r\n"
    "Accept-Language: en-US,en;q=0.8,sv;q=0.6\r\n"
    "\r\n",
    IW_WEB_METHOD_GET,
    "/favicon.ico",
    {
    "hOsT", "127.0.0.1:8080", // Testing different capitalization
    "hxst", NULL, // The header is not present
    "Connection", "keep-alive",
    "Accept", "*/*",
    "User-agent", "Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.65 Safari/537.36",
    "Accept-Encoding", "gzip, deflate, sdch",
    "Accept-Language", "en-US,en;q=0.8,sv;q=0.6",
    NULL, NULL,
    NULL, NULL
    }
};

// --------------------------------------------------------------------------

static req_test req_get_form = {
    "GET /Configuration?noval=&cfg.crashhandler.file=%2Ftmp%2Fcallstack.txt&cfg.opt.loglvl=l&cfg.loglvl=16&cfg.memtrack.enable=1&cfg.syslog.size=10000&cfg.allowquit=1&cfg.webgui.enable=1&cfg.cmdport=10000&cfg.daemonize=0&cfg.daemonize.opt=d&cfg.memtrack.size=10000&cfg.foreground=1&cfg.crashhandler.enable=1&cfg.prgname=simple&cfg.healthcheck.enable=1&cfg.webgui.css=%2Ftmp%2Fsimple.css&cfg.opt.foreground=f&Apply=Submit HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Connection: keep-alive\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux i686 (x86_64)) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.91 Safari/537.36\r\n"
    "Referer: http://localhost:8080/Configuration\r\n"
    "Accept-Encoding: gzip, deflate, sdch\r\n"
    "Accept-Language: en-US,en;q=0.8,sv;q=0.6\r\n"
    "\r\n",
    IW_WEB_METHOD_GET,
    "/Configuration",
    {
    "hOst", "localhost:8080", // Testing different capitalization
    "hxst", NULL, // The header is not present
    "Connection", "keep-alive",
    "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
    "User-agent", "Mozilla/5.0 (X11; Linux i686 (x86_64)) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.91 Safari/537.36",
    "Referer", "http://localhost:8080/Configuration",
    "Accept-Encoding", "gzip, deflate, sdch",
    "Accept-Language", "en-US,en;q=0.8,sv;q=0.6",

    NULL, NULL,
    "noval", "", // The param is present, it just has no value (empty string)
    "cfg.crashhandler.file", "/tmp/callstack.txt",
    "cfg.opt.loglvl", "l",
    "cfg.loglvl", "16",
    "cfg.memtrack.enable", "1",
    "cfg.syslog.size", "10000",
    "cfg.allowquit", "1",
    "cfg.webgui.enable", "1",
    "cfg.cmdport", "10000",
    "cfg.daemonize", "0",
    "cfg.daemonize.opt", "d",
    "cfg.memtrack.size", "10000",
    "cfg.foreground", "1",
    "cfg.crashhandler.enable", "1",
    "cfg.prgname", "simple",
    "cfg.healthcheck.enable", "1",
    "cfg.webgui.css", "/tmp/simple.css",
    "cfg.opt.foreground", "f",
    "Apply", "Submit",
    NULL, NULL
    }
};

// --------------------------------------------------------------------------
static req_test req_post_form = {
    "POST /Configuration HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Connection: keep-alive\r\n"
    "Content-Length: 389\r\n"
    "Cache-Control: max-age=0\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "Origin: http://localhost:8080\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux i686 (x86_64)) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.94 Safari/537.36\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Referer: http://localhost:8080/Configuration\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: en-US,en;q=0.8,sv;q=0.6\r\n"
    "\r\n"
    "cfg.crashhandler.file=%2Ftmp%2Fcallstack.txt&cfg.opt.loglvl=l&cfg.loglvl=31&cfg.memtrack.enable=1&cfg.syslog.size=10000&cfg.allowquit=1&cfg.webgui.enable=1&cfg.cmdport=10000&cfg.daemonize=0&cfg.daemonize.opt=d&cfg.memtrack.size=10000&cfg.foreground=1&cfg.crashhandler.enable=1&cfg.prgname=simple&cfg.healthcheck.enable=1&cfg.webgui.css=%2Ftmp%2Fsimple.css&cfg.opt.foreground=f&Apply=Submit",
    IW_WEB_METHOD_POST,
    "/Configuration",
    {
    "hOst", "localhost:8080", // Testing different capitalization
    "hxst", NULL, // The header is not present
    "Connection", "keep-alive",
    "Content-Length", "389",
    "Cache-Control", "max-age=0",
    "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
    "Origin", "http://localhost:8080",
    "User-agent", "Mozilla/5.0 (X11; Linux i686 (x86_64)) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.94 Safari/537.36",
    "Content-Type", "application/x-www-form-urlencoded",
    "Referer", "http://localhost:8080/Configuration",
    "Accept-Encoding", "gzip, deflate",
    "Accept-Language", "en-US,en;q=0.8,sv;q=0.6",
    NULL, NULL,
    "cfg.crashhandler.file", "/tmp/callstack.txt",
    "cfg.opt.loglvl", "l",
    "cfg.loglvl", "31",
    "cfg.memtrack.enable", "1",
    "cfg.syslog.size", "10000",
    "cfg.allowquit", "1",
    "cfg.webgui.enable", "1",
    "cfg.cmdport", "10000",
    "cfg.daemonize", "0",
    "cfg.daemonize.opt", "d",
    "cfg.memtrack.size", "10000",
    "cfg.foreground", "1",
    "cfg.crashhandler.enable", "1",
    "cfg.prgname", "simple",
    "cfg.healthcheck.enable", "1",
    "cfg.webgui.css", "/tmp/simple.css",
    "cfg.opt.foreground", "f",
    "Apply", "Submit",
    NULL, NULL
    }
};

// --------------------------------------------------------------------------

static req_test req_post_mix = {
    "POST /?uri1=123&uri2=456&uri3=abc HTTP/1.1\r\n"
    "Content-Length: 29\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "\r\n"
    "body1=abc&body2=def&body3=123",
    IW_WEB_METHOD_POST,
    "/",
    {
    "Content-Length", "29",
    "Content-Type", "application/x-www-form-urlencoded",
    NULL, NULL,
    "uri1", "123",
    "uri2", "456",
    "uri3", "abc",
    "body1", "abc",
    "body2", "def",
    "body3", "123",
    NULL, NULL
    }
};

// --------------------------------------------------------------------------

static void test_index(
    test_result *result,
    const char *buff,
    iw_parse_index *index,
    const char *reference)
{
    test(result,
         iw_parse_casecmp(reference, buff, index),
         "Checking value \"%.*s\", expected \"%s\"",
         index->len, buff + index->start, reference);
}

// --------------------------------------------------------------------------

static void test_header(
    test_result *result,
    iw_web_req *req,
    iw_web_req_header *hdr,
    const char *name,
    const char *value)
{
    if(hdr == NULL) {
        // No header given, we should try to get it here.
        hdr = iw_web_req_get_header(req, name);
        if(hdr == NULL) {
            if(value != NULL) {
                // The header should be present, the fact that we didn't find it
                // means this test failed.
                test(result, false, "Failed to get header \"%s\"", name);
            } else {
                // The header should not be present. The fact that we didn't find it
                // means that this test passed.
                test(result, true, "Could not get non-existent header \"%s\"",
                    name);
            }
            return;
        }
    }
    test(result,
         iw_parse_casecmp(name, req->buff, &hdr->name),
         "HDR name=\"%.*s\" expect=\"%s\"",
         hdr->name.len, req->buff + hdr->name.start, name);
    test(result,
         iw_parse_casecmp(value, req->buff, &hdr->value),
         "HDR value=\"%.*s\" expect=\"%s\"",
         hdr->value.len, req->buff + hdr->value.start, value);
}

// --------------------------------------------------------------------------

static void test_param(
    test_result *result,
    iw_web_req *req,
    iw_web_req_parameter *param,
    const char *name,
    const char *value)
{
    if(param == NULL) {
        // No parameter given, we should try to get it here.
        param = iw_web_req_get_parameter(req, name);
        if(param == NULL) {
            if(value != NULL) {
                // The header should be present, the fact that we didn't find it
                // means this test failed.
                test(result, false, "Failed to get parameter \"%s\"", name);
            } else {
                // The header should not be present. The fact that we didn't find it
                // means that this test passed.
                test(result, true, "Could not get non-existent parameter \"%s\"",
                    name);
            }
            return;
        }
    }
    test(result,
         strcasecmp(name, param->name) == 0,
         "PRM name=\"%s\" expect=\"%s\"",
         param->name, name);
    test(result,
         strcasecmp(value, param->value) == 0,
         "PRM value=\"%s\" expect=\"%s\"",
         param->value, value);
}

// --------------------------------------------------------------------------

static void test_req(
    test_result *result,
    req_test *rtest,
    iw_web_req *req)
{
    test_index(result, req->buff, &req->version, "HTTP/1.1");
    if(rtest->path != NULL) {
        test_index(result, req->buff, &req->path, rtest->path);
    }
    test(result, iw_web_req_get_method(req)==rtest->method,
         "Got method '%s', expected '%s'",
         iw_web_req_method_str(iw_web_req_get_method(req)),
         iw_web_req_method_str(rtest->method));

    // Go through the headers in the test. All headers up to the first NULL
    // name will be headers
    test_display("Test headers by name");
    int cnt;
    for(cnt=0;rtest->values[cnt] != NULL;cnt+=2) {
        test_header(result, req, NULL,
                   rtest->values[cnt],
                   rtest->values[cnt+1]);
    }

    // Test again, but iterate through all headers rather than getting the
    // headers by name
    test_display("Test headers by iteration");
    int cnt2;
    iw_web_req_header *hdr = iw_web_req_get_header(req, NULL);
    for(cnt2=0;hdr != NULL && rtest->values[cnt2] != NULL;cnt2+=2) {
        // We need to skip tests for non-existent headers when iterating
        // through headers.
        if(rtest->values[cnt2+1] != NULL) {
            test_header(result, req, hdr,
                        rtest->values[cnt2],
                        rtest->values[cnt2+1]);
            hdr = iw_web_req_get_next_header(req, NULL, hdr);
        }
    }

    // All values after the first NULL name will be parameters.
    // We continue from the point of cnt where we stopped with the header
    // testing (plus one to account for the NULL name).
    test_display("Test parameters by name");
    for(cnt+=2;rtest->values[cnt] != NULL;cnt+=2) {
        test_param(result, req, NULL,
                   rtest->values[cnt],
                   rtest->values[cnt+1]);
    }

    // Test again, but iterate through all parameters rather than getting the
    // parameters by name
    test_display("Test parameters by iteration");
    iw_web_req_parameter *param = iw_web_req_get_parameter(req, NULL);
    for(cnt2+=2;param != NULL && rtest->values[cnt2] != NULL;cnt2+=2) {
        test_param(result, req, param,
                   rtest->values[cnt2],
                   rtest->values[cnt2+1]);
        param = iw_web_req_get_next_parameter(NULL, param);
    }
}

// --------------------------------------------------------------------------

/// @brief Testing partial parsing of request.
/// This function will call the parse function multiple times before finally
/// calling the parse function with the complete request. This is done to test
/// the ability to handle partially received requests that are not
/// NUL-terminated.
/// @param result The test result structure.
/// @param name The name of the test.
/// @param rtest The test to perform.
static void test_req_buff(
    test_result *result,
    const char *name,
    req_test *rtest)
{
    IW_WEB_PARSE retval;
    iw_web_req req;

    iw_web_req_init(&req);
    test_display(name);

    unsigned int cnt;
    unsigned int tot_len = strlen(rtest->req);
    // Do a parse call with one byte extra each time.
    for(cnt=1;cnt < tot_len;cnt++) {
        req.buff = (char *)rtest->req;
        req.len  = cnt;
        retval = iw_web_req_parse(&req);
        if(retval != IW_WEB_PARSE_INCOMPLETE) {
            test(result, false, "Failed partial parsing at %d bytes", cnt);
            iw_web_req_free(&req);
            return;
        }
    }
    test(result, true, "Called parse with partial buffer [1-%d] bytes",
         tot_len - 1);

    // Do final step with the whole length.
    req.buff = (char *)rtest->req;
    req.len  = tot_len;
    retval = iw_web_req_parse(&req);
    test(result, retval == IW_WEB_PARSE_COMPLETE, "Complete parse successful");
    test(result, req.parse_point == tot_len, "Parsing read %d bytes", tot_len);
    test_req(result, rtest, &req);
    iw_web_req_free(&req);
}

// --------------------------------------------------------------------------

/// @brief Test a complete request buffer.
/// Used for debugging and adding new tests. Testing the complete buffer at
/// once simplifies the testing of a new request. Once the new request parses
/// successfully, the test_partial_req_buff() should be used to ensure that
/// the buffer can be parsed if a request is received partially.
/// @param result The test result structure.
/// @param name The name of the test.
/// @param rtest The test to perform.
void test_complete_req_buff(
    test_result *result,
    const char *name,
    req_test *rtest)
{
    IW_WEB_PARSE retval;
    iw_web_req req;

    iw_web_req_init(&req);
    test_display(name);
    req.buff = (char *)rtest->req;
    req.len  = strlen(rtest->req);
    retval = iw_web_req_parse(&req);
    test(result, retval == IW_WEB_PARSE_COMPLETE, "Parse successful");
    test_req(result, rtest, &req);
    iw_web_req_free(&req);
}

// --------------------------------------------------------------------------

void test_web_srv(test_result *result) {
    test_req_buff(result, "Parsing URI 1", &req_uri_1);
    test_req_buff(result, "Parsing basic request", &req_basic);
    test_req_buff(result, "Parsing favicon request", &req_favicon);
    test_req_buff(result, "Parsing get form request", &req_get_form);
    test_req_buff(result, "Parsing post form request", &req_post_form);
    test_req_buff(result, "Parsing mixed post request", &req_post_mix);
}

// --------------------------------------------------------------------------
