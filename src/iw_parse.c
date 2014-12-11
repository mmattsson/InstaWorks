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

IW_PARSE iw_parse_find_token(
    const char *buff,
    unsigned int len,
    unsigned int *offset,
    const char *token)
{
    int token_len = strlen(token);
    int tmp = *offset;
    while(tmp + token_len <= len) {
        if(strncmp(buff + tmp, token, token_len) == 0) {
            // Only set offset if we find a match.
            *offset = tmp + token_len;
            return IW_PARSE_MATCH;
        }
        tmp++;
    }

    return IW_PARSE_NO_MATCH;
}

// --------------------------------------------------------------------------

IW_PARSE iw_parse_is_token(
    const char *buff,
    unsigned int len,
    unsigned int *offset,
    const char *token)
{
    int token_len = strlen(token);
    const char *start = buff + *offset;

    if(*offset + token_len <= len) {
        if(strncmp(start, token, token_len) != 0) {
            return IW_PARSE_NO_MATCH;
        }

        *offset += token_len;
        return IW_PARSE_MATCH;
    }

    return IW_PARSE_NO_MATCH;
}

// --------------------------------------------------------------------------

IW_PARSE iw_parse_read_to_token(
    const char *buff,
    unsigned int len,
    unsigned int *offset,
    const char *token,
    bool trim,
    iw_parse_index *index)
{
    const char *start = buff + *offset;
    unsigned int token_len = strlen(token);

    // We need to take care to not skip over a separator we are searching for
    // when trimming whitespace (e.g. if the separator is a space). We do this
    // by first finding the separator and then trim, but only trim while the
    // pointer has not reached the separator.
    unsigned int tmp = *offset;
    while(tmp + token_len <= len) {
        if(strncmp(buff + tmp, token, token_len) == 0) {
            break;
        }
        tmp++;
    }
    if(tmp + token_len > len) {
        return IW_PARSE_NO_MATCH;
    }

    const char *end = buff + tmp;
    *offset = tmp + token_len;
    if(trim) {
        // If we should trim, start with trimming the leading spaces
        if(isblank(*start) && start < end) {
            start++;
        }

        // Then trim the end
        if(end > start && isblank(*(end-1))) {
            end--;
        }
    }

    // Set the index values.
    index->start = start - buff;
    index->len   = end - start;

    return IW_PARSE_MATCH;
}

// --------------------------------------------------------------------------

bool iw_parse_cmp(
    const char *compare,
    const char *buffer,
    iw_parse_index *index)
{
    int len = strlen(compare);
    return len == index->len &&
           strncmp(compare, buffer + index->start, len) == 0;
}

// --------------------------------------------------------------------------

bool iw_parse_casecmp(
    const char *compare,
    const char *buffer,
    iw_parse_index *index)
{
    int len = strlen(compare);
    return len == index->len &&
           strncasecmp(compare, buffer + index->start, len) == 0;
}


// --------------------------------------------------------------------------
