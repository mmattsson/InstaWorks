// --------------------------------------------------------------------------
///
/// @file iw_thread.h
///
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#ifndef _IW_THREAD_H_
#define _IW_THREAD_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Typedefs
//
// --------------------------------------------------------------------------

/// @brief The thread callback function definition.
typedef void *(*IW_THREAD_CALLBACK)(void *param);

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Create a new thread.
/// @param name The name of the thread.
/// @param func The thread callback function.
/// @param param An opaque parameter to pass to the callback function.
/// @return True if the thread was successfully created.
extern bool iw_thread_create(
    const char *name,
    IW_THREAD_CALLBACK func,
    void *param);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_THREAD_H_

// --------------------------------------------------------------------------
