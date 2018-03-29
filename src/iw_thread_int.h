// --------------------------------------------------------------------------
///
/// @file iw_thread_int.h
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_THREAD_INT_H_
#define _IW_THREAD_INT_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_list.h"
#include "iw_mutex.h"
#include "iw_thread.h"

#include <pthread.h>
#include <stdio.h>

// --------------------------------------------------------------------------
//
// Thread information structure
//
// --------------------------------------------------------------------------

/// The thread local storage.
extern pthread_key_t s_thread_key;

/// @brief The thread info structure.
typedef struct _iw_thread_info {
    iw_list_node node;      ///< The list node.
    char        *name;      ///< The name of the thread.
    pthread_t    thread;    ///< The thread handle.
    IW_MUTEX     mutex;     ///< The mutex this thread is waiting for (if any).
    IW_THREAD_CALLBACK fn;  ///< The callback function to call.
    bool         log;       ///< True if logging should be done for this thread.
    void *       param;     ///< The parameter to pass to the callback
} iw_thread_info;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Initialize the thread module.
extern void iw_thread_init();

// --------------------------------------------------------------------------

/// @brief Register the main thread for record keeping.
/// @return True if the main thread was successfully registered.
extern bool iw_thread_register_main();

// --------------------------------------------------------------------------

/// @brief Dump all thread information on the given file stream.
/// @param out The file stream to write the response to.
extern void iw_thread_dump(FILE *out);

// --------------------------------------------------------------------------

/// @brief Dump the callstack of a specific thread.
/// The callstack will be dumped on the debug logs.
/// @param out The file stream to write any errors to. Note that the result
///        is printed on the debug logs.
/// @param threadid The thread to dump the callstack for.
extern void iw_thread_callstack(FILE *out, pthread_t threadid);

// --------------------------------------------------------------------------

/// @brief Check for a deadlock.
/// @param log True if the deadlock information should be printed.
/// @return True if a deadlock is detected.
extern bool iw_thread_deadlock_check(bool log);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_THREAD_INT_H_

// --------------------------------------------------------------------------
