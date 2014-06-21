// --------------------------------------------------------------------------
///
/// @file iw_mutex.h
///
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#ifndef _IW_MUTEX_H_
#define _IW_MUTEX_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Typedefs
//
// --------------------------------------------------------------------------

/// The mutex typedef. Used to refer to a mutex instance.
typedef unsigned int IW_MUTEX;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Create a mutex.
/// @param name The name of the mutex.
/// @return The mutex ID of the created mutex, or zero on failure.
IW_MUTEX iw_mutex_create(const char *name);

// --------------------------------------------------------------------------

/// @brief Lock the given mutex.
/// @param mutex The mutex to lock.
/// @return True if the mutex was successfully locked.
bool iw_mutex_lock(IW_MUTEX mutex);

// --------------------------------------------------------------------------

/// @brief Unlock the given mutex.
/// @param mutex The mutex to unlock.
void iw_mutex_unlock(IW_MUTEX mutex);

// --------------------------------------------------------------------------

/// @brief Destroy the given mutex and give up all resources allocated by it.
/// @param mutex The mutex to destroy.
void iw_mutex_destroy(IW_MUTEX mutex);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_MUTEX_H_

// --------------------------------------------------------------------------
