// --------------------------------------------------------------------------
///
/// @file iw_cmds.h
///
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#ifndef _IW_CMDS_H_
#define _IW_CMDS_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_htable.h"

#include <stdio.h>

// --------------------------------------------------------------------------
//
// Data structures
//
// --------------------------------------------------------------------------

/// @brief The client request information.
/// The structure containing the information needed to parse the client request.
typedef struct _iw_cmd_parse_info {
    char *token;    ///< A token that has been read (if any).
    char *saveptr;  ///< The save pointer to use for subsequent calls.
} iw_cmd_parse_info;

// --------------------------------------------------------------------------

/// @brief The command function
/// Called when a command is being executed.
/// @param out The output file descriptor to write the response to.
/// @param cmd The command that was called.
/// @param info The request parsing information.
/// @return True if the command was successfully executed.
typedef bool (*IW_CMD_FN)(FILE *out, const char *cmd, iw_cmd_parse_info *info);

// --------------------------------------------------------------------------

/// @brief The command information.
/// The structure containing the command information needed to execute a
/// command.
typedef struct _iw_cmd_info {
    char *cmd;          ///< The command.
    char *info;         ///< Short, informational description of the command.
    char *help;         ///< Longer help text regarding the command.
    iw_htable children; ///< Children nodes for this command.
    IW_CMD_FN cmd_fn;   ///< The command function to call for this command.
    struct _iw_cmd_info *parent; ///< The parent command for this node.
} iw_cmd_info;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Initialize the command module.
/// @return True if the initialization was successful
extern bool iw_cmd_init();

// --------------------------------------------------------------------------

/// @brief Add a command to an existing node.
/// @param parent The parent node to add the command to or NULL if the command
///        is a top-level command.
/// @param cmd The command to add.
/// @param cmd_fn The command function to call if the command is issued.
/// @param info The info text for the command.
/// @param help The help text for the command.
/// @return True if the command was successfully added.
extern bool iw_cmd_add(
        const char *parent,
        const char *cmd,
        IW_CMD_FN cmd_fn,
        const char *info,
        const char *help);

// --------------------------------------------------------------------------

/// @brief Return the next token to process for client requests.
/// @param info The command info object.
extern char *iw_cmd_get_token(void *info);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_CMDS_H_

// --------------------------------------------------------------------------
