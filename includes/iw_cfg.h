// --------------------------------------------------------------------------
///
/// @file iw_cfg.h
///
/// This file controls the configuration for the InstaWorks library. To change
/// the settings, just write the new value to the corresponding variable
/// before calling iw_main().
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_CFG_H_
#define _IW_CFG_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_val_store.h"

#include <stdbool.h>

// TODO: Add notification callbacks so that programs can be notified when a
// setting changes.

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

#define IW_CFG                          "cfg"
#define IW_CFG_OPT                      IW_CFG ".opt"

#define IW_CFG_CMD_PORT                 IW_CFG ".cmdport"
#define IW_DEF_CMD_PORT                 10000
#define IW_CFG_FOREGROUND               IW_CFG ".foreground"
#define IW_DEF_FOREGROUND               0
#define IW_CFG_FOREGROUND_OPT           IW_CFG_OPT ".foreground"
#define IW_DEF_FOREGROUND_OPT           "f"
#define IW_CFG_DAEMONIZE                IW_CFG ".daemonize"
#define IW_DEF_DAEMONIZE                0
#define IW_CFG_DAEMONIZE_OPT            IW_CFG ".daemonize.opt"
#define IW_DEF_DAEMONIZE_OPT            "d"
#define IW_CFG_LOGLEVEL                 IW_CFG ".loglvl"
#define IW_DEF_LOGLEVEL                 0
#define IW_CFG_LOGLEVEL_OPT             IW_CFG_OPT ".loglvl"
#define IW_DEF_LOGLEVEL_OPT             "l"
#define IW_CFG_ALLOW_QUIT               IW_CFG ".allowquit"
#define IW_DEF_ALLOW_QUIT               1
#define IW_CFG_CRASHHANDLER_ENABLE      IW_CFG ".crashhandler.enable"
#define IW_DEF_CRASHHANDLER_ENABLE      1
#define IW_CFG_CRASHHANDLER_FILE        IW_CFG ".crashhandler.file"
#define IW_DEF_CRASHHANDLER_FILE        "/tmp/callstack.txt"
#define IW_CFG_MEMTRACK_ENABLE          IW_CFG ".memtrack.enable"
#ifdef IW_NO_MEMORY_TRACKING
#define IW_DEF_MEMTRACK_ENABLE          0
#else
#define IW_DEF_MEMTRACK_ENABLE          1
#endif
#define IW_CFG_MEMTRACK_SIZE            IW_CFG ".memtrack.size"
#define IW_DEF_MEMTRACK_SIZE            10000
#define IW_CFG_HEALTHCHECK_ENABLE       IW_CFG ".healthcheck.enable"
#define IW_DEF_HEALTHCHECK_ENABLE       1
#define IW_CFG_WEBSRV_ENABLE            IW_CFG ".websrv.enable"
#define IW_DEF_WEBSRV_ENABLE            1
#define IW_CFG_SYSLOG_SIZE              IW_CFG ".syslog.size"
#define IW_DEF_SYSLOG_SIZE              10000
#define IW_CFG_PRG_NAME                 IW_CFG ".prgname"
#define IW_DEF_PRG_NAME                 "instaworks"

// --------------------------------------------------------------------------

/// The global settings variable. All InstaWorks settings can be set from here.
extern iw_val_store iw_cfg;

// --------------------------------------------------------------------------

/// The callback for shutdown.
typedef bool (*IW_SHUTDOWN_CB)();

// --------------------------------------------------------------------------

/// The set of callbacks to register with the library.
typedef struct _iw_callbacks {
    /// The shutdown callback. Should be set if the program needs to do
    /// cleanup at shutdown.
    IW_SHUTDOWN_CB shutdown;
} iw_callbacks;

// --------------------------------------------------------------------------

/// The global callback variable.
extern iw_callbacks iw_cb;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Initialize the configuration store.
extern void iw_cfg_init();

// --------------------------------------------------------------------------

/// @brief Destroy the configuration store.
extern void iw_cfg_exit();

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_CFG_H_

// --------------------------------------------------------------------------
