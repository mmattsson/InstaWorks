// --------------------------------------------------------------------------
///
/// @file iw_memory.h
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_MEMORY_H_
#define _IW_MEMORY_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_log.h"

#include <stdio.h>
#include <stdlib.h>

// --------------------------------------------------------------------------
//
// API Wrappers
//
// --------------------------------------------------------------------------

/// A wrapper for the malloc() call.
#define IW_MALLOC(size)  \
    ( LOG_EX(IW_LOG_MEM, "++ malloc(%d)", size), IW_MEM_MALLOC(size) )

/// A wrapper for the calloc() call.
#define IW_CALLOC(elems,size) \
    ( LOG_EX(IW_LOG_MEM, "++ calloc(%d,%d)",elems,size), IW_MEM_CALLOC(elems,size) )

/// A wrapper for the realloc() call.
#define IW_REALLOC(ptr,size) \
    ( LOG_EX(IW_LOG_MEM, "++ realloc(%p,%d)",ptr,size), IW_MEM_REALLOC(ptr,size) )

/// A wrapper for the strdup() call.
#define IW_STRDUP(ptr) \
    ( LOG_EX(IW_LOG_MEM, "++ strdup(%p)",ptr), IW_MEM_STRDUP(ptr) )

/// A wrapper for the free() call.
#define IW_FREE(ptr) \
    ( LOG_EX(IW_LOG_MEM, "-- free(%p)",ptr), IW_MEM_FREE(ptr) )

#ifdef IW_NO_MEMORY_TRACKING
/// Map malloc calls to malloc() if memory tracking is disabled
#define IW_MEM_MALLOC(size)         malloc(size)
/// Map calloc calls to calloc() if memory tracking is disabled
#define IW_MEM_CALLOC(elems,size)   calloc(elems,size)
/// Map realloc calls to realloc() if memory tracking is disabled
#define IW_MEM_REALLOC(ptr,size)    realloc(ptr,size)
/// Map strdup calls to strdup() if memory tracking is disabled
#define IW_MEM_STRDUP(ptr)          strdup(ptr)
/// Map free calls to free() if memory tracking is disabled
#define IW_MEM_FREE(ptr)            free(ptr)
#else
/// Map malloc calls to iw_malloc() if memory tracking is enabled
#define IW_MEM_MALLOC(size)         iw_malloc(__FILE__,__LINE__,size)
/// Map calloc calls to iw_calloc() if memory tracking is enabled
#define IW_MEM_CALLOC(elems,size)   iw_calloc(__FILE__,__LINE__,elems,size)
/// Map realloc calls to iw_realloc() if memory tracking is enabled
#define IW_MEM_REALLOC(ptr,size)    iw_realloc(__FILE__,__LINE__,ptr,size)
/// Map strdup calls to iw_strdup() if memory tracking is enabled
#define IW_MEM_STRDUP(ptr)          iw_strdup(__FILE__,__LINE__,ptr)
/// Map free calls to iw_free() if memory tracking is enabled
#define IW_MEM_FREE(ptr)            iw_free(ptr)
#endif

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Allocates memory using the InstaWorks memory tracking module.
/// @param file The file that the memory block was allocated at.
/// @param line The line that the memory block was allocated at.
/// @param size The size of the memory block to allocate.
/// @return A pointer to the memory block that is allocated or NULL at failure.
extern void *iw_malloc(
    const char *file,
    unsigned int line,
    size_t size);

// --------------------------------------------------------------------------

/// @brief Allocates a memory block and initializes it to zero.
/// @param file The file that the memory block was allocated at.
/// @param line The line that the memory block was allocated at.
/// @param elems The number of elements to allocate.
/// @param size The size of each element.
/// @return A pointer to the memory block that is allocated or NULL at failure.
extern void *iw_calloc(
    const char *file,
    unsigned int line,
    int elems,
    size_t size);

// --------------------------------------------------------------------------

/// @brief Reallocates a memory block to the new given size.
/// @param file The file that the memory block was allocated at.
/// @param line The line that the memory block was allocated at.
/// @param ptr A pointer to the old memory block.
/// @param size The size of the new memory block.
/// @return A pointer to the memory block that is allocated or NULL at failure.
extern void *iw_realloc(
    const char *file,
    unsigned int line,
    void *ptr,
    size_t size);

// --------------------------------------------------------------------------

/// @brief Duplicates a string.
/// @param file The file that the memory block was allocated at.
/// @param line The line that the memory block was allocated at.
/// @param ptr A pointer to the string to be duplicated.
/// @return A pointer to the memory block that is allocated or NULL at failure.
extern char *iw_strdup(
    const char *file,
    unsigned int line,
    const char *ptr);

// --------------------------------------------------------------------------

/// @brief Frees an allocated memory block.
/// @param ptr A pointer to the memory to free.
extern void iw_free(
    void *ptr);

// --------------------------------------------------------------------------

/// @brief Utility function to display memory amount in a readable format.
/// For example, 10240 bytes would be displayed as 10 MBytes, 10 bytes would
/// be displayed as 10 Bytes.
/// @param len The length of the buffer to write to.
/// @param buff The buffer to write to.
/// @param bytes The number of bytes to display.
/// @return The provided buffer with the memory amount written into it.
extern char *iw_memory_display_str(
    unsigned int len,
    char *buff,
    unsigned int bytes);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_MEMORY_H_

// --------------------------------------------------------------------------
