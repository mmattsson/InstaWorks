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

/// @brief Create a menu and display it.
/// @param req The request that was made.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully written.
static bool iw_web_gui_construct_menu(iw_web_req *req, FILE *out) {
    int cnt = 0;
    char *menu[] = {
        "About", "Run-time", "Configuration"
    };

    fprintf(out, "<ul id='menu'>\n");
    for(cnt=0;cnt < IW_ARR_LEN(menu);cnt++) {
        fprintf(out,
            "  <li><a href='%s'>%s</a></li>\n", menu[cnt], menu[cnt]);
    }
    fprintf(out, "</ul>\n");

    return true;
}

// --------------------------------------------------------------------------

/// @brief Create a style-sheet and send it.
/// @param req The request that was made.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully written.
static bool iw_web_gui_construct_style_sheet(iw_web_req *req, FILE *out) {
    fprintf(out,
        "body {\n"
        "  background-color: #E8E8E8;\n"
        "}\n"
        "#menu {\n"
        "  min-width: 700px;\n"
        "  height: 70px;\n"
        "  line-height: 70px;\n"
        "  font-size: 36px;\n"
        "  font-family: Arial, sans-serif;\n"
        "  font-weight: bold;\n"
        "  text-align: center;\n"
        "  background-color: #5C5C5C;\n"
        "  border-radius: 8px;\n"
        "}\n"
        "#menu ul {\n"
        "  height: auto;\n"
        "  padding: 8px 0px;\n"
        "  margin: 0px;\n"
        "}\n"
        "#menu li {\n"
        "  display: inline;\n"
        "  padding: 10px;\n"
        "}\n"
        "#menu a {\n"
        "  text-decoration: none;\n"
        "  color: #6666C4;\n"
        "  padding: 8px 8px 8px 8px;\n"
        "}\n"
        "#menu a:hover {\n"
        "  color: #A9A9F5;\n"
        "  background-color: #5C5C5C;\n"
        "}\n"
        "\n");

    return true;
}

// --------------------------------------------------------------------------

/// @brief Create a web page and send it.
/// @param req The request that was made.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully written.
static bool iw_web_gui_construct_web_page(iw_web_req *req, FILE *out) {
    // Print HTML header
    fprintf(out,
        "<!doctype html>\n"
        "<html>\n"
        "<head>\n"
        "  <title>%s</title>\n"
        "  <link rel='stylesheet' href='style.css'>\n"
        "  <link href='data:image/x-icon;base64,iVBORw0KGgoAAAANSUhEUgAAAB"
        "AAAAAQEAYAAAB""PYyMiAAAABmJLR0T///////8JWPfcAAAACXBIWXMAAABIAAAAS"
        "ABGyWs+AAAAF0lEQVRIx2NgGAWjYBSMglEwCkbBSAcACBAAAeaR9cIAAAAASUVORK"
        "5CYII=' rel='icon' type='image/x-icon' />\n"
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

/// @brief Create a response and send it.
/// @param req The request that was made.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully written.
static bool iw_web_gui_construct_response(iw_web_req *req, FILE *out) {
    LOG(IW_LOG_GUI, "Received request for \"%.*s\"",
        req->uri.len, req->buff + req->uri.start);
    if(iw_parse_cmp("/style.css", req->buff, &req->uri)) {
        LOG(IW_LOG_GUI, "Sending style sheet");
        iw_web_gui_construct_style_sheet(req, out);
    } else if(iw_parse_cmp("/", req->buff, &req->uri)) {
        LOG(IW_LOG_GUI, "Sending web page");
        iw_web_gui_construct_web_page(req, out);
    } else {
        return false;
    }


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
