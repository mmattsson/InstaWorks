// --------------------------------------------------------------------------
///
/// @file iw_parse.c
///
/// Module for parsing text.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_parse.h"

#include <ctype.h>
#include <string.h>

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

IW_PARSE iw_parse_is_token(
    const char *buff,
    unsigned int *offset,
    const char *sep)
{
    const char *start = buff + *offset;
    if(strncmp(start, sep, strlen(sep)) != 0) {
        return IW_PARSE_NO_MATCH;
    }

    *offset += strlen(sep);
    return IW_PARSE_MATCH;
}

// --------------------------------------------------------------------------

IW_PARSE iw_parse_find_token(
    const char *buff,
    unsigned int *offset,
    const char *sep)
{
    const char *start = buff + *offset;
    char *end = strstr(start, sep);
    if(end == NULL) {
        return IW_PARSE_NO_MATCH;
    }

    *offset = end - buff + strlen(sep);
    return IW_PARSE_MATCH;
}

// --------------------------------------------------------------------------

IW_PARSE iw_parse_read_token(
    const char *buff,
    unsigned int *offset,
    const char *sep,
    bool trim,
    iw_parse_index *index)
{
    const char *start = buff + *offset;

    // We need to take care to not skip over a separator we are searching for
    // when trimming whitespace (e.g. if the separator is a space). We do this
    // by first finding the separator and then trim, but only trim while the
    // pointer has not reached the separator.
    char *end = strstr(start, sep);
    if(end == NULL) {
        return IW_PARSE_NO_MATCH;
    }

    *offset = end - buff + strlen(sep);
    if(trim) {
        // If we should trim, start with trimming the leading spaces
        if(isblank(*start) && start < end) {
            start++;
        }

        // Then trim the end
        if(isblank(*end) && end > start) {
            end--;
        }
    }

    // Set the index values.
    index->start = start - buff;
    index->len   = end - start;

    return IW_PARSE_MATCH;
}

// --------------------------------------------------------------------------
