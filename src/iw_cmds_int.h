// --------------------------------------------------------------------------
///
/// @file iw_cmds_int.h
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_CMDS_INT_H_
#define _IW_CMDS_INT_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_cmds.h"

#include <stdio.h>

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Called to process a client request.
/// @param info The command info structure for parsing the request.
/// @param out The file stream to write the response to.
/// @return True if the command was successfully processed.
extern bool iw_cmds_process(iw_cmd_parse_info *info, FILE *out);

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_CMDS_INT_H_

// --------------------------------------------------------------------------
