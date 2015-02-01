// --------------------------------------------------------------------------
///
/// @file test_syslog.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_syslog.h"

#include "tests.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// --------------------------------------------------------------------------

static void test_syslog_add(const char *msg) {
    test_display("iw_syslog, adding '%s'", msg);
    iw_syslog(LOG_INFO, msg);
}

// --------------------------------------------------------------------------

static void test_syslog_check(test_result *result, int cmp_size, ...) {
    va_list ap;
    FILE *out;
    char *buff;
    size_t size;

    out = open_memstream(&buff, &size);
    iw_syslog_display(out);
    fclose(out);

    // Find each message
    int cnt = 0;
    int found = 0;
    char *ptr = buff, *end;
    va_start(ap, cmp_size);
    for(;cnt < cmp_size && ptr - buff < size;cnt++) {
        while(ptr - buff < size && *ptr != 'X') {
            ptr++;
        }
        end = ptr;
        while(end - buff < size && *end != '\n') {
            end++;
        }
        char *cmp = va_arg(ap, char *);
        int len = strlen(cmp);
        if(strncmp(cmp, ptr, len) == 0 && (end - ptr) == len) {
            test(result, true, "Expected \"%s\", found \"%.*s\"",
                 cmp, (int)(end - ptr), ptr);
            found++;
        } else {
            test(result, false, "Expected \"%s\", found \"%.*s\"",
                 cmp, (int)(end - ptr), ptr);
        }
        ptr = end;
    }
    va_end(ap);

    free(buff);
}

// --------------------------------------------------------------------------

void test_syslog(test_result *result) {
    // We have internal knowledge of the syslog implementation and will use
    // that to create our tests. First, each entry size will depend on the
    // size of an int as well as the size of the timeval struct. So we will
    // call syslog_exit() to close the existing buffer and init() to create
    // a new buffer with our desired size.
    int hdr_size = sizeof(int) + sizeof(struct timeval);

    // Allocate a buffer big enough to fit 3 messages with 3 characters in
    // each message and one NUL byte.
    int size = 3 * (hdr_size + 4);
    iw_syslog_reinit(size);

    test_syslog_add("XA1");
    test_syslog_check(result, 1, "XA1");

    test_syslog_add("XA2");
    test_syslog_check(result, 2, "XA1", "XA2");

    test_syslog_add("XA3");
    test_syslog_check(result, 3, "XA1", "XA2", "XA3");

    test_syslog_add("XA4");
    test_syslog_check(result, 3, "XA2", "XA3", "XA4");

    test_syslog_add("XA5");
    test_syslog_check(result, 3, "XA3", "XA4", "XA5");

    test_syslog_add("XA6");
    test_syslog_check(result, 3, "XA4", "XA5", "XA6");

    test_syslog_add("X1");
    test_syslog_check(result, 3, "XA5", "XA6", "X1");

    test_syslog_add("X2");
    test_syslog_check(result, 3, "XA6", "X1", "X2");

    test_syslog_add("X3");
    test_syslog_check(result, 3, "X1", "X2", "X3");

    test_syslog_add("XB1");
    test_syslog_check(result, 2, "X3", "XB1");

    test_syslog_add("XB2");
    test_syslog_check(result, 2, "XB1", "XB2");

    test_syslog_add("XB3");
    test_syslog_check(result, 3, "XB1", "XB2", "XB3");

    // The following message is larger than the whole buffer so the
    // content should not change.
    test_syslog_add("Xabcdefghijklmnopqrstuvwxyz012345678901234567890123456789");
    test_syslog_check(result, 3, "XB1", "XB2", "XB3");

    test_syslog_add("Xabcdefghijklmnopqrstuvwxyz0");
    test_syslog_check(result, 1, "Xabcdefghijklmnopqrstuvwxyz0");
}

// --------------------------------------------------------------------------
