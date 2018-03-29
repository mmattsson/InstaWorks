// --------------------------------------------------------------------------
///
/// @file iw_parse.h
///
/// Module for parsing text.
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_PARSE_H_
#define _IW_PARSE_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Typedefs
//
// --------------------------------------------------------------------------

#define IW_PARSE_CRLF       "\r\n" ///< CR + LF
#define IW_PARSE_SPACE      " " ///< Space
#define IW_PARSE_COLON      ":" ///< Colon
#define IW_PARSE_QUERY      "?" ///< Question mark
#define IW_PARSE_EQUAL      "=" ///< Equal sign
#define IW_PARSE_AMPERSAND  "&" ///< Ampersand

/// The parse return value
typedef enum _IW_PARSE {
    IW_PARSE_MATCH,
    IW_PARSE_NO_MATCH,
    IW_PARSE_ERROR
} IW_PARSE;

// --------------------------------------------------------------------------

/// An index into the buffer for a given value.
typedef struct _iw_parse_index {
    int start;  ///< The start of the value.
    int len;    ///< The length of the value (from the start).
} iw_parse_index;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Search for a token from the given starting point.
/// @param buff The buffer to process.
/// @param len The length of the buffer.
/// @param offset [in/out] The offset into the buffer to start. Will be updated
///        to the offset of the character after the separator.
/// @param token Token to search for.
/// @return A parse value to signify whether the operation succeeded or not.
extern IW_PARSE iw_parse_find_token(
    const char *buff,
    unsigned int len,
    unsigned int *offset,
    const char *token);

// --------------------------------------------------------------------------

/// @brief Checks whether the next characters is a given token.
/// Updates the \a offset if the token is found.
/// @param buff The buffer to process.
/// @param len The length of the buffer.
/// @param offset [in/out] The offset into the buffer to start. Will be updated
///        to the offset of the character after the separator.
/// @param sep The separator to search for.
/// @return A parse value to signify whether the operation succeeded or not.
extern IW_PARSE iw_parse_is_token(
    const char *buff,
    unsigned int len,
    unsigned int *offset,
    const char *sep);

// --------------------------------------------------------------------------

/// @brief Read data up to the next token.
/// Searches the buffer given by \a buff and \a offset and finds the next
/// occurance of the token \a token. If trim is set, the value between the
/// current start and the token is trimmed of whitespace. The start
/// of the data and the length of the data is returned in \a index.
/// @param buff The buffer to process.
/// @param len The length of the buffer.
/// @param offset [in/out] The offset into the buffer to start. Will be updated
///        to the offset of the character after the token.
/// @param token The token to search for.
/// @param trim True if whitespace should be trimmed.
/// @param index [out] The start and length of the value before the separator.
/// @return A parse value to signify whether the operation succeeded or not.
extern IW_PARSE iw_parse_read_to_token(
    const char *buff,
    unsigned int len,
    unsigned int *offset,
    const char *token,
    bool trim,
    iw_parse_index *index);

// --------------------------------------------------------------------------

/// @brief Compare the parse index with the given reference string.
/// @param compare The string to compare agains.
/// @param buffer The buffer to apply the parse index against.
/// @param index The parse index to compare.
/// @return True if the value matches the reference string.
extern bool iw_parse_cmp(
    const char *compare,
    const char *buffer,
    iw_parse_index *index);

// --------------------------------------------------------------------------

/// @brief Compare the parse index with the given reference string.
/// Uses a case insensitive comparison.
/// @param compare The string to compare agains.
/// @param buffer The buffer to apply the parse index against.
/// @param index The parse index to compare.
/// @return True if the value matches the reference string.
extern bool iw_parse_casecmp(
    const char *compare,
    const char *buffer,
    iw_parse_index *index);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_PARSE_H_

// --------------------------------------------------------------------------
