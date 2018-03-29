// --------------------------------------------------------------------------
///
/// @file iw_util.c
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_util.h"

#include "iw_memory.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

bool iw_util_strtoll(const char *str, long long int *number, unsigned int base) {
    char *endptr;
    errno = 0;
    *number = strtoll(str, &endptr, base);
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

char *iw_util_concat(int num, ...) {
    va_list ap;
    int cnt;
    size_t tot_len = 0, len;
    char *result = NULL;

    va_start(ap, num);
    for(cnt=0;cnt < num;cnt++) {
        char *str = va_arg(ap, char *);
        if(str == NULL) {
            break;
        }
        len = strlen(str);
        result = IW_REALLOC(result, tot_len + len + 1);
        if(result == NULL) {
            break;
        }
        memcpy(result + tot_len, str, len);

        tot_len += len;
    }
    va_end(ap);

    // NUL-terminate result
    if(result != NULL) {
        *(result + tot_len) = '\0';
    }

    return result;
}

// --------------------------------------------------------------------------
