// --------------------------------------------------------------------------
///
/// @file iw_mutex_int.h
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_MUTEX_INT_H_
#define _IW_MUTEX_INT_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_htable.h"
#include "iw_mutex.h"

#include <stdbool.h>
#include <stdio.h>

// --------------------------------------------------------------------------
//
// Variables
//
// --------------------------------------------------------------------------

extern iw_htable s_mutexes;

// --------------------------------------------------------------------------

/// @brief The mutex info structure.
typedef struct _iw_mutex_info {
    IW_MUTEX        id;         ///< The mutex id for external use.
    char           *name;       ///< The name of the mutex.
    pthread_mutex_t mutex;      ///< The mutex handle.
    pthread_t       thread;     ///< The thread owning this mutex (if any).
} iw_mutex_info;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Initializes the mutex framework.
extern void iw_mutex_init();

// --------------------------------------------------------------------------

/// @brief Terminates the mutex framework.
extern void iw_mutex_exit();

// --------------------------------------------------------------------------

/// @brief Dump all mutex information on the given file stream.
/// @param out The file stream to write the response to.
extern void iw_mutex_dump(FILE *out);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_MUTEX_INT_H_

// --------------------------------------------------------------------------
