// --------------------------------------------------------------------------
///
/// @file iw_util.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_util.h"

#include <ctype.h>
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
    if(*endptr != '\0' && !isspace(*endptr)) {
        // If the endptr isn't the end of the string or a space, then consider
        // the digit invalid.
        return false;
    }

    return true;
}

// --------------------------------------------------------------------------
