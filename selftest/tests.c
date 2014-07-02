// --------------------------------------------------------------------------
///
/// @file tests.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "tests.h"

#include <stdio.h>

// --------------------------------------------------------------------------

test_info s_tests[] = {
    { test_list,        "List test" },
    { test_buff,        "Buffer test" },
    { test_hash_table,  "Hash table test" },
    { test_syslog,      "Syslog ring buffer test" },
    { test_opts,        "Command-line option parsing test" },
    { test_ip,          "IP address utility test" },
    { NULL, NULL }
};

// --------------------------------------------------------------------------
