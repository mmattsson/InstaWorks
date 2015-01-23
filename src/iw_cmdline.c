// --------------------------------------------------------------------------
///
/// @file iw_cmdline.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_cmdline.h"
#include "iw_cmdline_int.h"

#include "iw_cfg.h"
#include "iw_htable.h"
#include "iw_log.h"
#include "iw_util.h"

#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------
//
// Typedefs
//
// --------------------------------------------------------------------------

/// @brief The command-line option information.
/// The structure containing the option information.
typedef struct _iw_opt_info {
    char *option;       ///< The command-line option.
    char *help;         ///< Longer help text regarding the option.

    bool mandatory;     ///< True if the option is mandatory.

    iw_opt *opt;        ///< The option value (if set by the user).

    IW_OPT_PROC_FN proc_fn;     ///< The argument processing function.
    IW_OPT_HELP_FN help_fn;     ///< A help display callback function.
} iw_opt_info;

// --------------------------------------------------------------------------

/// The indentation level for command-line options.
#define IW_OPT_INDENT "    "

// --------------------------------------------------------------------------
//
// Local variables
//
// --------------------------------------------------------------------------

// The hash table containing all the options.
static iw_htable s_options;

// The locally pre-defined options
static iw_opt s_foreground;
static iw_opt s_daemon;
static iw_opt s_loglevel;

// --------------------------------------------------------------------------
//
// Internal API
//
// --------------------------------------------------------------------------

void iw_cmdline_check_opts() {
    if(s_foreground.is_set) {
        iw_val_store_set_number(&iw_cfg, IW_CFG_FOREGROUND, s_foreground.val.flag);
    }
    if(s_daemon.is_set) {
        iw_val_store_set_number(&iw_cfg, IW_CFG_DAEMONIZE, s_daemon.val.flag);
    }
    if(s_loglevel.is_set) {
        iw_val_store_set_number(&iw_cfg, IW_CFG_LOGLEVEL, s_loglevel.val.num);
    }
}

// --------------------------------------------------------------------------

static bool iw_cmdline_help_log(const char *option) {
    printf(" -l <loglevel>\n"
           IW_OPT_INDENT"The <loglevel> is the desired log level. The log level is a sum of individual\n"
           IW_OPT_INDENT"levels in either decimal or hexadecimal.\n");
    iw_log_list(stdout);
    printf("\n");
    return true;
}

// --------------------------------------------------------------------------

static bool iw_cmdline_add_predefined_option(
    const char *ch,
    const char *help,
    IW_OPT_TYPE type,
    iw_opt *opt,
    IW_OPT_HELP_FN help_fn)
{

    if(ch == NULL || *ch == '\0') {
        // We won't add this option. Still return true since this isn't
        // a failure.
        return true;
    }

    {
        char opt_string[3] = { '-', *ch, '\0' };
        iw_cmdline_add_option(opt_string, help, false, type, opt, NULL, help_fn);
    }

    return true;
}

// --------------------------------------------------------------------------

static bool iw_cmdline_add_predefined_options() {
    iw_cmdline_add_predefined_option(
        iw_val_store_get_string(&iw_cfg, IW_CFG_FOREGROUND_OPT),
        IW_OPT_INDENT "Run the program in the foreground.",
        IW_OPT_FLAG, &s_foreground, NULL);
    iw_cmdline_add_predefined_option(
        iw_val_store_get_string(&iw_cfg, IW_CFG_DAEMONIZE_OPT),
        IW_OPT_INDENT "Run the process as a daemon.",
        IW_OPT_FLAG, &s_daemon, NULL);
    iw_cmdline_add_predefined_option(
        iw_val_store_get_string(&iw_cfg, IW_CFG_LOGLEVEL_OPT),
        NULL,
        IW_OPT_NUM, &s_loglevel, iw_cmdline_help_log);

    return true;
}

// --------------------------------------------------------------------------

IW_CMD_OPT_RET iw_cmdline_process(int *processed, int argc, char **argv) {
    bool found_opts = false;
    int cnt;

    // Start with clearing all option values. This may be needed if the
    // iw_cmdline_process() function is called multiple times.
    unsigned long hash;
    iw_opt_info *opt_info = (iw_opt_info *)iw_htable_get_first(&s_options, &hash);
    while(opt_info != NULL) {
        opt_info->opt->is_set = false;
        memset(&(opt_info->opt->val), 0, sizeof(opt_info->opt->val));
        opt_info = (iw_opt_info *)iw_htable_get_next(&s_options, &hash);
    }

    for(;*processed < argc;(*processed)++) {
        char *cur_argv = argv[*processed];
        iw_opt_info *opt_info = (iw_opt_info *)iw_htable_get(&s_options,
                                                             strlen(cur_argv),
                                                             cur_argv);
        if(opt_info == NULL) {
            // The option is not found. If the argument starts with a dash,
            // then consider this an unknown option, otherwise, it may be
            // further input to the program. In either case, let the main
            // program decide. Since this is not necessarily an invalid
            // result, we must set the options we've processed so far.
            iw_cmdline_check_opts();

            return (cur_argv[0] == '-') ?
                            IW_CMD_OPT_UNKNOWN :
                            (found_opts ? IW_CMD_OPT_OK : IW_CMD_OPT_NONE);
        }

        found_opts = true;
        switch(opt_info->opt->type) {
        case IW_OPT_FLAG :
            // Just a flag, set the flag and continue without updating the
            // counter.
            opt_info->opt->val.flag = true;
            break;
        case IW_OPT_CHAR :
            // A character, there must be at least one more argument and it
            // must be a character.
            if(*processed >= argc - 1 || strlen(argv[*processed+1]) != 1) {
                // No character present.
                return IW_CMD_OPT_INVALID;
            }
            opt_info->opt->val.ch = argv[*processed+1][0];
            (*processed)++; // Account for the parameter.
            break;
        case IW_OPT_NUM :
            // A number, there must be at least one more argument and it
            // must be a number.
            if(*processed >= argc - 1 ||
               !iw_strtoll(argv[*processed+1], &(opt_info->opt->val.num), 0))
            {
                // No number present.
                return IW_CMD_OPT_INVALID;
            }
            (*processed)++; // Account for the parameter.
            break;
        case IW_OPT_STR :
            // A string, there must be at least one more argument and it
            // should be interpreted as a string.
            if(*processed >= argc - 1) {
                return IW_CMD_OPT_INVALID;
            }
            opt_info->opt->val.str = argv[*processed+1];
            (*processed)++; // Account for the parameter.
            break;
        case IW_OPT_CALLBACK :
            // A callback. We should call the given function.
            (*processed)++;
            cnt = 0;
            if(!opt_info->proc_fn(&cnt,
                                  argc - *processed,
                                  argv + *processed,
                                  opt_info->opt))
            {
                return IW_CMD_OPT_INVALID;
            }
            // Update 'processed' so that we reflect the number of arguments
            // that the callback function handled.
            *processed += cnt;
            break;
        }
        opt_info->opt->is_set = true;
    }

    // Check for mandatory options. Make sure that all mandatory options
    // actually were set.
    opt_info = (iw_opt_info *)iw_htable_get_first(&s_options, &hash);
    while(opt_info != NULL) {
        if(opt_info->mandatory && !opt_info->opt->is_set) {
            return IW_CMD_OPT_INVALID;
        }
        opt_info = (iw_opt_info *)iw_htable_get_next(&s_options, &hash);
    }


    // Set the pre-defined option settings if applicable.
    iw_cmdline_check_opts();

    return found_opts ? IW_CMD_OPT_OK : IW_CMD_OPT_NONE;
}

// --------------------------------------------------------------------------

static const char *iw_cmdline_print_type(IW_OPT_TYPE type) {
    switch(type) {
    case IW_OPT_FLAG     : return "";
    case IW_OPT_CHAR     : return "<char>";
    case IW_OPT_NUM      : return "<number>";
    case IW_OPT_STR      : return "<string>";
    case IW_OPT_CALLBACK : return "";
    }
    return "";
}

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_cmdline_init() {
    iw_cmdline_add_predefined_options();
}

// --------------------------------------------------------------------------

void iw_cmdline_print_help() {
    unsigned long hash;
    iw_opt_info *opt_info = (iw_opt_info *)iw_htable_get_first(&s_options,
                                                               &hash);
    while(opt_info != NULL) {
        if(opt_info->help_fn != NULL) {
            opt_info->help_fn(opt_info->option);
        } else {
            printf(" %s %s\n"
                   "%s\n",
                   opt_info->option,
                   iw_cmdline_print_type(opt_info->opt->type),
                   opt_info->help);
        }
        opt_info = (iw_opt_info *)iw_htable_get_next(&s_options, &hash);
    }
}

// --------------------------------------------------------------------------

bool iw_cmdline_add_option(
    const char *option,
    const char *help,
    bool mandatory,
    IW_OPT_TYPE type,
    iw_opt *opt,
    IW_OPT_PROC_FN proc_fn,
    IW_OPT_HELP_FN help_fn)
{
    static bool s_initialized = false;

    if(option == NULL ||
       (help == NULL && help_fn == NULL) ||
       opt == NULL)
    {
        // Required parameter is not defined.
        return false;
    }

    if(!s_initialized) {
        if(!iw_htable_init(&s_options, 100, false, NULL)) {
            return false;
        }
        s_initialized = true;
    }

    iw_opt_info *opt_info = (iw_opt_info *)calloc(1, sizeof(iw_opt_info));
    if(opt_info == NULL) {
        return false;
    }

    opt_info->option    = strdup(option);
    opt_info->help      = help != NULL ? strdup(help) : NULL;
    opt_info->mandatory = mandatory;
    memset(opt, 0, sizeof(*opt));
    opt_info->opt       = opt;
    opt_info->opt->type = type;
    if(type == IW_OPT_CALLBACK) {
        opt_info->proc_fn = proc_fn;
    }
    opt_info->help_fn = help_fn;
    iw_htable_insert(&s_options,
            strlen(opt_info->option),
            opt_info->option,
            opt_info);

    return true;
}

// --------------------------------------------------------------------------
