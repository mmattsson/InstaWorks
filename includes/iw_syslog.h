// --------------------------------------------------------------------------
///
/// @file iw_syslog.h
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_SYSLOG_H_
#define _IW_SYSLOG_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>
#include <syslog.h>

// --------------------------------------------------------------------------
//
// Defines
//
// --------------------------------------------------------------------------

/// @brief A syslog wrapper used to send a syslog message.
/// This wrapper takes care of printing the syslog message on the debug logs
/// as well as doing an actual syslog.
/// @param prio The priority of the message.
/// @param lvl The log level to use.
/// @param fmt The format of the message string.
#define IW_SYSLOG(prio, lvl, fmt, ...)   \
    ( LOG_EX((IW_LOG_SYSLOG|lvl), fmt, __VA_ARGS__), iw_syslog(prio, fmt, __VA_ARGS__) )

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Restart the syslog module.
/// Free's up all allocated resources and re-initializes using the new
/// buffer size given.
/// @param buff_size The size of the buffer to use for syslog messages.
extern void iw_syslog_reinit(int buff_size);

// --------------------------------------------------------------------------

/// @brief Terminate the syslog module.
/// Free all allocated resources.
extern void iw_syslog_exit();

// --------------------------------------------------------------------------

/// @brief Display all syslog messages in the buffer.
/// @param out The output file stream to print the messages on.
extern void iw_syslog_display(FILE *out);

// --------------------------------------------------------------------------

/// @brief Clear the syslog buffer.
extern void iw_syslog_clear();

// --------------------------------------------------------------------------

/// @brief Add a syslog message to the buffer.
/// @param priority The priority of the syslog message.
/// @param fmt The format of the message.
extern void iw_syslog(int priority, const char *fmt, ...);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_SYSLOG_H_

// --------------------------------------------------------------------------
