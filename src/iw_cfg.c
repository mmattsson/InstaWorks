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

/// Add a number to the configuration settings.
/// @param n The name of the setting.
/// @param r The range of the number.
#define ADD_NUM(n,r) iw_cfg_add_number(IW_CFG_##n,r,IW_DEF_##n)

/// Add a port number to the configuration settings. The allowed range for the
/// port number is 0-65535.
/// @param n The name of the setting.
#define ADD_PORT(n)  ADD_NUM(n,IW_VAL_CRIT_PORT)

/// Add a boolean to the configuration settings. The allowed range for the
/// number is 0 or 1.
/// @param n The name of the setting.
#define ADD_BOOL(n)  ADD_NUM(n,IW_VAL_CRIT_BOOL)

/// Add a string to the configuration settings.
/// @param n The name of the setting.
/// @param r The regular expression for the string.
#define ADD_STR(n,r) iw_cfg_add_string(IW_CFG_##n,r,IW_DEF_##n)

/// Add a character to the configuration settings. The string can only be
/// one character long.
/// @param n The name of the setting.
#define ADD_CHAR(n)  ADD_STR(n,IW_VAL_CRIT_CHAR)

// --------------------------------------------------------------------------

iw_val_store iw_cfg;

// --------------------------------------------------------------------------
//
// Data structures
//
// --------------------------------------------------------------------------

iw_callbacks iw_cb = {
        NULL
};

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

static void iw_cfg_add_number(const char *name, const char *regexp, int def) {
    if(regexp != NULL) {
        iw_val_store_add_name_regexp(&iw_cfg, name, IW_VAL_TYPE_NUMBER, regexp);
    } else {
        iw_val_store_add_name(&iw_cfg, name, IW_VAL_TYPE_NUMBER);
    }
    iw_val_store_set_number(&iw_cfg, name, def);
}

// --------------------------------------------------------------------------

static void iw_cfg_add_string(const char *name, const char *regexp, const char *def) {
    if(regexp != NULL) {
        iw_val_store_add_name_regexp(&iw_cfg, name, IW_VAL_TYPE_STRING, regexp);
    } else {
        iw_val_store_add_name(&iw_cfg, name, IW_VAL_TYPE_STRING);
    }
    if(def != NULL) {
        iw_val_store_set_string(&iw_cfg, name, def);
    }
}

// --------------------------------------------------------------------------

void iw_cfg_init() {
    static bool initialized = false;
    if(initialized) {
        return;
    }
    initialized = true;

    iw_val_store_initialize(&iw_cfg, true);

    ADD_PORT(CMD_PORT);
    ADD_BOOL(FOREGROUND);
    ADD_CHAR(FOREGROUND_OPT);
    ADD_BOOL(DAEMONIZE);
    ADD_CHAR(DAEMONIZE_OPT);
    ADD_NUM(LOGLEVEL, NULL);
    ADD_CHAR(LOGLEVEL_OPT);
    ADD_BOOL(ALLOW_QUIT);
    ADD_BOOL(CRASHHANDLER_ENABLE);
    ADD_STR(CRASHHANDLER_FILE, NULL);
    ADD_BOOL(MEMTRACK_ENABLE);
    ADD_NUM(MEMTRACK_SIZE, NULL);
    ADD_BOOL(HEALTHCHECK_ENABLE);
    ADD_BOOL(WEBGUI_ENABLE);
    ADD_STR(WEBGUI_CSS_FILE, NULL);
    ADD_NUM(SYSLOG_SIZE, NULL);
    ADD_STR(PRG_NAME, NULL);
    ADD_STR(PRG_ABOUT, NULL);
}

// --------------------------------------------------------------------------

void iw_cfg_exit() {
    iw_val_store_destroy(&iw_cfg);
}

// --------------------------------------------------------------------------
