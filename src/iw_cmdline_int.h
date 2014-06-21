// --------------------------------------------------------------------------
///
/// @file iw_cmdline_int.h
///
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#ifndef _IW_CMDLINE_INT_H_
#define _IW_CMDLINE_INT_H_
#ifdef _cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Enumerating return values from the command-line processing function.
typedef enum {
    IW_CMD_OPT_NONE,    ///< No command-line options were found
    IW_CMD_OPT_UNKNOWN, ///< An unknown command-line option was encountered.
    IW_CMD_OPT_INVALID, ///< An invalid command-line option was encountered.
    IW_CMD_OPT_OK       ///< The options were processed successfully.
} IW_CMD_OPT_RET;

// --------------------------------------------------------------------------

/// @brief Check the pre-defined command-line options.
/// This function will check any pre-defined options that was set by the
/// command-line and apply them to the module settings in iw_settings.c
extern void iw_cmdline_check_opts();

// --------------------------------------------------------------------------

/// @brief Process the command-line options
/// @param processed [out] The number of arguments processed by this function.
/// @param argc The number of arguments passed to the program.
/// @param argv The argument array.
/// @return The result of the processing.
extern IW_CMD_OPT_RET iw_cmdline_process(int *processed, int argc, char **argv);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_CMDLINE_INT_H_

// --------------------------------------------------------------------------
