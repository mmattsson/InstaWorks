// --------------------------------------------------------------------------
///
/// @file iw_web_gui.h
///
/// A web based GUI for the InstaWorks framework. Used to access and display
/// run-time statistics as well as configuration.
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#ifndef _IW_WEB_GUI_H_
#define _IW_WEB_GUI_H_
#ifdef _cplusplus
extern "C" {
#endif

#include "iw_buff.h"
#include "iw_ip.h"
#include "iw_list.h"
#include "iw_main.h"
#include "iw_web_srv.h"

#include <stdbool.h>

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

/// @brief Called to create a web server to process client requests.
/// @param address The address to bind to or NULL for local host.
/// @param port The port number to use to serve client requests. If the port
///        is set to zero, the default port of 8080 will be used.
extern bool iw_web_gui_init(
    iw_ip *address,
    unsigned short port);

// --------------------------------------------------------------------------

/// @brief Called to terminate the web GUI.
extern void iw_web_gui_exit();

// --------------------------------------------------------------------------

#ifdef _cplusplus
}
#endif
#endif // _IW_WEB_GUI_H_

// --------------------------------------------------------------------------
