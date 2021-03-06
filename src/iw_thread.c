// --------------------------------------------------------------------------
///
/// @file iw_thread.c
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_thread.h"
#include "iw_thread_int.h"

#include "iw_cfg.h"
#include "iw_common.h"
#include "iw_log.h"
#include "iw_main.h"
#include "iw_memory.h"
#include "iw_mutex_int.h"

#include <execinfo.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// --------------------------------------------------------------------------
//
// Defines
//
// --------------------------------------------------------------------------

/// The maximum number of function calls to include in the backtrace.
#define MAX_STACK   100

/// Helper macro to write a pointer to a string using write()
#define WRITE_PTR(fd,x)  \
    { \
        int cnt; \
        for(cnt=0;(x)[cnt] != '\0';cnt++) \
            ; \
        write(fd, x, cnt); \
    }

/// Helper macro to write a string to a file descriptor using write()
#define WRITE_STR(fd,x)     write(fd, x, sizeof(x))

/// Helper macro to write a number to a file descriptor using write()
#define WRITE_NUM(fd,num)  \
    { \
        char buff[16] = ""; \
        int tmp, idx; \
        for(idx=14,tmp=num;tmp != 0 && idx >= 0;tmp/=10,idx--) { \
            buff[idx]='0'+(tmp%10); \
        } \
        WRITE_PTR(fd, buff + idx + 1); \
    }

/// Helper macro to write a hex number to a file descriptor using write()
#define WRITE_HEX(fd,num) \
    { \
        char buff[16] = ""; \
        long tmp, idx; \
        for(idx=14,tmp=(long)num;tmp != 0 && idx >= 0;tmp>>=4,idx--) { \
            buff[idx] = s_digits[tmp&0xF]; \
        } \
        WRITE_STR(fd, "0x"); \
        WRITE_PTR(fd, buff + idx + 1); \
    }

// --------------------------------------------------------------------------
//
// Variables and data structures.
//
// --------------------------------------------------------------------------

/// The global thread list.
static iw_htable s_threads;

/// The main thread info.
static iw_thread_info *s_main_tinfo = NULL;

/// The thread local storage for the threads.
pthread_key_t s_thread_key;

/// The internal rwlock for access to the mutex hash.
static pthread_rwlock_t s_thread_lock;

/// Counter to track number of SIGINTs received.
static int s_sigint_cnt = 0;

/// Helper for WRITE_HEX() to convert to character string.
static char s_digits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

// --------------------------------------------------------------------------
//
// Internal helper functions
//
// --------------------------------------------------------------------------

/// @brief Thread info deletion function.
/// @param node The thread info structure to delete.
static void iw_thread_info_delete(void *node) {
    iw_thread_info *tinfo = (iw_thread_info *)node;
    free(tinfo->name);
    free(tinfo);
}

// --------------------------------------------------------------------------

/// @brief Thread info creation function.
/// @param name The name of the thread.
/// @param thread The thread ID or zero if not known.
/// @param fn The callback function, or NULL if not applicable.
/// @param param The callback parameter, or NULL if not applicable.
/// @return The created thread info structure or NULL for failure.
static iw_thread_info *iw_thread_info_create(
    const char *name,
    pthread_t   thread,
    IW_THREAD_CALLBACK fn,
    void *param)
{
    iw_thread_info *tinfo = (iw_thread_info *)calloc(1,
                                                     sizeof(iw_thread_info));

    if(tinfo != NULL) {
        tinfo->name   = strdup(name);
        tinfo->log    = true;
        tinfo->thread = thread;
        tinfo->fn     = fn;
        tinfo->param  = param;
    }

    return tinfo;
}

// --------------------------------------------------------------------------

/// @brief The thread signal handler.
/// @param sig The signal being sent to the thread.
static void iw_thread_signal(int sig, siginfo_t *si, void *param) {
    UNUSED(param);

    switch(sig) {
    case SIGUSR1 : {
        iw_thread_info *tinfo = pthread_getspecific(s_thread_key);
        int cnt;
        void *buffer[MAX_STACK];
        int nptrs = backtrace(buffer, MAX_STACK);
        char **strings = backtrace_symbols(buffer, nptrs);
        LOG(IW_LOG_IW, " v-- Thread [%08lX] backtrace --v", pthread_self());
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
        LOG(IW_LOG_IW, " ^-- Thread [%08lX] backtrace --^", pthread_self());
        } break;
    case SIGINT  : {
        // Handle shutdown by Ctrl-C. By handling Ctrl-C properly we can
        // clean up known memory to make memory leaks more noticable.
        LOG(IW_LOG_IW, "Received SIGINT, shutting down");
        s_sigint_cnt++;
        if(s_sigint_cnt == 1) {
            // This is the first SIGINT, attempt to shut down orderly.
        iw_main_loop_terminate();
        } else {
            // This is not the first SIGINT we receive, it is possible
            // that the shutdown process deadlocked. Go ahead and
            // forcibly exit the program.
            LOG(IW_LOG_IW, "Received multiple SIGINT, exiting forcefully");
            exit(-1);
        }
        } break;
    case SIGILL  :
    case SIGABRT :
    case SIGFPE  :
    case SIGBUS  :
    case SIGSEGV : {
            // First try to get backtrace and symbols without calling
            // other non-safe functions. These calls aren't safe either
            // but without them we have nothing.
            int fd = -1;
            char *file = iw_val_store_get_string(&iw_cfg,
                                                 IW_CFG_CRASHHANDLER_FILE);
            char *prg = iw_val_store_get_string(&iw_cfg,
                                                IW_CFG_PRG_NAME);
            if(file != NULL) {
                fd = open(file, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR);
            }
            if(fd != -1) {
                WRITE_STR(fd, "Program: ");
                WRITE_PTR(fd, prg ? prg : "");
                WRITE_STR(fd, "\r\nCaught signal: ");
                WRITE_NUM(fd, sig)
                WRITE_STR(fd, " (");
                switch(sig) {
                case SIGILL  : WRITE_STR(fd, "SIGILL");  break;
                case SIGABRT : WRITE_STR(fd, "SIGABRT"); break;
                case SIGFPE  : WRITE_STR(fd, "SIGFPE");  break;
                case SIGBUS  : WRITE_STR(fd, "SIGBUS");  break;
                case SIGSEGV : WRITE_STR(fd, "SIGSEGV"); break;
                default : WRITE_STR(fd, "?"); break;
                }
                WRITE_STR(fd, ")\r\nAddress: ");
                void *ptr = si->si_addr;
                WRITE_HEX(fd, ptr);
                WRITE_STR(fd, "\r\nCallstack:\r\n-------------------\r\n");

                void *buffer[MAX_STACK];
                int nptrs = backtrace(buffer, MAX_STACK);
                backtrace_symbols_fd(buffer, nptrs, fd);
                WRITE_STR(fd, "\r\n");
                close(fd);
            }

            // Finally, exit.
            _exit(-1);
       } break;
    }
}

// --------------------------------------------------------------------------

/// @brief Install signal handlers for the thread.
static void iw_thread_install_sighandler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = iw_thread_signal;
    sa.sa_flags     = SA_RESTART | SA_SIGINFO;
    sigfillset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    // Install SIGINT to handle shutdown through Ctrl-C
    sigaction(SIGINT, &sa, NULL);

    int *enable = iw_val_store_get_number(&iw_cfg, IW_CFG_CRASHHANDLER_ENABLE);
    if(enable != NULL && *enable) {
        sigaction(SIGILL, &sa, NULL);
        sigaction(SIGABRT, &sa, NULL);
        sigaction(SIGFPE, &sa, NULL);
        sigaction(SIGBUS, &sa, NULL);
        sigaction(SIGSEGV, &sa, NULL);
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
    iw_thread_install_sighandler();

    // Call back into the thread callback function
    LOG(IW_LOG_IW, "Calling thread callback function for thread \"%s\"",
                    tinfo->name);
    tinfo->fn(tinfo->param);
    LOG(IW_LOG_IW, "Thread callback function for thread \"%s\" returned",
                    tinfo->name);

    // Don't delete the thread info structure here since we won't be able
    // to join the thread if we do so.

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
    iw_htable_init(&s_threads, 128, false, NULL);
}

// --------------------------------------------------------------------------

void iw_thread_exit() {
    // When exiting, make sure to remove and delete the main thread info
    LOG(IW_LOG_IW, "Terminating thread module");
    pthread_rwlock_wrlock(&s_thread_lock);
    if(s_main_tinfo != NULL) {
        iw_htable_delete(&s_threads,
                     sizeof(s_main_tinfo->thread), &(s_main_tinfo->thread),
                     iw_thread_info_delete);
        s_main_tinfo = NULL;
    }
    iw_htable_destroy(&s_threads, iw_thread_info_delete);
    pthread_rwlock_unlock(&s_thread_lock);
}

// --------------------------------------------------------------------------

bool iw_thread_register_main() {
    // Install signal handler for the thread
    iw_thread_install_sighandler();

    s_main_tinfo = iw_thread_info_create("Main", pthread_self(),
                                                  NULL, NULL);
    if(s_main_tinfo == NULL) {
        return false;
    }

    if(pthread_key_create(&s_thread_key, NULL) != 0 ||
       pthread_setspecific(s_thread_key, s_main_tinfo) != 0)
    {
        // Can't use thread local storage, just note this in the logs. No
        // need t return since we can function with reduced features.
        LOG(IW_LOG_IW, "Failed to create thread local storage");
    }

    // No other thread is created yet, no need to lock thread lock
    return iw_htable_insert(&s_threads,
                     sizeof(s_main_tinfo->thread),
                     &(s_main_tinfo->thread), 
                     s_main_tinfo);
}

// --------------------------------------------------------------------------

bool iw_thread_get_log(pthread_t threadid) {
    bool retval = false;
    iw_thread_info *tinfo = NULL;
    pthread_rwlock_rdlock(&s_thread_lock);
    if(threadid == 0) {
        tinfo = (iw_thread_info *)pthread_getspecific(s_thread_key);
    } else {
        tinfo = (iw_thread_info *)iw_htable_get(&s_threads,
                                            sizeof(threadid),
                                            &threadid);
    }

    retval = tinfo != NULL && tinfo->log;
    pthread_rwlock_unlock(&s_thread_lock);

    return retval;
}

// --------------------------------------------------------------------------

void iw_thread_set_log_all(bool log_on) {
    unsigned long hash;
    pthread_rwlock_rdlock(&s_thread_lock);
    iw_thread_info *tinfo = (iw_thread_info *)iw_htable_get_first(&s_threads,
                                                                   &hash);
    while(tinfo != NULL) {
        tinfo->log = log_on;
        tinfo = (iw_thread_info *)iw_htable_get_next(&s_threads, &hash);
    }
    pthread_rwlock_unlock(&s_thread_lock);
}

// --------------------------------------------------------------------------

bool iw_thread_set_log(pthread_t threadid, bool log_on) {
    bool retval = false;
    iw_thread_info *tinfo = NULL;
    pthread_rwlock_rdlock(&s_thread_lock);
    if(threadid == 0) {
        tinfo = (iw_thread_info *)pthread_getspecific(s_thread_key);
    } else {
        tinfo = (iw_thread_info *)iw_htable_get(&s_threads,
                                            sizeof(threadid),
                                            &threadid);
    }
    if(tinfo != NULL) {
        tinfo->log = log_on;
        retval = true;
    }
    pthread_rwlock_unlock(&s_thread_lock);
    return retval;
}

// --------------------------------------------------------------------------

bool iw_thread_create_int(
    pthread_t *tid,
    const char *name, 
    IW_THREAD_CALLBACK func, 
    bool client,
    void *param)
{
    if(tid != NULL) {
        *tid = 0;
    }
    iw_thread_info *tinfo = iw_thread_info_create(name, 0, func, param);
    if(tinfo == NULL) {
        return false;
    }

    tinfo->client = client;
    if(pthread_create(&tinfo->thread, NULL, iw_thread_callback, tinfo) == 0) {
        if(tid != NULL) {
            *tid = tinfo->thread;
        }
        return true;
    } else {
        // Failed to create the thread.
        iw_thread_info_delete((void *)tinfo);
        return false;
    }
}

// --------------------------------------------------------------------------

bool iw_thread_create(
    pthread_t *tid,
    const char *name,
    IW_THREAD_CALLBACK func,
    void *param)
{
    return iw_thread_create_int(tid, name, func, true, param);
}

// --------------------------------------------------------------------------

void iw_thread_wait_all() {
    bool foundClientThread = true;
    unsigned long hash;

    LOG(IW_LOG_IW, "iw_thread_wait_all");
    while(foundClientThread) {
        pthread_rwlock_rdlock(&s_thread_lock);
        iw_thread_info *tinfo = (iw_thread_info *)iw_htable_get_first(&s_threads,
                                                                   &hash);
        while(tinfo != NULL && !tinfo->client) {
            tinfo = (iw_thread_info *)iw_htable_get_next(&s_threads, &hash);
        }
        if(tinfo != NULL && tinfo->client) {
            foundClientThread = true;
            LOG(IW_LOG_IW, "Joining thread \"%s\"", tinfo->name);
            pthread_t pid = tinfo->thread;
            pthread_rwlock_unlock(&s_thread_lock);
            pthread_join(tinfo->thread, NULL);

            // Thread has exited, remove it from the thread list
            iw_htable_delete(&s_threads,
                     sizeof(pid), &(pid),
                     iw_thread_info_delete);
        } else {
            foundClientThread = false;
            pthread_rwlock_unlock(&s_thread_lock);
        }
    }
    LOG(IW_LOG_IW, "iw_thread_wait_all done");
}

// --------------------------------------------------------------------------

void iw_thread_dump(FILE *out) {
    unsigned long hash;
    pthread_rwlock_rdlock(&s_thread_lock);
    iw_thread_info *thread = (iw_thread_info *)iw_htable_get_first(&s_threads,
                                                                   &hash);
    fprintf(out, "== Thread Information ==\n");
    fprintf(out, "Thread-ID  Log Mutex Clnt Thread-name\n");
    fprintf(out, "---------------------------------\n");
    while(thread != NULL) {
        fprintf(out, "[%08lX] %3s %04X %c: \"%s\"\n",
            (unsigned long int)thread->thread,
            thread->log ? "on " : "off",
            thread->mutex,
            thread->client ? 'Y' : 'N',
            thread->name);
        thread = (iw_thread_info *)iw_htable_get_next(&s_threads, &hash);
    }
    pthread_rwlock_unlock(&s_thread_lock);
}

// --------------------------------------------------------------------------

void iw_thread_callstack(FILE *out, pthread_t threadid) {
    pthread_rwlock_rdlock(&s_thread_lock);
    iw_thread_info *thread =
        (iw_thread_info *)iw_htable_get(&s_threads,
                                        sizeof(threadid),
                                        &threadid);
    pthread_rwlock_unlock(&s_thread_lock);
    if(thread == NULL) {
        fprintf(out, "Error: Thread %08lX does not exist\n",
                (unsigned long int)threadid);
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
                LOG(IW_LOG_IW, "Thread %08lX is waiting for mutex %d",
                    (unsigned long int)thread->thread, thread->mutex);
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
                LOG(IW_LOG_IW, "Mutex %d is owned by thread %08lX",
                    mutex->id, (unsigned long int)mutex->thread);
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
