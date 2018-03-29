// --------------------------------------------------------------------------
///
/// @file iw_health.c
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_health_int.h"

#include "iw_cfg.h"
#include "iw_log.h"
#include "iw_thread_int.h"
#include "iw_thread.h"

#include <stdbool.h>
#include <unistd.h>

// --------------------------------------------------------------------------
//
// Variables
//
// --------------------------------------------------------------------------

/// The health thread TID, used to terminate and join the thread when exiting.
static pthread_t s_health_tid = 0;

/// Status for whether the health thread should continue to execute.
static bool s_health_go = true;

// --------------------------------------------------------------------------
//
// Health thread callback
//
// --------------------------------------------------------------------------

static void *iw_health_thread(void *param) {
    while(s_health_go) {
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

void iw_health_init() {
    int *enable = iw_val_store_get_number(&iw_cfg, IW_CFG_HEALTHCHECK_ENABLE);
    if(enable && *enable) {
        if(!iw_thread_create(&s_health_tid, "Health Check", iw_health_thread, NULL)) {
            LOG(IW_LOG_IW, "Failed to create health check thread");
        }
    }
}

// --------------------------------------------------------------------------

void iw_health_exit() {
    if(s_health_tid != 0) {
        s_health_go = false;
        pthread_join(s_health_tid, NULL);
    }
}

// --------------------------------------------------------------------------

