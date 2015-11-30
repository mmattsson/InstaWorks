// --------------------------------------------------------------------------
///
/// @file test_util.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_util.h"

#include "iw_memory.h"

#include "tests.h"

#include <string.h>

// --------------------------------------------------------------------------

static void test_good_dec(test_result *result, const char *str, int num) {
    long long int tmp;
    test(result, iw_util_strtoll(str, &tmp, 10), "Converting '%s' successful?", str);
}

// --------------------------------------------------------------------------

static void test_bad_dec(test_result *result, const char *str) {
    long long int tmp;
    test(result, !iw_util_strtoll(str, &tmp, 10), "Converting '%s' fails?", str);
}

// --------------------------------------------------------------------------

static void test_good_hex(test_result *result, const char *str, int num) {
    long long int tmp;
    test(result, iw_util_strtoll(str, &tmp, 16), "Converting '%s' successful?", str);
}

// --------------------------------------------------------------------------

static void test_bad_hex(test_result *result, const char *str) {
    long long int tmp;
    test(result, !iw_util_strtoll(str, &tmp, 16), "Converting '%s' fails?", str);
}

// --------------------------------------------------------------------------

static void test_strtoll(test_result *result) {
    long long int num;

    bool retval = iw_util_strtoll("01234", &num, 10);
    if(num == 01234) {
        test_display("true\n");
    }

    test_display("Converting decimal strings to integers");
    test_good_dec(result, "1", 1);
    test_good_dec(result, "12", 12);
    test_good_dec(result, "123", 123);
    test_good_dec(result, "123456", 123456);
    test_good_dec(result, "99999999", 99999999);
    test_good_dec(result, "0", 0);
    test_good_dec(result, "01234", 1234);
    test_good_dec(result, "-1", -1);
    test_good_dec(result, "-99999999", -99999999);
    test_bad_dec(result, "0.123");
    test_bad_dec(result, "abcdef");
    test_bad_dec(result, "0xabcdef");
    test_bad_dec(result, "0xdefghi");
    test_bad_dec(result, "0x0123");

    test_display("Converting hexadecimal strings to integers");
    test_good_hex(result, "1", 1);
    test_good_hex(result, "12", 12);
    test_good_hex(result, "123", 123);
    test_good_hex(result, "123456", 123456);
    test_good_hex(result, "99999999", 99999999);
    test_good_hex(result, "0", 0);
    test_good_hex(result, "01234", 1234);
    test_good_hex(result, "-1", -1);
    test_good_hex(result, "-99999999", -99999999);
    test_good_hex(result, "0xabcdef", 0xabcdef);
    test_good_hex(result, "0x0123", 0x0123);
    test_bad_hex(result, "0.123");
    test_bad_hex(result, "0xdefghi");
}

// --------------------------------------------------------------------------

static void test_concat(test_result *result) {
    char *str = iw_util_concat(3, "a", "b", "c");
    test(result, strcmp(str, "abc") == 0, "Concatenating 'a', 'b', and 'c' gives 'abc'?");
    IW_FREE(str);
    str = iw_util_concat(7, "a", "b", "c", "d", "e", "f", "g");
    test(result, strcmp(str, "abcdefg") == 0, "Concatenating 'a', 'b', ... 'g' gives 'abcdefg'?");
    IW_FREE(str);
    str = iw_util_concat(2, "abcd", "efg");
    test(result, strcmp(str, "abcdefg") == 0, "Concatenating 'abcd', 'efg' gives 'abcdefg'?");
    IW_FREE(str);
    str = iw_util_concat(0);
    test(result, str == NULL, "Concatenating zero args give NULL?");
    IW_FREE(str);
    str = iw_util_concat(1, "abcd");
    test(result, strcmp(str, "abcd") == 0, "Concatenating 'abcd' gives 'abcd'?");
    IW_FREE(str);
}

// --------------------------------------------------------------------------

void test_util(test_result *result) {
    test_display("Testing function iw_util_strtoll()");
    test_strtoll(result);

    test_display("Testing function iw_util_concat()");
    test_concat(result);
}

// --------------------------------------------------------------------------
