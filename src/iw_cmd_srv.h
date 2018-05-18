// --------------------------------------------------------------------------
///
/// @file iw_cmd_srv.h
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
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
/// @param port The port number to use to serve client requests.
extern bool iw_cmd_srv(unsigned short port);

// --------------------------------------------------------------------------

/// @brief Terminate the command server.
extern void iw_cmd_srv_exit();

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_CMD_SRV_H_

// --------------------------------------------------------------------------
