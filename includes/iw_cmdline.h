// --------------------------------------------------------------------------
///
/// @file iw_cmdline.h
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_CMDLINE_H_
#define _IW_CMDLINE_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Data structures
//
// --------------------------------------------------------------------------

/// The type of the command-line option.
typedef enum {
    IW_OPT_FLAG,
    IW_OPT_CHAR,
    IW_OPT_NUM,
    IW_OPT_STR,
    IW_OPT_CALLBACK
} IW_OPT_TYPE;

// --------------------------------------------------------------------------

// TODO: Merge iw_opt and iw_val structures.

/// The value that the option provides
typedef struct _iw_opt {
    IW_OPT_TYPE type;           ///< The type of option.
    bool is_set;                ///< True if the option was set.

    /// The option value.
    union {
        bool           flag;    ///< A flag.
        long long int  num;     ///< A number.
        char          *str;     ///< A string.
        char           ch;      ///< A character.
    } val;
} iw_opt;

// --------------------------------------------------------------------------

/// @brief A command-line option processing function.
/// Passed in to the iw_cmdline_add_option() function when adding an option
/// that is not suited for one of the pre-defined types. This function will
/// be called when the option is processed and allows the caller to set the
/// value.
/// When the callback function is called, cnt will be zero, and argc will be
/// set to the number of arguments in the argument array. The callee will
/// have to update cnt to be set to the next argument that should be
/// processed.
/// If the option cannot be parsed, the callback should return false. This
/// will interrupt any further parsing of options and exit the program.
/// @param cnt [out] The start of the remaining arguments.
/// @param argc The number of arguments.
/// @param argv The argument array.
/// @param opt  The option value structure.
/// @return True if the option was successfully parsed.
typedef bool (*IW_OPT_PROC_FN)(int *cnt, int argc, char **argv, iw_opt *opt);

// --------------------------------------------------------------------------

/// @brief A help function callback used to display help for an option.
/// @param option The option to display help for.
/// @return True if the help was successfully displayed.
typedef bool (*IW_OPT_HELP_FN)(const char *option);

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Initialize the module.
extern void iw_cmdline_init();

// --------------------------------------------------------------------------

/// @brief Display help for all the defined command line options.
extern void iw_cmdline_print_help();

// --------------------------------------------------------------------------

/// @brief Add a command-line option.
/// Adds a command line option to the set of options that the program can
/// accept. Can be a single character switch (-q) or a string (--silent).
/// A requirement is that any option must begin with one dash.
///
/// The caller has to pass a pointer to an option value to the function.
/// If the command line sets the given option, the option value will
/// receive the value. This allows the caller to easily access all
/// option values after the command line is parsed.
///
/// @param name The command-line option to add.
/// @param help The help text for the command.
/// @param mandatory True if the option is mandatory.
/// @param type The type of the option value.
/// @param option A pointer to the option value.
/// @param proc_fn The command line processing function. Called to process
///        a command line option if none of the pre-defined types is suitable.
/// @param help_fn A help function callback. Called, if specified, to print
///        help for the command line option.
/// @return True if the option was successfully added.
extern bool iw_cmdline_add_option(
        const char *name,
        const char *help,
        bool mandatory,
        IW_OPT_TYPE type,
        iw_opt *option,
        IW_OPT_PROC_FN proc_fn,
        IW_OPT_HELP_FN help_fn);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_CMDLINE_H_

// --------------------------------------------------------------------------
