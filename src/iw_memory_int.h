// --------------------------------------------------------------------------
///
/// @file iw_memory_int.h
///
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#ifndef _IW_MEMORY_INT_H_
#define _IW_MEMORY_INT_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_memory.h"

// --------------------------------------------------------------------------

/// @brief Initializes the memory module.
extern void iw_memory_init();

// --------------------------------------------------------------------------

/// @brief Terminates the memory module.
extern void iw_memory_exit();

// --------------------------------------------------------------------------

/// @brief Show all memory information on the given file stream.
/// @param out The file stream to write the response to.
extern void iw_memory_show(FILE *out);

// --------------------------------------------------------------------------

/// @brief Show a summary of the memory allocations on the given file stream.
/// @param out The file stream to write the response to.
extern void iw_memory_summary(FILE *out);

// --------------------------------------------------------------------------

/// @brief Show a brief summary of the memory allocations on the given stream.
/// @param out The file stream to write the response to.
extern void iw_memory_brief(FILE *out);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_MEMORY_INT_H_

// --------------------------------------------------------------------------
