// --------------------------------------------------------------------------
///
/// @file iw_cfg.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_cfg.h"

#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------
//
// Data structures
//
// --------------------------------------------------------------------------

iw_cfg_store iw_cfg = {
    IW_DEF_COMMAND_PORT,
    {
        IW_DEF_OPT_FOREGROUND,
        IW_DEF_OPT_DAEMONIZE,
        IW_DEF_OPT_LOG_LEVEL
    },
    false, // Foreground?
    false, // Daemonize?
    false, // Allow quit?
    IW_DEF_ENABLE_CRASH_HANDLER,
    IW_DEF_ENABLE_MEMTRACK,
    IW_DEF_MEMTRACK_SIZE,
    IW_DEF_LOG_LEVEL,
    IW_DEF_SYSLOG_SIZE,
    IW_DEF_ENABLE_HEALTHCHECK,
    IW_DEF_ENABLE_WEB_SERVER,

    // The following parameters should only be accessed.
    "instaworks"
};

// --------------------------------------------------------------------------

iw_callbacks iw_cb = {
        NULL
};

// --------------------------------------------------------------------------

char *s_callstack_file = NULL;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_cfg_set_callstack_file(const char *file) {
    free(s_callstack_file);
    s_callstack_file = strdup(file);
}

// --------------------------------------------------------------------------
