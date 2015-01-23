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

/// The top-level config path.
#define IW_CFG                          "cfg"
/// The options config path.
#define IW_CFG_OPT                      IW_CFG ".opt"
/// The command port to use.
#define IW_CFG_CMD_PORT                 IW_CFG ".cmdport"
/// The default command port value.
#define IW_DEF_CMD_PORT                 10000
/// The run-in-foreground flag.
#define IW_CFG_FOREGROUND               IW_CFG ".foreground"
/// The default run-in-foreground flag value.
#define IW_DEF_FOREGROUND               0
/// The run-in-foreground command line option character.
#define IW_CFG_FOREGROUND_OPT           IW_CFG_OPT ".foreground"
/// The default run-in-foreground command line option character.
#define IW_DEF_FOREGROUND_OPT           "f"
/// The daemonize flag.
#define IW_CFG_DAEMONIZE                IW_CFG ".daemonize"
/// The default daemonize flag value.
#define IW_DEF_DAEMONIZE                0
/// The damonize command line option character.
#define IW_CFG_DAEMONIZE_OPT            IW_CFG ".daemonize.opt"
/// The default daemonize command line option character.
#define IW_DEF_DAEMONIZE_OPT            "d"
/// The log-level setting.
#define IW_CFG_LOGLEVEL                 IW_CFG ".loglvl"
/// The default log-level setting.
#define IW_DEF_LOGLEVEL                 0
/// The log-level command line option character.
#define IW_CFG_LOGLEVEL_OPT             IW_CFG_OPT ".loglvl"
/// The default log-level command line option character.
#define IW_DEF_LOGLEVEL_OPT             "l"
/// The allow-quit flag, true if the program should have a 'quit' command.
#define IW_CFG_ALLOW_QUIT               IW_CFG ".allowquit"
/// The default allow-quit flag value.
#define IW_DEF_ALLOW_QUIT               1
/// The crash-handler enable flag.
#define IW_CFG_CRASHHANDLER_ENABLE      IW_CFG ".crashhandler.enable"
/// The default crash-handler enable flag value.
#define IW_DEF_CRASHHANDLER_ENABLE      1
/// The crash-handler file to use for crash call-stacks.
#define IW_CFG_CRASHHANDLER_FILE        IW_CFG ".crashhandler.file"
/// The default crash-handler file.
#define IW_DEF_CRASHHANDLER_FILE        "/tmp/callstack.txt"
/// The memory tracker enable flag.
#define IW_CFG_MEMTRACK_ENABLE          IW_CFG ".memtrack.enable"
#ifdef IW_NO_MEMORY_TRACKING
/// The default memory tracker enable flag value.
#define IW_DEF_MEMTRACK_ENABLE          0
#else
/// The default memory tracker enable flag value.
#define IW_DEF_MEMTRACK_ENABLE          1
#endif
/// The memory tracker hash table size.
#define IW_CFG_MEMTRACK_SIZE            IW_CFG ".memtrack.size"
/// The memory tracker default hash table size.
#define IW_DEF_MEMTRACK_SIZE            10000
/// The health check enable flag.
#define IW_CFG_HEALTHCHECK_ENABLE       IW_CFG ".healthcheck.enable"
/// The default health check enable flag value.
#define IW_DEF_HEALTHCHECK_ENABLE       1
/// The web server enable flag.
#define IW_CFG_WEBSRV_ENABLE            IW_CFG ".websrv.enable"
/// The default web server enable flag value.
#define IW_DEF_WEBSRV_ENABLE            1
/// The syslog backlog size.
#define IW_CFG_SYSLOG_SIZE              IW_CFG ".syslog.size"
/// The default syslog backlog size value.
#define IW_DEF_SYSLOG_SIZE              10000
/// The program name.
#define IW_CFG_PRG_NAME                 IW_CFG ".prgname"
/// The default program name value.
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
