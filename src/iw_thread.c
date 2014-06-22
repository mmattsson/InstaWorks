// --------------------------------------------------------------------------
///
/// @file iw_thread.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_thread.h"
#include "iw_thread_int.h"

#include "iw_log.h"
#include "iw_memory.h"
#include "iw_mutex_int.h"

#include <execinfo.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------
//
// Defines
//
// --------------------------------------------------------------------------

/// The maximum number of function calls to include in the backtrace.
#define MAX_STACK   100

// --------------------------------------------------------------------------
//
// Variables and data structures.
//
// --------------------------------------------------------------------------

/// The global thread list.
static iw_htable s_threads;

/// The thread local storage for the threads.
pthread_key_t s_thread_key;

/// The internal rwlock for access to the mutex hash.
static pthread_rwlock_t s_thread_lock;

// --------------------------------------------------------------------------

/// @brief Thread info deletion function.
/// @param node The thread info structure to delete.
static void iw_thread_info_delete(void *node) {
    iw_thread_info *tinfo = (iw_thread_info *)node;
    free(tinfo->name);
    free(tinfo);
}

// --------------------------------------------------------------------------

/// @brief The thread signal handler.
/// @param sig The signal being sent to the thread.
static void iw_thread_signal(int sig) {
    iw_thread_info *tinfo = pthread_getspecific(s_thread_key);
    if(sig == SIGUSR1) {
        int cnt;
        void *buffer[MAX_STACK];
        int nptrs = backtrace(buffer, MAX_STACK);
        char **strings = backtrace_symbols(buffer, nptrs);
        LOG(IW_LOG_IW, " v-- Thread [%08X] backtrace --v", pthread_self());
        LOG(IW_LOG_IW, "Name: %s", tinfo->name);
        if(tinfo->mutex != 0) {
            LOG(IW_LOG_IW, "Waiting for mutex: %d", tinfo->mutex);
        }
        if(strings != NULL) {
            for(cnt=0;cnt < nptrs;cnt++) {
                LOG(IW_LOG_IW, "%s", strings[cnt]);
            }
            free(strings);
        } else {
            for(cnt=0;cnt < nptrs;cnt++) {
                LOG(IW_LOG_IW, "[%p]", buffer[cnt]);
            }
        }
        LOG(IW_LOG_IW, " ^-- Thread [%08X] backtrace --^", pthread_self());
    }
}

// --------------------------------------------------------------------------

/// @brief The thread callback function.
/// @param param The parameter being passed to the thread.
/// @return The exit value of the thread.
static void *iw_thread_callback(void *param) {
    // Create the thread info object
    iw_thread_info *tinfo = (iw_thread_info *)param;

    // Point the thread local storage to the tinfo object
    pthread_setspecific(s_thread_key, tinfo);

    // Insert tinfo object into thread hash table
    pthread_rwlock_wrlock(&s_thread_lock);
    iw_htable_insert(&s_threads,
                     sizeof(tinfo->thread), &(tinfo->thread), tinfo);
    pthread_rwlock_unlock(&s_thread_lock);

    // Install signal handler for the thread
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = iw_thread_signal;
    sigaction(SIGUSR1, &sa, NULL);

    // Call back into the thread callback function
    tinfo->fn(tinfo->param);

    // Thread has exited, remove it from the thread list
    pthread_rwlock_wrlock(&s_thread_lock);
    iw_htable_delete(&s_threads,
                     sizeof(tinfo->thread), &(tinfo->thread),
                     iw_thread_info_delete);
    pthread_rwlock_unlock(&s_thread_lock);

    return NULL;
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_thread_init() {
    // Initialize the thread hash table
    pthread_rwlock_init(&s_thread_lock, NULL);
    iw_htable_init(&s_threads, 128, NULL);
}

// --------------------------------------------------------------------------

void iw_thread_register_main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = iw_thread_signal;
    sigaction(SIGUSR1, &sa, NULL);

    iw_thread_info *tinfo = (iw_thread_info *)calloc(1,
                                                     sizeof(iw_thread_info));
    tinfo->name   = strdup("Main");
    tinfo->thread = pthread_self();
    if(pthread_key_create(&s_thread_key, NULL) != 0 ||
       pthread_setspecific(s_thread_key, tinfo) != 0)
    {
        // Can't use thread local storage, just note this in the logs. No
        // need t return since we can function with reduced features.
        LOG(IW_LOG_IW, "Failed to create thread local storage");
    }

    // No other thread is created yet, no need to lock thread lock
    iw_htable_insert(&s_threads,
                     sizeof(tinfo->thread), &(tinfo->thread), tinfo);
}

// --------------------------------------------------------------------------

bool iw_thread_create(const char *name, IW_THREAD_CALLBACK func, void *param) {
    iw_thread_info *info = (iw_thread_info *)calloc(1, sizeof(iw_thread_info));
    info->name  = strdup(name);
    info->fn    = func;
    info->param = param;
    if(pthread_create(&info->thread, NULL, iw_thread_callback, info) == 0) {
        return true;
    } else {
        // Failed to create the thread.
        iw_thread_info_delete((void *)info);
        return false;
    }
}

// --------------------------------------------------------------------------

void iw_thread_dump(FILE *out) {
    unsigned long hash;
    pthread_rwlock_rdlock(&s_thread_lock);
    iw_thread_info *thread = (iw_thread_info *)iw_htable_get_first(&s_threads,
                                                                   &hash);
    fprintf(out, " v-- Threads --v\n");
    while(thread != NULL) {
        if(thread->mutex != 0) {
            fprintf(out, "Thread[%08X]: \"%s\", waiting for mutex=%04X\n",
                (unsigned int)thread->thread, thread->name, thread->mutex);
        } else {
            fprintf(out, "Thread[%08X]: \"%s\"\n",
                (unsigned int)thread->thread, thread->name);
        }
        thread = (iw_thread_info *)iw_htable_get_next(&s_threads, &hash);
    }
    fprintf(out, " ^-- Threads --^\n");
    pthread_rwlock_unlock(&s_thread_lock);
}

// --------------------------------------------------------------------------

void iw_thread_callstack(FILE *out, unsigned int threadid) {
    pthread_rwlock_rdlock(&s_thread_lock);
    iw_thread_info *thread =
        (iw_thread_info *)iw_htable_get(&s_threads,
                                        sizeof(threadid),
                                        &threadid);
    pthread_rwlock_unlock(&s_thread_lock);
    if(thread == NULL) {
        fprintf(out, "Error: Thread %08X does not exist\n", threadid);
        return;
    }

    pthread_kill(threadid, SIGUSR1);
    fprintf(out, "The thread callstack has been printed on the debug logs.\n");
}

// --------------------------------------------------------------------------

bool iw_thread_deadlock_check(bool log) {
    // Walk through the threads to find the first thread waiting for a mutex
    unsigned long hash;
    pthread_rwlock_rdlock(&s_thread_lock);
    iw_thread_info *curr = (iw_thread_info *)iw_htable_get_first(&s_threads,
                                                                 &hash);

    while(curr != NULL) {

        // Start the cycle check with thread curr, then update thread to point
        // to the next thread in the cycle as we search.
        iw_thread_info *thread = curr;
        while(thread != NULL && thread->mutex != 0) {
            // This thread is waiting for a mutex. Find out who owns this mutex.

            if(log) {
                LOG(IW_LOG_IW, "Thread %08X is waiting for mutex %d",
                    (unsigned int)thread->thread, thread->mutex);
            }

            // First get the mutex this thread is waiting for.
            iw_mutex_info *mutex =
                (iw_mutex_info *)iw_htable_get(&s_mutexes,
                                                sizeof(thread->mutex),
                                                &thread->mutex);
            if(mutex == NULL) {
                // This shouldn't happen since this thread was waiting for this
                // mutex but there may have been a race where this mutex was
                // just released.
                continue;
            }

            if(log) {
                LOG(IW_LOG_IW, "Mutex %d is owned by thread %08X",
                    mutex->id, (unsigned int)mutex->thread);
            }

            // Then get the thread owning this mutex.
            thread =
                (iw_thread_info *)iw_htable_get(&s_threads,
                                                sizeof(mutex->thread),
                                                &(mutex->thread));
            if(thread == NULL) {
                // This shouldn't happen since another thread was waiting to
                // own this mutex, but maybe there was a race where this mutex
                // just got released.
                continue;
            }

            // Print backtrace for this thread
            if(log) {
                pthread_kill(thread->thread, SIGUSR1);
            }

            // Check whether this thread is the same thread as we started out
            // with, i.e. if thread == curr. If so, we've found a cycle and
            // detected a deadlock
            if(thread == curr) {
                pthread_rwlock_unlock(&s_thread_lock);
                return true;
            }
        }

        // Check the next thread for a deadlock
        curr = (iw_thread_info *)iw_htable_get_next(&s_threads, &hash);
    }
    pthread_rwlock_unlock(&s_thread_lock);

    // No deadlock detected.
    return false;
}

// --------------------------------------------------------------------------
