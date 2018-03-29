// --------------------------------------------------------------------------
///
/// @file iw_cmd_clnt.h
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_CMD_CLNT_H_
#define _IW_CMD_CLNT_H_
#ifdef _cplusplus
extern "C" {
#endif

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Called to issue a client request to the server.
/// @param port The port number to use to serve client requests.
/// @param argc The number of arguments being passed.
/// @param argv The arguments being passed.
extern bool iw_cmd_clnt(unsigned short port, int argc, char **argv);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_CMD_CLNT_H_

// --------------------------------------------------------------------------
