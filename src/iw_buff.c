// --------------------------------------------------------------------------
///
/// @file iw_buff.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_buff.h"

#include "iw_log.h"
#include "iw_memory.h"

#include <stdlib.h>
#include <string.h>

// TODO: Add chunk size to buffer instead of growing one byte at the time.

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

bool iw_buff_create(
    iw_buff *buff,
    unsigned int initial_size,
    unsigned int max_size)
{
    memset(buff, 0, sizeof(*buff));
    buff->buff = (char *)IW_CALLOC(1, initial_size);
    if(buff->buff == NULL) {
        LOG(IW_LOG_IW, "Failed to create buffer of size %d", initial_size);
        return false;
    }

    buff->end        = 0;
    buff->size       = initial_size;
    buff->max_size   = max_size;

    return true;
}

// --------------------------------------------------------------------------

void iw_buff_destroy(iw_buff *buff) {
    IW_FREE(buff->buff);
    memset(buff, 0, sizeof(*buff));
}

// --------------------------------------------------------------------------

bool iw_buff_add_data(iw_buff *buff, char *data, unsigned int size) {
    char *ptr;
    if(!iw_buff_reserve_data(buff, &ptr, size)) {
        return false;
    }

    memcpy(ptr, data, size);
    iw_buff_commit_data(buff, size);

    return true;
}

// --------------------------------------------------------------------------

bool iw_buff_reserve_data(iw_buff *buff, char **data, unsigned int size) {
    int remainder = iw_buff_remainder(buff);
    if(size <= remainder) {
        // No problem, we got space enough
        *data = buff->buff + buff->end;
        return true;
    } else if(buff->size < buff->max_size) {
        // Not enough space, see if we can reallocate the buffer.
        int new_size = buff->size + (size - remainder);
        char *ptr = (char *)IW_REALLOC(buff->buff, new_size);
        if(ptr == NULL) {
            // Couldn't realloc
            LOG(IW_LOG_IW, "Failed to reallocate buffer from %d to %d bytes",
                buff->size, new_size);
            return false;
        }
        buff->buff = ptr;
        buff->size = new_size;
        *data = buff->buff + buff->end;
        return true;
    } else {
        // Can't grow buffer, return false
        *data = NULL;
        return false;
    }
}

// --------------------------------------------------------------------------

void iw_buff_commit_data(iw_buff *buff, unsigned int size) {
    buff->end += size;
}

// --------------------------------------------------------------------------

void iw_buff_remove_data(iw_buff *buff, unsigned int size) {
    memmove(buff->buff, buff->buff + size, buff->end - size);
    buff->end -= size;
}

// --------------------------------------------------------------------------

int iw_buff_remainder(const iw_buff *buff) {
    return buff->size - buff->end;
}

// --------------------------------------------------------------------------
