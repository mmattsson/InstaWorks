// --------------------------------------------------------------------------
///
/// @file iw_thread.h
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_THREAD_H_
#define _IW_THREAD_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <pthread.h>

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

/// @brief Set logging for all threads.
/// Set logging to on or off for all threads.
/// @param log_on True if logging should be enabled, false for disabled.
extern void iw_thread_set_log_all(bool log_on);

// --------------------------------------------------------------------------

/// @brief Check if logging should be done for the given thread.
/// @param threadid The thread to check logging for or 0 for the calling thread.
/// @return True if logging should be done.
extern bool iw_thread_get_log(pthread_t threadid);

// --------------------------------------------------------------------------

/// @brief Set logging for the given thread.
/// @param threadid The thread to set logging for or 0 for the calling thread.
/// @param log_on True if logging should be enabled, false for disabled.
/// @return True if the thread was found and the log level was set.
extern bool iw_thread_set_log(pthread_t threadid, bool log_on);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_THREAD_H_

// --------------------------------------------------------------------------
