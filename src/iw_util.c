// --------------------------------------------------------------------------
///
/// @file iw_util.c
///
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#include "iw_util.h"

#include <errno.h>
#include <stdlib.h>

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

bool iw_strtol(const char *str, long int *number, unsigned int base) {
    char *endptr;
    errno = 0;
    *number = strtol(str, &endptr, base);
    if(errno != 0) {
        // Error occurred.
        return false;
    }
    if(endptr == str) {
        // No digits found.
        return false;
    }

    return true;
}

// --------------------------------------------------------------------------
