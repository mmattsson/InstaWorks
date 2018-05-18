// --------------------------------------------------------------------------
///
/// @file iw_hash.c
///
/// Using hash function written by Dan Bernstein.
///
// --------------------------------------------------------------------------

#include "iw_hash.h"

#include <stdlib.h>

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

unsigned long iw_hash_data(unsigned int len, const void *data) {
    unsigned long hash = 5381;
    unsigned int c, cnt;
    const unsigned char *ptr = data;

    if(data == NULL) {
        return 0;
    }

    for(cnt=0;cnt < len;cnt++) {
        c = *ptr++;
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

// --------------------------------------------------------------------------
