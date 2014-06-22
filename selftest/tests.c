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
    { test_list, "List Test" },
    { test_buff, "Buffer Test" },
    { test_hash_table, "Hash Table Test" },
    { test_syslog, "Syslog Ring Buffer Test" },
    { NULL, NULL }
};

// --------------------------------------------------------------------------
