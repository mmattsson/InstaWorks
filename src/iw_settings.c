// --------------------------------------------------------------------------
///
/// @file iw_settings.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_settings.h"

#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------
//
// Data structures
//
// --------------------------------------------------------------------------

iw_settings iw_stg = {
    IW_DEF_COMMAND_PORT,
    {
        IW_DEF_OPT_FOREGROUND,
        IW_DEF_OPT_DAEMONIZE,
        IW_DEF_OPT_LOG_LEVEL
    },
    false, // Foreground?
    false, // Daemonize?
    false, // Allow quit?
    IW_DEF_CRASH_HANDLER_ENABLE,
    IW_DEF_MEMTRACK_ENABLE,
    IW_DEF_MEMTRACK_SIZE,
    IW_DEF_LOG_LEVEL,
    IW_DEF_SYSLOG_SIZE,
    IW_DEF_HEALTHCHECK_ENABLE,

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

void iw_stg_set_callstack_file(const char *file) {
    free(s_callstack_file);
    s_callstack_file = strdup(file);
}

// --------------------------------------------------------------------------
