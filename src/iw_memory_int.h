// --------------------------------------------------------------------------
///
/// @file iw_memory_int.h
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_MEMORY_INT_H_
#define _IW_MEMORY_INT_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_memory.h"

// --------------------------------------------------------------------------
//
// Internal helper macros
//
// --------------------------------------------------------------------------

/// Internal calloc macro, used by utility modules that needs to be able to
/// work in two modes, one mode where memory is tracked when used by external
/// callers, and one mode where memory is not tracked when used internally.
#define INT_CALLOC(x,ptr,num,tp)    if(x) { \
                                        ptr = (tp *)IW_CALLOC(num, sizeof(tp)); \
                                    } else { \
                                        ptr = (tp *)calloc(num, sizeof(tp)); \
                                    }

// --------------------------------------------------------------------------

/// Internal free macro, used by utility modules that needs to be able to
/// work in two modes, one mode where memory is tracked when used by external
/// callers, and one mode where memory is not tracked when used internally.
#define INT_FREE(x,ptr)             if(x) { \
                                        IW_FREE(ptr); \
                                    } else { \
                                        free(ptr); \
                                    }

// --------------------------------------------------------------------------
//
// Function API
//
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
