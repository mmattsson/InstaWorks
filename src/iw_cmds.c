// --------------------------------------------------------------------------
///
/// @file iw_cmds.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_cmds.h"
#include "iw_cmds_int.h"

#include "iw_htable.h"
#include "iw_log_int.h"
#include "iw_main.h"
#include "iw_memory_int.h"
#include "iw_mutex_int.h"
#include "iw_cfg.h"
#include "iw_syslog.h"
#include "iw_thread_int.h"
#include "iw_util.h"
#include "iw_version.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/// The width of the command column
#define IW_CMD_WIDTH    16

// --------------------------------------------------------------------------
//
// Local variables
//
// --------------------------------------------------------------------------

static iw_cmd_info s_root;

// --------------------------------------------------------------------------
//
// Helper functions
//
// --------------------------------------------------------------------------

/// @brief Show the command help text.
/// @param out The output file stream to print the help on.
/// @param cinfo The command information structure for the command.
/// @param unknown The command string if an unknown command was entered.
static void iw_cmds_help(FILE *out, iw_cmd_info *cinfo, const char *unknown) {
    fprintf(out, "\n");
    if(unknown != NULL) {
        if(strcasecmp(unknown, "help") != 0) {
            fprintf(out,
                    "Unknown command: %s\n\n", unknown);
        }
    } else if(cinfo->help != NULL) {
        fprintf(out,
                "%s\n\n",
                cinfo->help);
    }

    // Then print out all command nodes under this command
    unsigned long hash;
    iw_cmd_info *child = (iw_cmd_info *)iw_htable_get_first(&cinfo->children,
                                                            &hash);
    if(child != NULL) {
        fprintf(out, "The following sub-commands are available:\n");
        // TODO: Alphabetize the commands
        while(child != NULL) {
            fprintf(out, " %-*s %s\n", IW_CMD_WIDTH, child->cmd, child->info);

            // Continue to check the next node in the hash table.
            child = (iw_cmd_info *)iw_htable_get_next(&cinfo->children, &hash);
        }
    } else {
        fprintf(out, "\n");
    }
}

// --------------------------------------------------------------------------

/// @brief Find the parent command with the given name.
/// Searches through the hierarcy to find the command with the given name.
/// This can then be used to add a child command under this parent.
/// @param table The table to start searching from.
/// @param parent The name of the parent command.
/// @return The command info structure of the parent command.
static iw_cmd_info *iw_cmd_find_parent(iw_htable *table, const char *parent) {
    unsigned long hash;

    // First try to access the hash table to see if this node contains
    // the parent. If it does, we don't have to search further.
    iw_cmd_info *cinfo = (iw_cmd_info *)iw_htable_get(table,
                                                      strlen(parent),
                                                      parent);
    if(cinfo != NULL) {
        return cinfo;
    }

    // It didn't contain the parent. Now we need to traverse the
    // hierarchy from here. Iterate through all children to find the
    // parent node for this command.
    cinfo = (iw_cmd_info *)iw_htable_get_first(table, &hash);
    while(cinfo != NULL) {
        // Search through the children as well.
        iw_cmd_info *retval = iw_cmd_find_parent(&cinfo->children, parent);
        if(retval != NULL) {
            return retval;
        }

        // Continue to check the next node in the hash table.
        cinfo = (iw_cmd_info *)iw_htable_get_next(table, &hash);
    }

    if(cinfo == NULL) {
        // Failed to find the parent.
        return NULL;
    }

    table = &cinfo->children;

    return NULL;
}

// --------------------------------------------------------------------------

/// @brief Process the command.
/// Parses the command given in the parse information and tries to execute
/// the command. If the command is entered incorrectly or is missing
/// parameters, a help text is printed.
/// @param parent The command node parsed so far.
/// @param info The parse information for further parsing.
/// @param out The output file stream to print responses to.
/// @return True if the command was successfully processed.
static bool iw_cmds_process_internal(
    iw_cmd_info *parent,
    iw_cmd_parse_info *info,
    FILE *out)
{
    char *cmd = info->token;
    if(cmd != NULL) {
        iw_cmd_info *cinfo = (iw_cmd_info *)iw_htable_get(&parent->children,
                                                          strlen(cmd),
                                                          cmd);
        if(cinfo != NULL) {
            if(cinfo->cmd_fn != NULL) {
                // There's a command associated with this node. Let's call
                // the command function and return.
                return cinfo->cmd_fn(out, cmd, info);
            } else {
                // There's no command associated with this node. Let's try
                // to get another token to see if there's more nodes to drill
                // down to.
                char *token = iw_cmd_get_token(info);
                if(token != NULL) {
                    // We found another token, go ahead and process that token
                    return iw_cmds_process_internal(cinfo, info, out);
                } else {
                    // No more tokens, and no command function. Let's just
                    // print the help and return.
                    iw_cmds_help(out, cinfo, NULL);
                    return false;
                }
            }
        }
    }

    // Couldn't find the command under this node. Let's just print out
    // the help and return.
    iw_cmds_help(out, parent, cmd);
    return false;
}

// --------------------------------------------------------------------------
//
// Built-in command functions
//
// --------------------------------------------------------------------------

static bool cmd_help(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    iw_cmds_help(out, &s_root, "help");
    return true;
}

// --------------------------------------------------------------------------

static bool cmd_thread_dump(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    iw_thread_dump(out);
    return true;
}

// --------------------------------------------------------------------------

static bool cmd_mutex_dump(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    iw_mutex_dump(out);
    return true;
}

// --------------------------------------------------------------------------

static bool cmd_memory_show(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    iw_memory_show(out);
    return true;
}

// --------------------------------------------------------------------------

static bool cmd_memory_summary(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    iw_memory_summary(out);
    return true;
}

// --------------------------------------------------------------------------

static bool cmd_memory_brief(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    iw_memory_brief(out);
    return true;
}

// --------------------------------------------------------------------------

static bool cmd_syslog_dump(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    iw_syslog_display(out);
    return true;
}

// --------------------------------------------------------------------------

static bool cmd_syslog_clear(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    iw_syslog_clear(out);
    return true;
}

// --------------------------------------------------------------------------

static bool cmd_iwver(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    fprintf(out, INSTAWORKS" version %s", IW_VER_STR);
    return true;
}

// --------------------------------------------------------------------------

static bool cmd_quit(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    fprintf(out, "Shutting down");
    if(iw_cb.shutdown != NULL) {
        iw_cb.shutdown();
    }
    iw_exit();
    exit(0);
}

// --------------------------------------------------------------------------

static bool cmd_callstack(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    char *threadidstr = iw_cmd_get_token(info);
    if(threadidstr == NULL) {
        fprintf(out, "Missing parameters\n");
        return false;
    }
    errno = 0;
    unsigned int threadid = strtoul(threadidstr, NULL, 16);
    if(errno != 0) {
        fprintf(out, "Invalid thread id\n");
        return false;
    }

    iw_thread_callstack(out, threadid);
    return true;
}

// --------------------------------------------------------------------------

static void cmd_log_help(FILE *out) {
    fprintf(out,
            "\n"
            "Usage: log lvl <level> <device>\n"
            " The <level> is the desired log level. The log level is a sum of individual\n"
            " levels in either decimal or hexadecimal. The <device> is either a file path\n"
            " to a file or a tty or the actual word 'stdout' to send the logs to standard output.\n"
            "\n"
            " To disable logging, set the log level to zero.\n"
            "\n"
            "Examples:\n"
            " $ %s log lvl 0xF `tty`\n"
            "or\n"
            " $ %s log lvl 8 stdout\n"
            "\n",
            iw_val_store_get_string(&iw_cfg, IW_CFG_PRG_NAME),
            iw_val_store_get_string(&iw_cfg, IW_CFG_PRG_NAME));
    fprintf(out, "The following log levels are available:\n");
    iw_log_list(out);
}

// --------------------------------------------------------------------------

static bool cmd_log_lvl(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    char *lvlstr = iw_cmd_get_token(info);
    char *dev = iw_cmd_get_token(info);
    if(lvlstr == NULL) {
        fprintf(out, "\nMissing parameter\n");
        cmd_log_help(out);
        return false;
    }
    long long int lvl;
    if(!iw_strtoll(lvlstr, &lvl, 16)) {
        fprintf(out, "\nInvalid log level\n");
        cmd_log_help(out);
        return false;
    }
    if(lvl != 0 && dev == NULL) {
        fprintf(out, "\n    Missing parameter\n");
        cmd_log_help(out);
        return false;
    }

    iw_log_set_level(dev, lvl);
    return true;
}

// --------------------------------------------------------------------------

static void cmd_log_thread_help(FILE *out) {
    fprintf(out,
            "\n"
            "Usage: log thread <thread> <on|off>\n"
            " The <thread> is either the thread ID of the thread to enable or disable logging\n"
            " for or the word 'all' for all threads. By default, all threads have logging enabled.\n"
            " To enable logging for just one thread, do 'log thread all off' followed by\n"
            " 'log thread <id> on'. This command will not affect log levels as set by 'log lvl'.\n"
            "\n"
            "Examples:\n"
            " $ %s log thread all off\n"
            "or\n"
            " $ %s log thread 0x1234abcd on\n"
            "\n",
            iw_val_store_get_string(&iw_cfg, IW_CFG_PRG_NAME),
            iw_val_store_get_string(&iw_cfg, IW_CFG_PRG_NAME));
}

// --------------------------------------------------------------------------

static bool cmd_log_thread(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    long long int threadid;
    bool log_on = false;
    char *threadstr = iw_cmd_get_token(info);
    char *onoffstr  = iw_cmd_get_token(info);

    // Make sure we have two parameters.
    if(threadstr == NULL || onoffstr == NULL) {
        fprintf(out, "\nMissing parameter\n");
        cmd_log_thread_help(out);
        return false;
    }

    // Check thread id parameter
    if(strcmp(threadstr, "all") == 0) {
        threadid = 0;
    } else if(!iw_strtoll(threadstr, &threadid, 16)) {
        fprintf(out, "\nInvalid parameter\n");
        cmd_log_thread_help(out);
    }

    // Check on/off parameter
    if(strcmp(onoffstr, "on") == 0) {
        log_on = true;
    } else if(strcmp(onoffstr, "off") != 0) {
        fprintf(out, "\nInvalid parameter\n");
        cmd_log_thread_help(out);
        return false;
    }

    // At this point we have the thread id and the on/off setting
    if(threadid == 0) {
        // Set all threads to on or off
        iw_thread_set_log_all(log_on);
    } else {
        // Set just the given thread to on or off
        if(!iw_thread_set_log(threadid, log_on)) {
            fprintf(out, "\nInvalid thread ID\n");
            cmd_log_thread_help(out);
            return false;
        }
    }

    return true;
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

bool iw_cmd_init() {
    memset(&s_root, 0, sizeof(s_root));

    iw_htable_init(&s_root.children, 100, false, NULL);

    iw_cmd_add(NULL, "help", cmd_help,
            "Display help", "Displays help for the possible commands.");
    iw_cmd_add(NULL, "threads", cmd_thread_dump,
            "Display thread information", "Display information for all the threads running in the process.");
    iw_cmd_add(NULL, "mutexes", cmd_mutex_dump,
            "Display mutex information", "Display information for all the mutexes created in the process.");
    iw_cmd_add(NULL, "callstack", cmd_callstack,
            "Display callstacks for a given thread", "Displays the callstack for the given thread ID.");
    iw_cmd_add(NULL, "log", NULL,
            "Log-related commands", "Commands related to debug log settings.");
    iw_cmd_add("log", "lvl", cmd_log_lvl,
            "Set the program log level", "Enables debug log output with the given log level.");
    iw_cmd_add("log", "thread", cmd_log_thread,
            "Enables or disables logging for threads", "Enables or disables logging for individual threads.");
    iw_cmd_add(NULL, "memory", NULL,
            "Display memory information", "Displays the memory allocated by the process.");
    iw_cmd_add("memory", "show", cmd_memory_show,
            "Display all allocations", "Displays all the memory allocated by the process.");
    iw_cmd_add("memory", "summary", cmd_memory_summary,
            "Display a summary of allocations",
            "Displays the memory allocated by the process. Only shows the number of allocation\n"
            "for a given file, line, and size.");
    iw_cmd_add("memory", "brief", cmd_memory_brief,
            "Display a brief summary of allocations",
            "Displays the top number of places where memory was allocated by the process.");
    iw_cmd_add(NULL, "syslog", NULL,
            "Execute a syslog related command", "Commands related to syslogs.");
    iw_cmd_add("syslog", "show", cmd_syslog_dump,
            "Display the syslog buffer", "Displays the syslogs sent by the process.");
    iw_cmd_add("syslog", "clear", cmd_syslog_clear,
            "Clear the syslog buffer", "Clears all messages from the syslog buffer.");
    iw_cmd_add(NULL, "iwver", cmd_iwver,
            "Displays "INSTAWORKS" version", "Displays the "INSTAWORKS" version information.");

    int *allow = iw_val_store_get_number(&iw_cfg, IW_CFG_ALLOW_QUIT);
    if(allow && *allow) {
        iw_cmd_add(NULL, "quit", cmd_quit,
                "Shut down the program", "Sends a command to the running program that causes it to shut down");
    }

    return true;
}

// --------------------------------------------------------------------------

bool iw_cmd_add(
    const char *parent,
    const char *cmd,
    IW_CMD_FN cmd_fn,
    const char *info,
    const char *help)
{
    iw_cmd_info *node = NULL;

    // First we need to find the given parent (or the top node if NULL is
    // passed in)
    if(parent == NULL) {
        node = &s_root;
    } else {
        node = iw_cmd_find_parent(&s_root.children, parent);
    }

    if(node == NULL) {
        LOG(IW_LOG_IW, "Failed to find parent \"%s\", cannot add command \"%s\"",
                parent, cmd);
        return false;
    }

    // Now we should have the parent, create a new node, populate it and
    // add the node to the parent.
    iw_cmd_info *cinfo = (iw_cmd_info *)calloc(sizeof(iw_cmd_info), 1);
    cinfo->cmd = strdup(cmd);
    cinfo->info = strdup(info);
    cinfo->help = strdup(help);
    cinfo->cmd_fn = cmd_fn;
    cinfo->parent = node;
    iw_htable_init(&cinfo->children, 32, false, NULL);
    iw_htable_insert(&node->children, strlen(cmd), (void *)cmd, cinfo);

    return true;
}

// --------------------------------------------------------------------------

char *iw_cmd_get_token(void *info) {
    iw_cmd_parse_info *inf = (iw_cmd_parse_info *)info;
    inf->token = strtok_r(NULL, " ", &(inf->saveptr));

    return inf->token;
}

// --------------------------------------------------------------------------

bool iw_cmds_process(iw_cmd_parse_info *info, FILE *out) {
    return iw_cmds_process_internal(&s_root, info, out);
}

// --------------------------------------------------------------------------
