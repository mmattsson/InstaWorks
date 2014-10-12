// --------------------------------------------------------------------------
///
/// @file iw_health.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_health_int.h"

#include "iw_log.h"
#include "iw_settings.h"
#include "iw_thread_int.h"
#include "iw_thread.h"

#include <stdbool.h>
#include <unistd.h>

// --------------------------------------------------------------------------
//
// Health thread callback
//
// --------------------------------------------------------------------------

static void *iw_health_thread(void *param) {
    while(true) {
        if(iw_thread_deadlock_check(false)) {
            LOG(IW_LOG_IW, "Deadlock detected!");
            iw_thread_deadlock_check(true);

            return NULL;
        }
        sleep(1);
    }
    return NULL;
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_health_start() {
    if(iw_stg.iw_enable_healthcheck) {
        if(!iw_thread_create("Health Check", iw_health_thread, NULL)) {
            LOG(IW_LOG_IW, "Failed to create health check thread");
        }
    }
}

// --------------------------------------------------------------------------
