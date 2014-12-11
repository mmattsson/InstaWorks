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
    { test_list,        "list",     "List test" },
    { test_buff,        "buffer",   "Buffer test" },
    { test_hash_table,  "hash",     "Hash table test" },
    { test_syslog,      "syslog",   "Syslog ring buffer test" },
    { test_opts,        "cli",      "Command-line option parsing test" },
    { test_ip,          "ip",       "IP address utility test" },
    { test_web_srv,     "web",      "Web server parsing test" },
    { NULL, NULL, NULL }
};

// --------------------------------------------------------------------------
