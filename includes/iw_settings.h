// --------------------------------------------------------------------------
///
/// @file iw_settings.h
///
/// This file controls the settings for the InstaWorks library. To change
/// the settings, just write the new value to the corresponding variable
/// before calling iw_main().
///
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#ifndef _IW_SETTINGS_H_
#define _IW_SETTINGS_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------

/// The default command control port to use for the program.
#define IW_DEF_COMMAND_PORT         10000

/// The default foreground option character.
#define IW_DEF_OPT_FOREGROUND       'f'

/// The default log-level option character.
#define IW_DEF_OPT_LOG_LEVEL        'l'

/// The default memory tracking enabled setting.
#ifdef IW_NO_MEMORY_TRACKING
#define IW_DEF_MEMTRACK_ENABLE      false
#else
#define IW_DEF_MEMTRACK_ENABLE      true
#endif

/// The default memory tracking hash table size.
#define IW_DEF_MEMTRACK_SIZE        10000

/// The default log level.
#define IW_DEF_LOG_LEVEL            0

/// The default syslog buffer size.
#define IW_DEF_SYSLOG_SIZE          10000

/// The default healthcheck enabled setting.
#define IW_DEF_HEALTHCHECK_ENABLE   true

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// The InstaWorks settings structure.
typedef struct _iw_settings {
    /// The command control port number to use for this program.
    unsigned short iw_cmd_port;

    /// Comand line option character settings. If an options should not be
    /// used, '\0' can be assigned to the option.
    struct {
        /// The option character to foreground the process. This in effect
        /// tells the program to run as a server.
        char foreground;

        /// The option character to set a log level when running the process.
        char log_level;
    } iw_cmd_line;

    /// True if the program should be run in the foreground as a server.
    bool iw_foreground;

    /// True if the memory tracking module should be used (default is true)
    bool iw_memtrack_enable;

    /// The size of the memory tracking hash table (default is 10000)
    int  iw_memtrack_hash_size;

    /// The log level to use.
    long int iw_log_level;

    /// The size of the syslog buffer (default is 10000)
    int  iw_syslog_size;

    /// True if health-check should be enabled (default is true).
    bool iw_healthcheck_enable;

    /// The program name
    const char *iw_prg_name;

} iw_settings;

// --------------------------------------------------------------------------

/// The global settings variable. All InstaWorks settings can be set from here.
extern iw_settings iw_stg;

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_SETTINGS_H_

// --------------------------------------------------------------------------
