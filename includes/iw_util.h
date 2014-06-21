// --------------------------------------------------------------------------
///
/// @file iw_util.h
///
/// Miscellaneous utility functionality.
///
/// Copyright (C) Mattias Mattsson - 2014
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
