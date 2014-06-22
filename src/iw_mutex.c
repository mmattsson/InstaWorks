// --------------------------------------------------------------------------
///
/// @file iw_mutex.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_mutex.h"
#include "iw_mutex_int.h"

#include "iw_htable.h"
#include "iw_log.h"
#include "iw_thread_int.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------
//
// Variables and data structures.
//
// --------------------------------------------------------------------------

/// The mutex ID counter.
static int s_mutex_id = 1;

/// The global mutex hash.
iw_htable s_mutexes;

/// The internal rwlock for access to the mutex hash.
static pthread_rwlock_t s_mtx_lock;

// --------------------------------------------------------------------------

/// @brief Mutex info deletion function
/// @param info The mutex info structure to delete.
static void iw_mutex_info_delete(void *info) {
    iw_mutex_info *ptr = (iw_mutex_info *)info;
    free(ptr->name);
    free(ptr);
}

// --------------------------------------------------------------------------

/// @brief Find the mutex with the given ID.
/// @param mutex The mutex ID to find.
/// @return The mutex info structure or NULL if no match was found.
static iw_mutex_info *iw_mutex_find(IW_MUTEX id) {
    return (iw_mutex_info *)iw_htable_get(&s_mutexes, sizeof(id), &id);
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_mutex_init() {
    iw_htable_init(&s_mutexes, 128, NULL);
    pthread_rwlock_init(&s_mtx_lock, NULL);
}

// --------------------------------------------------------------------------

void iw_mutex_exit() {
    iw_htable_destroy(&s_mutexes, iw_mutex_info_delete);
}

// --------------------------------------------------------------------------

IW_MUTEX iw_mutex_create(const char *name) {
    iw_mutex_info *minfo = (iw_mutex_info *)calloc(1, sizeof(iw_mutex_info));
    minfo->name = strdup(name);
    minfo->id = s_mutex_id++;

    // Initialize the mutex
    if(pthread_mutex_init(&minfo->mutex, NULL) != 0) {
        // Failed to create the mutex.
        iw_mutex_info_delete(minfo);
        return 0;
    }

    // Insert the object into the table.
    pthread_rwlock_wrlock(&s_mtx_lock);
    bool retval = iw_htable_insert(&s_mutexes,
                                   sizeof(minfo->id), &(minfo->id),
                                   (iw_list_node *)minfo);
    pthread_rwlock_unlock(&s_mtx_lock);

    if(!retval) {
        // The insertion failed, let's delete the mutex
        pthread_mutex_destroy(&minfo->mutex);
        iw_mutex_info_delete(minfo);
        return 0;
    }

    return minfo->id;
}

// --------------------------------------------------------------------------

bool iw_mutex_lock(IW_MUTEX mutex) {
    // Start with doing a readlock on the mutex lock so that we can find
    // the mutex we are trying to lock.
    pthread_rwlock_rdlock(&s_mtx_lock);
    iw_mutex_info *minfo = iw_mutex_find(mutex);
    if(minfo == NULL) {
        // Failed to find the mutex, return
        pthread_rwlock_unlock(&s_mtx_lock);
        return false;
    }

    // Get the thread local storage
    pthread_mutex_t *tmp_mtx = &minfo->mutex;
    iw_thread_info *tinfo = pthread_getspecific(s_thread_key);
    tinfo->mutex = mutex;

    int retval = pthread_mutex_trylock(tmp_mtx);
    if(retval != 0) {
        // Failed to immediately take the mutex. Now we must release the
        // mutex lock so we don't block other threads trying to lock or
        // unlock other mutexes.
        pthread_rwlock_unlock(&s_mtx_lock);

        // Try to lock the mutex we want
        pthread_mutex_lock(tmp_mtx);

        // Got the mutex, now we need to reaquire the mutex lock and find
        // the mutex info again (since it may have been removed since we
        // last had it).
        pthread_rwlock_rdlock(&s_mtx_lock);
        iw_mutex_info *minfo = iw_mutex_find(mutex);
        if(minfo == NULL) {
            pthread_rwlock_unlock(&s_mtx_lock);
            return false;
        }
    }

    minfo->thread = tinfo->thread;
    pthread_rwlock_unlock(&s_mtx_lock);

    tinfo->mutex = 0;

    return true;
}

// --------------------------------------------------------------------------

void iw_mutex_unlock(IW_MUTEX mutex) {
    // Lock the mutex lock
    pthread_rwlock_rdlock(&s_mtx_lock);

    // Find the mutex we want to unlock
    iw_mutex_info *minfo = iw_mutex_find(mutex);
    minfo->thread = 0;
    pthread_mutex_unlock(&minfo->mutex);

    // Unlock the mutex lock
    pthread_rwlock_unlock(&s_mtx_lock);
}

// --------------------------------------------------------------------------

void iw_mutex_destroy(IW_MUTEX mutex) {
    // Lock the mutex lock
    pthread_rwlock_rdlock(&s_mtx_lock);

    iw_mutex_info *minfo = iw_htable_remove(&s_mutexes, sizeof(mutex), &mutex);

    // Unlock the mutex lock
    pthread_rwlock_unlock(&s_mtx_lock);

    if(minfo != NULL) {
        iw_mutex_info_delete(minfo);
    }
}

// --------------------------------------------------------------------------

void iw_mutex_dump(FILE *out) {
    // Lock the mutex lock
    pthread_rwlock_rdlock(&s_mtx_lock);

    unsigned long hash;
    iw_mutex_info *minfo = (iw_mutex_info *)iw_htable_get_first(&s_mutexes,
                                                                &hash);
    fprintf(out, " v-- Mutexes --v\n");
    while(minfo != NULL) {
        fprintf(out, "Mutex[%04X]: \"%s\", owned by thread=%08X\n",
            minfo->id, minfo->name, (unsigned int)minfo->thread);
        minfo = (iw_mutex_info *)iw_htable_get_next(&s_mutexes, &hash);
    }
    fprintf(out, " ^-- Mutexes --^\n");

    // Unlock the mutex lock
    pthread_rwlock_unlock(&s_mtx_lock);
}

// --------------------------------------------------------------------------
