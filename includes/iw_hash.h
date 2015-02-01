// --------------------------------------------------------------------------
///
/// @file iw_hash.h
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_HASH_H_
#define _IW_HASH_H_
#ifdef _cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
//
// Typedefs
//
// --------------------------------------------------------------------------

/// The function definition for hash functions.
typedef unsigned long(*IW_HASH_FN)(unsigned int, const void *);

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief A hash function used to create keys from given data.
/// @param len The length of the data provided.
/// @param data A pointer to the data used.
/// @return The hash value for the given data.
extern unsigned long iw_hash_data(unsigned int len, const void *data);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_HASH_H_

// --------------------------------------------------------------------------
