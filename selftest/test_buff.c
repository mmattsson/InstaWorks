// --------------------------------------------------------------------------
///
/// @file test_buff.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_buff.h"

#include "tests.h"

#include <string.h>

// --------------------------------------------------------------------------

void test_buff(test_result *result) {
    iw_buff buff;

    iw_buff_create(&buff, 4, 8);

    test(result, buff.end == 0, "Initalized buffer has no data");
    test(result, buff.size == 4, "Initalized buffer has 4 bytes size");
    test(result, buff.max_size == 8, "Initalized buffer has 8 bytes max size");

    char test1[] = "ab";
    char test2[] = "cd";
    char test3[] = "ef";
    char test4[] = "gh";
    char test5[] = "ij";
    char test6[] = "kl";

    bool retval;
    retval = iw_buff_add_data(&buff, test1, 2);
    test_display("iw_buff_add_data, adding '%s'", test1);
    test(result, retval, "iw_buff_add_data succeeded");
    test(result, buff.size == 4 && buff.end == 2,
         "Buffer size is 4 and end is 2");
    test(result, strncmp(buff.buff, "ab", 2) == 0,
         "Buffer contains 'ab'");

    retval = iw_buff_add_data(&buff, test2, 2);
    test_display("iw_buff_add_data, adding '%s'", test2);
    test(result, retval, "iw_buff_add_data succeeded");
    test(result, buff.size == 4 && buff.end == 4,
         "Buffer size is 4 and end is 4");
    test(result, strncmp(buff.buff, "abcd", 4) == 0,
         "Buffer contains 'abcd'");

    iw_buff_remove_data(&buff, 2);
    test_display("iw_buff_remove_data, removing 2 bytes");
    test(result, buff.size == 4 && buff.end == 2,
         "Buffer size is 4 and end is 2");
    test(result, strncmp(buff.buff, "cd", 2) == 0,
         "Buffer contains 'cd'");

    retval = iw_buff_add_data(&buff, test3, 2);
    test_display("iw_buff_add_data, adding '%s'", test3);
    test(result, retval, "iw_buff_add_data succeeded");
    test(result, buff.size == 4 && buff.end == 4,
         "Buffer size is 4 and end is 4");
    test(result, strncmp(buff.buff, "cdef", 4) == 0,
         "Buffer contains 'cdef'");

    retval = iw_buff_add_data(&buff, test4, 2);
    test_display("iw_buff_add_data, adding '%s'", test4);
    test(result, retval, "iw_buff_add_data succeeded");
    test(result, buff.size == 6 && buff.end == 6,
         "Buffer size is 6 and end is 6");
    test(result, strncmp(buff.buff, "cdefgh", 6) == 0,
         "Buffer contains 'cdefgh'");

    retval = iw_buff_add_data(&buff, test5, 2);
    test_display("iw_buff_add_data, adding '%s'", test5);
    test(result, retval, "iw_buff_add_data succeeded");
    test(result, buff.size == 8 && buff.end == 8,
         "Buffer size is 8 and end is 8");
    test(result, strncmp(buff.buff, "cdefghij", 8) == 0,
         "Buffer contains 'cdefghij'");

    retval = iw_buff_add_data(&buff, test6, 2);
    test_display("iw_buff_add_data, adding '%s'", test6);
    test(result, !retval, "iw_buff_add_data failed");
    test(result, buff.size == 8 && buff.end == 8,
         "Buffer size is 8 and end is 8");
    test(result, strncmp(buff.buff, "cdefghij", 8) == 0,
         "Buffer contains 'cdefghij'");

    iw_buff_destroy(&buff);
}

// --------------------------------------------------------------------------
