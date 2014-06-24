// --------------------------------------------------------------------------
///
/// @file iw_util.h
///
/// Miscellaneous utility functionality.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_UTIL_H_
#define _IW_UTIL_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Defines
//
// --------------------------------------------------------------------------

/// Helper macro to stringify tokens
#define _STR(x) #x

/// @brief Macro to create strings out of tokens.
/// For example, if you have a define that defines a number, you can use
/// IW_STR(x) to create a string using that number.
/// \#define NUM 5
/// strlen(IW_STR(NUM));
/// @param x The token to stringify
#define IW_STR(x) _STR(x)

// --------------------------------------------------------------------------

/// @brief Return the length of the given array.
/// @param x The array to return the length of.
/// @return The length of the array.
#define IW_ARR_LEN(x)   (sizeof(x)/sizeof(x[0]))

// --------------------------------------------------------------------------

/// @brief Converts a string to a number.
/// Converts a string to a number and returns true if the conversion was
/// successful.
/// @param str The string to convert.
/// @param num The resulting number.
/// @param base The base to use when converting.
/// @return True if the conversion was successful.
extern bool iw_strtol(const char *str, long int *num, unsigned int base);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_UTIL_H_

// --------------------------------------------------------------------------
