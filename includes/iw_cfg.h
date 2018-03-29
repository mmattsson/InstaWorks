// --------------------------------------------------------------------------
///
/// @file iw_cfg.h
///
/// This file controls the configuration for the InstaWorks library. To change
/// the settings, just write the new value to the corresponding variable
/// before calling iw_main().
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
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
#define IW_CFG_DAEMONIZE_OPT            IW_CFG_OPT ".daemonize"
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
/// The web GUI server enable flag.
#define IW_CFG_WEBGUI_ENABLE            IW_CFG ".webgui.enable"
/// The default web GUI server enable flag value.
#define IW_DEF_WEBGUI_ENABLE            1
/// The web GUI CSS style sheet file to use (if any)
#define IW_CFG_WEBGUI_CSS_FILE          IW_CFG ".webgui.css"
/// No CSS style sheet file by default
#define IW_DEF_WEBGUI_CSS_FILE          ""
/// The syslog backlog size.
#define IW_CFG_SYSLOG_SIZE              IW_CFG ".syslog.size"
/// The default syslog backlog size value.
#define IW_DEF_SYSLOG_SIZE              10000
/// The program name.
#define IW_CFG_PRG_NAME                 IW_CFG ".prgname"
/// The default program name value.
#define IW_DEF_PRG_NAME                 "InstaWorks"
/// The about information for the Web GUI display
#define IW_CFG_PRG_ABOUT                IW_CFG ".prgabout"
/// The default about information (NULL will print InstaWorks information.
#define IW_DEF_PRG_ABOUT                NULL

// --------------------------------------------------------------------------

/// The global settings variable. All InstaWorks settings can be set from here.
extern iw_val_store iw_cfg;

// --------------------------------------------------------------------------

/// The callback for shutdown.
typedef bool (*IW_SHUTDOWN_CB)();

// --------------------------------------------------------------------------

/// The callback for Web GUI run-time statistics.
typedef bool (*IW_RUNTIME_CB)(FILE *out);

// --------------------------------------------------------------------------

/// The set of callbacks to register with the library.
typedef struct _iw_callbacks {
    /// The shutdown callback. Should be set if the program needs to do
    /// cleanup at shutdown.
    IW_SHUTDOWN_CB shutdown;

    /// The run-time statistics callback. Should be set if the program wants to
    /// get a callback to display the run-time statistics in the built-in
    /// Web GUI.
    IW_RUNTIME_CB runtime;
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
/// This does not load configuration from file, this just sets up the
/// configuration store. You can add configuration values to the store
/// before calling \a iw_cfg_load(). The typical set of instructions would be:
/// @code
///    iw_cfg_init();
///    iw_val_store_set_number(...);
///    iw_val_store_set_string(...);
///    iw_cfg_load(cfg_file_name);
/// @endcode

extern void iw_cfg_init();

// --------------------------------------------------------------------------

/// @brief Load configuration settings from file.
/// The given file name is used to load settings. If no file name is given,
/// the file name used in a previous call to \a iw_cfg_load() or
/// \a iw_cfg_save() is used.
/// @param file The name of the configuration file to use.
extern bool iw_cfg_load(const char *file);

// --------------------------------------------------------------------------

/// @brief Save configuration settings to file.
/// The given file name is used to save settings. If no file name is given,
/// the file name used in a previous call to \a iw_cfg_load() or
/// \a iw_cfg_save() is used.
/// @param file The name of the configuration fiel to use or NULL.
/// @return True if the configuration was successfully saved.
extern bool iw_cfg_save(const char *file);

// --------------------------------------------------------------------------

/// @brief Destroy the configuration store.
extern void iw_cfg_exit();

// --------------------------------------------------------------------------

/// @brief Called to add a number to the configuration settings for the program.
/// Should be called after iw_cfg_init() but before iw_main(). This will add
/// a number with a given name, message, and regexp criteria, as well as a
/// default value.
/// @param name The name of the configuration setting to add.
/// @param persist True if the value should be peristed when loading or saving
///        the configuration to file.
/// @param msg The validation message to show in case the validation fails.
/// @param regexp The validation criteria to use.
/// @param def The default value.
extern void iw_cfg_add_number(
    const char *name,
    bool persist,
    const char *msg,
    const char *regexp,
    int def);

// --------------------------------------------------------------------------

/// @brief Called to add a string to the configuration settings for the program.
/// Should be called after iw_cfg_init() but before iw_main(). This will add
/// a string with a given name, message, and regexp criteria, as well as a
/// default value.
/// @param name The name of the configuration setting to add.
/// @param persist True if the value should be peristed when loading or saving
///        the configuration to file.
/// @param msg The validation message to show in case the validation fails.
/// @param regexp The validation criteria to use.
/// @param def The default value.
extern void iw_cfg_add_string(
    const char *name,
    bool persist,
    const char *msg,
    const char *regexp,
    const char *def);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_CFG_H_

// --------------------------------------------------------------------------
