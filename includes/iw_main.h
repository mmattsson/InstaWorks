// --------------------------------------------------------------------------
///
/// @file iw_main.h
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_MAIN_H_
#define _IW_MAIN_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Typedefs
//
// --------------------------------------------------------------------------

/// The iw_main() function exit code.
typedef enum {
    /// An invalid command line parameter was passed to the program.
    IW_MAIN_SRV_INVALID_PARAMETER,

    /// The server failed to start.
    IW_MAIN_SRV_FAILED,

    // The server was successfully started.
    IW_MAIN_SRV_OK,

    /// No options or arguments were passed to the program.
    IW_MAIN_SRV_NO_OPTS,

    /// The client failed to start.
    IW_MAIN_CLNT_FAILED,

    /// The client was successfully started.
    IW_MAIN_CLNT_OK
} IW_MAIN_EXIT;

// --------------------------------------------------------------------------
//
// Callbacks
//
// --------------------------------------------------------------------------

/// @brief The main function callback typedef.
/// A function pointer definition for the main function callback to be called
/// after the framework has parsed its parameters.
/// @param argc The number of arguments being passed in.
/// @param argv The arguments being passed in.
/// @return True if the arguments were successfully parsed.
typedef bool (*IW_MAIN_FN)(int argc, char **argv);

/// @brief The main loop termination notification callback.
/// If the framework receives a termination request (such as a 'quit' command
/// on the client command channel, the program will get a notification through
/// this callback. The program should terminate any processing and free
/// any resources in use. Termination can be done by setting a flag to
/// indicate that the main thread should exit its loop, or shut down a
/// server socket. After local cleanup is done, iw_exit() and then exit()
/// should be called.
typedef void (*IW_TERM_FN)();

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Initialize the InstaWorks module.
extern void iw_init();

// --------------------------------------------------------------------------

/// @brief Terminate the InstaWorks module.
/// Releases any allocated memory to allow external tools to check for memory
/// leaks.
extern void iw_exit();

// --------------------------------------------------------------------------

/// @brief The main entry point.
/// Should be called from main(). If the main program wants to handle
/// command line options before calling the InstaWorks framework it can do so.
/// In this case, parse_options should be set to false, otherwise, set it to
/// true for iw_main() to do the parsing of options. If the client parses the
/// options, it is up to the client to set the iw_stg.iw_foreground flag to
/// true when the program should be run in server mode.
/// Note that argc and argv should still be passed in in order for the client
/// control functionality to work.
/// @param main_fn The main function to call back after the framework is
///        initialized or NULL if the main thread is not needed.
/// @param term_fn The termination notification callback.
/// @param parse_options True if options should be parsed.
/// @param argc The number of arguments being passed.
/// @param argv The arguments being passed.
/// @return The exit code for the function call.
extern IW_MAIN_EXIT iw_main(
    IW_MAIN_FN main_fn,
    IW_TERM_FN term_fn,
    bool parse_options,
    int argc,
    char **argv);

// --------------------------------------------------------------------------

/// @brief A main loop to prevent the program from exiting.
/// This should be called if the main thread is not used for processing to
/// prevent the main thread from exiting.
extern void iw_main_loop();

// --------------------------------------------------------------------------

/// @brief Terminate the main loop and exit the program.
/// This should be called if the program chose to call the iw_main_loop()
/// function call. If the program is using the main thread for its own
/// processing, it should listen to the main termination notifiation
/// callback rather than calling this function.
extern void iw_main_loop_exit();

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_MAIN_H_

// --------------------------------------------------------------------------
