// --------------------------------------------------------------------------
///
/// @file iw_log.c
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_log.h"

#include "iw_thread_int.h"
#include "iw_util.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------
//
// Variables
//
// --------------------------------------------------------------------------

static char *s_dev = NULL;
static FILE *s_fd = NULL;

/// @brief The log level structure
typedef struct _log_level {
    const char *desc;   ///< The description of the log level.
} log_level;

static log_level s_levels[32] = { { NULL } };

unsigned int s_log_level = 0;

// --------------------------------------------------------------------------
//
// Internal helpers
//
// --------------------------------------------------------------------------

static void iw_vlog(
    const char *file,
    unsigned int line,
    const char *msg,
    va_list argp)
{
    if(s_fd == NULL) {
        return;
    }

    if(!iw_thread_get_log(0)) {
        // If we can get the thread specific info and logging is disabled
        // in the thread info, then just return rather than print the
        // debug log.
        return;
    }

    fprintf(s_fd, "[%X]%s(%d): ", (unsigned int)pthread_self(), file, line);
    vfprintf(s_fd, msg, argp);
    fprintf(s_fd, "\n");
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_log_init() {
    static bool initialized = false;
    if(!initialized) {
        iw_log_add_level(IW_LOG_IW,     "General "INSTAWORKS" logs");
        iw_log_add_level(IW_LOG_SYSLOG, "Syslog messages");
        iw_log_add_level(IW_LOG_MEM,    "Memory allocation");
        iw_log_add_level(IW_LOG_WEB,    "Web server related logs");
        iw_log_add_level(IW_LOG_GUI,    "Web GUI logs");
        initialized = true;
    }
}

// --------------------------------------------------------------------------

void iw_log_list(FILE *out) {
    iw_log_init(); // Ensure that log levels have been added.

    int cnt;
    for(cnt=0;cnt < 32;cnt++) {
        if(s_levels[cnt].desc != NULL) {
            fprintf(out, "    0x%08X - %s\n",
                    1 << cnt,
                    s_levels[cnt].desc);
        }
    }
}

// --------------------------------------------------------------------------

void iw_log_set_level(const char *dev, unsigned int level) {
    bool dev_change = (dev == NULL && s_dev == NULL) ||
                      (dev != NULL && s_dev == NULL) ||
                      (dev == NULL && s_dev != NULL) ||
                      (dev != NULL && s_dev != NULL && (strcmp(dev, s_dev) != 0));

    // Only close and reopen device if the settings really change.
    if(level == s_log_level && !dev_change) {
        return;
    } else {
        // Notify change in log-level before the fact
        LOG(s_log_level, "Changing log level, old device=\"%s\", old level=\"%X\"",
            s_dev != NULL ? s_dev : "<none>", s_log_level);
    }
    // A log level of zero means turning off the logs
    if(dev_change || level == 0) {
        // Temporary file descriptor variable. Make sure we can open
        // new file log output device before we close the old one.
        FILE *tmp_fd = NULL;
        if(level != 0 && dev != NULL) {
            if(strcmp(dev, "stdout") == 0) {
                tmp_fd = stdout;
            } else {
                tmp_fd = fopen(dev, "w");
            }
            if(tmp_fd == NULL) {
                LOG(s_log_level, "Failed to open log device \"%s\"", dev);
                return;
            }
        }

        if(s_fd != NULL) {
            // Note the new log level and device on the old
            // device before closing it.
            LOG(level, "Changed log level, new device=\"%s\", new level=\"%X\"",
                dev != NULL ? dev : "<none>", level);
            if(s_dev != NULL && strcmp(s_dev, "stdout") != 0) {
                fclose(s_fd);
            }
            s_fd = NULL;
            free(s_dev);
        }

        // Finally set the device and fd to the opened device and fd.
        if(level != 0 && tmp_fd != NULL) {
            s_dev = strdup(dev);
            s_fd  = tmp_fd;
        }
    }
    s_log_level = level;

    // Notify change in log-level after the fact.
    LOG(s_log_level, "Changed log level, new device=\"%s\", new level=\"%X\"",
        s_dev != NULL ? s_dev : "<none>", s_log_level);
}

// --------------------------------------------------------------------------

bool iw_log_add_level(
    unsigned int level,
    const char *desc)
{
    // Make sure that the level is a single bit.
    int clear = level & (level - 1);
    if(clear != 0) {
        // There was more than one bit set.
        return false;
    }
    int cnt = 0;
    for(cnt=0;!(level & 1) && cnt < 32;cnt++) {
        level >>= 1;
    }
    if(cnt == 32) {
        // Can't have more than 32 log levels.
        return false;
    }
    if(s_levels[cnt].desc != NULL) {
        // There was already a log level defined for this level.
        return false;
    }
    s_levels[cnt].desc = desc;
    return true;
}

// --------------------------------------------------------------------------

void iw_log(const char *file, unsigned int line, const char *msg, ...) {
    va_list ap;
    va_start(ap, msg);
    iw_vlog(file, line, msg, ap);
    va_end(ap);
}

// --------------------------------------------------------------------------

void iw_log_ex(
    unsigned int lvl,
    const char *file,
    unsigned int line,
    const char *msg, ...)
{
    if(DO_LOG(lvl)) {
        va_list ap;
        va_start(ap, msg);
        iw_vlog(file, line, msg, ap);
        va_end(ap);
    }
}

// --------------------------------------------------------------------------
