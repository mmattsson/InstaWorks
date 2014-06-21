// --------------------------------------------------------------------------
///
/// @file iw_cmd_srv.h
///
/// Copyright (C) Mattias Mattsson - 2014
///
// --------------------------------------------------------------------------

#ifndef _IW_CMD_SRV_H_
#define _IW_CMD_SRV_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_main.h"

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Called to create a command server to process client requests.
/// @param main_fn The main function callback or NULL if none.
/// @param port The port number to use to serve client requests.
/// @param argc The number of arguments being passed.
/// @param argv The arguments being passed.
extern bool iw_cmd_srv(
    IW_MAIN_FN main_fn,
    unsigned short port,
    int argc,
    char **argv);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_CMD_SRV_H_

// --------------------------------------------------------------------------
