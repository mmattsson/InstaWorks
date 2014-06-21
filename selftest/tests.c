// --------------------------------------------------------------------------
///
/// @file tests.c
///
/// Copyright (C) Mattias Mattsson - 2014
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
