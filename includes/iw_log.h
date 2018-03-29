// --------------------------------------------------------------------------
///
/// @file iw_log.h
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_LOG_H_
#define _IW_LOG_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

// --------------------------------------------------------------------------

// Pre-defined log levels
#define IW_LOG_IW      0x1   ///< The basic informational log level
#define IW_LOG_SYSLOG  0x2   ///< Show syslogs
#define IW_LOG_MEM     0x4   ///< Show memory tracking logs
#define IW_LOG_WEB     0x8   ///< Web-related logs
#define IW_LOG_GUI     0x10  ///< Web-GUI related logs

// --------------------------------------------------------------------------

/// @brief Check whether a given log level should be logged.
#define DO_LOG(LVL)         (LVL & s_log_level)

// --------------------------------------------------------------------------

/// @brief A logging macro to call when outputting a log message.
/// @param LVL The log level to use.
/// @param MSG The message to output.
#define LOG(LVL, MSG...)   { if(DO_LOG(LVL)) { iw_log(__FILE__, __LINE__, MSG); } }

// --------------------------------------------------------------------------

/// @brief A logging macro to call when outputting a log message.
/// @param LVL The log level to use.
/// @param MSG The message to output.
#define LOG_EX(LVL, MSG...)    iw_log_ex(LVL, __FILE__, __LINE__, MSG)

// --------------------------------------------------------------------------

/// The program's current log level.
extern unsigned int s_log_level;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Set the log level for the program.
/// @param dev The device to output logs to.
/// @param level The new log level to use, zero disabled logging.
extern void iw_log_set_level(const char *dev, unsigned int level);

// --------------------------------------------------------------------------

/// @brief Add a log level to the existing levels of logging.
/// This adds a new level to log. Any log issued with this level will be shown
/// if the log level is enabled. The log level must be a single bit, e.g. 0x1,
/// 0x2, 0xF, 0x10, etc.
/// @param level The level to add, a bit number between 1 and 32.
/// @param desc The brief description of the log level.
/// @return True if the log level was successfully added.
extern bool iw_log_add_level(
    unsigned int level,
    const char *desc);

// --------------------------------------------------------------------------

/// @brief List the current log level commands.
/// @param out The file descriptor to display the log levels on.
extern void iw_log_list(FILE *out);

// --------------------------------------------------------------------------
//
// Private API - should not be called directly. Use macros above instead.
//
// --------------------------------------------------------------------------

/// @brief The log function for outputting log messages.
/// This function should not be used directly. Instead call the LOG macro.
/// @param file The file that the message is output from.
/// @param line The line that the message is output from.
/// @param msg The message to output.
extern void iw_log(
    const char *file,
    unsigned int line,
    const char *msg, ...);

// --------------------------------------------------------------------------

/// @brief The log function for outputting log messages.
/// This function should not be used directly. Instead call the LOG macro.
/// This function differs from iw_log() in that no if statement is needed in
/// the macro. Instead, the log level check is performed in the function call.
/// The trade off is that the function call is made whether or not the message
/// is logged, but the up-side is that this can be used as an operand for a
/// comma operator.
/// @param lvl The log level to use.
/// @param file The file that the message is output from.
/// @param line The line that the message is output from.
/// @param msg The message to output.
extern void iw_log_ex(
    unsigned int lvl,
    const char *file,
    unsigned int line,
    const char *msg, ...);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_LOG_H_

// --------------------------------------------------------------------------
