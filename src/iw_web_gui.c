// --------------------------------------------------------------------------
///
/// @file iw_web_gui.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_web_gui.h"

#include "iw_cfg.h"
#include "iw_log.h"
#include "iw_util.h"

#include <string.h>

// --------------------------------------------------------------------------
//
// Variables and structures
//
// --------------------------------------------------------------------------

static iw_web_srv *s_web_gui;  /// The web GUI server instance.

// --------------------------------------------------------------------------
//
// Helper functions
//
// --------------------------------------------------------------------------

static bool iw_web_gui_construct_menu(iw_web_req *req, FILE *out) {
    int cnt = 0;
    char *menu[] = {
        "About", "Run-time", "Configuration"
    };

    fprintf(out, "<ul id='menu'>\n");
    for(cnt=0;cnt < IW_ARR_LEN(menu);cnt++) {
        fprintf(out,
            "  <li><a href='#'>%s</a></li>\n", menu[cnt]);
    }
    fprintf(out, "</ul>\n");

    return true;
}

// --------------------------------------------------------------------------

/// @brief Create a response and send it.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully written.
static bool iw_web_gui_construct_response(iw_web_req *req, FILE *out) {
    LOG(IW_LOG_GUI, "Received request for \"%.*s\"",
        req->uri.len, req->buff + req->uri.start);

    // Print HTML header
    fprintf(out,
        "<!doctype html>\n"
        "<html>\n"
        "<head>\n"
        "  <title>%s</title>\n"
        "  <link rel='stylesheet' href='style.css'>\n"
        "</head>\n"
        "\n",
        iw_val_store_get_string(&iw_cfg, IW_CFG_PRG_NAME));

    // Start of body
    fprintf(out, "<body>\n");

    iw_web_gui_construct_menu(req, out);

    // The main header
    fprintf(out,
        "<h1>Response</h1>\n");

    // Input form
    fprintf(out,
        "<form action='/submit'>\n"
        "Name: <input type='text' name='name'>\n"
        "<input type='submit' value='Submit'>\n"
        "</form>\n");

    // End of body
    fprintf(out,
        "</body>\n");

    // End of HTML
    fprintf(out, "</html>\n");

    return true;
}

// --------------------------------------------------------------------------

bool iw_web_gui_init(
    iw_ip *address,
    unsigned short port)
{
    s_web_gui = iw_web_srv_init(address, port, iw_web_gui_construct_response);
    return s_web_gui != NULL;
}

// --------------------------------------------------------------------------
