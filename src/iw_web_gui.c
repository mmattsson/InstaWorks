// --------------------------------------------------------------------------
///
/// @file iw_web_gui.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_web_gui.h"

#include "iw_cfg.h"
#include "iw_ip.h"
#include "iw_log.h"
#include "iw_util.h"

#include <string.h>

// --------------------------------------------------------------------------
//
// Variables and structures
//
// --------------------------------------------------------------------------

/// The web GUI server instance.
static iw_web_srv *s_web_gui = NULL;

/// The menu structure.
static char *s_menu[] = {
    "/About", "/Run-time", "/Configuration"
};

/// The page to display
typedef enum {
    PG_NONE    = -1,
    PG_ABOUT   = 0,
    PG_RUNTIME = 1,
    PG_CONFIG  = 2
} PAGE;

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

    fprintf(out, "<ul id='menu'>\n");
    for(cnt=0;cnt < IW_ARR_LEN(s_menu);cnt++) {
        fprintf(out,
            "  <li><a href='%s'>%s</a></li>\n", s_menu[cnt]+1, s_menu[cnt]+1);
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
    char *file = iw_val_store_get_string(&iw_cfg, IW_CFG_WEBGUI_CSS_FILE);
    FILE *fptr = NULL;
    if(file != NULL && strlen(file) > 0) {
        fptr = fopen(file, "r");
    }
    if(fptr != NULL) {
        char buffer[1024];
        while(!feof(fptr)) {
            if(fgets(buffer, sizeof(buffer), fptr) == NULL) {
                break;
            }
            fputs(buffer, out);
        }
    } else {
        fprintf(out,
            "body {\n"
            "  background-color: #E8E8E8;\n"
            "  font-family: Arial, sans-serif;\n"
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
            "  color: #4FDE1F;\n"
            "  padding: 8px 8px 8px 8px;\n"
            "}\n"
            "#menu a:hover {\n"
            "  color: #57FF1F;\n"
            "  background-color: #5C5C5C;\n"
            "}\n"
            ".data {\n"
            "  border-collapse: collapse;\n"
            "  width: 80%%;\n"
            "}\n"
            ".data td, th {\n"
            "  padding: 10px;\n"
            "  border-bottom: solid 1px black;\n"
            "}\n"
            ".data tbody tr:nth-of-type(even) {\n"
            "  background-color: rgba(0,0,0,.05);\n"
            "}\n"
            "\n");
    }

    return true;
}

// --------------------------------------------------------------------------

/// @brief Create the about page.
/// @param req The request that was made.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully created.
static bool iw_web_gui_construct_about_page(iw_web_req *req, FILE *out) {
    char *prg = iw_val_store_get_string(&iw_cfg, IW_CFG_PRG_NAME);
    char *about = iw_val_store_get_string(&iw_cfg, IW_CFG_PRG_ABOUT);

    // The main header
    fprintf(out, "<h1>About '%s'</h1>\n", prg);

    if(about == NULL) {
        fprintf(out,
            "<p>The program '%s' uses a debug framework called "INSTAWORKS" which\n"
            "provides extensive debug support for this program.</p>\n"
            "\n", prg);

        fprintf(out,
            "<p>"INSTAWORKS" is a support library for adding a debug framework to programs or\n"
            "daemons. A debug framework is usually not the first thing being considered\n"
            "when creating a new program. When creating a new program, the first priority\n"
            "is to quickly get a proof of concept working. This may be due to general\n"
            "excitement of trying out something new, or because a dead-line is imposed\n"
            "by the manager of the project. Once the proof of concept is done, the next\n"
            "priority is usually to extend the functionality to a usable first version.\n"
            "Again, dead-lines have to be met.</p>\n"
            "\n"
            "<p>Because of this, it isn't until after the first version has shipped\n"
            "that the matter of debugging the program is considered. At this point it\n"
            "may be hard to graft a debug framework onto the program in question. Also,\n"
            "since there are always more features to add, there is never a good time to\n"
            "take the time out of the schedule to add the debug facilities.</p>\n"
            "\n"
            "<p>Therefore, "INSTAWORKS" was created to provide an instant debug framework\n"
            "support library that can be used when creating new programs. By simply\n"
            "linking "INSTAWORKS" and use the provided API, a number of services are\n"
            "provided that helps provide debug facilities to any new program with a\n"
            "minimal amount of time needed. The time savings from not having to create\n"
            "debug facilities can be spent on adding more features to the new program\n"
            "instead.</p>\n"
        );
    } else {
        fprintf(out, "<p>%s</p>\n", about);
    }

    return true;
}

// --------------------------------------------------------------------------

/// @brief Assign values set from the config page.
/// @param req The request that was made.
/// @param out The file stream to write the response to.
/// @return True if the values were successfully set.
static bool iw_web_gui_assign_config_values(iw_web_req *req, FILE *out) {
    char buff[256];
    iw_web_req_parameter *param = iw_web_req_get_parameter(req, NULL);
    while(param != NULL) {
        if(!iw_val_store_set_existing_value(&iw_cfg, param->name, param->value,
                                        buff, sizeof(buff)))
        {
            fprintf(out, "<p>Error: %s</p>", buff);
        }
        param = iw_web_req_get_next_parameter(req, NULL, param);
    }
    return true;
}

// --------------------------------------------------------------------------

/// @brief Create the run-time page.
/// @param req The request that was made.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully created.
static bool iw_web_gui_construct_config_page(iw_web_req *req, FILE *out) {
    fprintf(out, "<h1>Configuration Settings</h1>\n");

    unsigned long token;
    fprintf(out, "<form method='post'>\n");
    fprintf(out, "<table class='data'>\n");
    fprintf(out, "<tr><th>Name</th><th>Value</th></tr>\n");
    iw_val *value = iw_val_store_get_first(&iw_cfg, &token);
    while(value != NULL) {
        char value_buff[128];
        char output_buff[128];
        iw_val_to_str(value, value_buff, sizeof(value_buff));
        iw_web_req_sanitize(value_buff, output_buff, sizeof(output_buff));
        fprintf(out,
            "<tr>\n"
            "  <td>%s</td>\n"
            "  <td><input type='text' name='%s' value='%s'></td>\n"
            "</tr>\n",
            value->name, value->name, output_buff);
        value = iw_val_store_get_next(&iw_cfg, &token);
    }
    fprintf(out, "</table>\n");
    fprintf(out, "<input type='submit' name='Apply'>\n");
    fprintf(out, "</form>\n");

    return true;
}

// --------------------------------------------------------------------------

/// @brief Create the configuration page.
/// @param req The request that was made.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully created.
static bool iw_web_gui_construct_runtime_page(iw_web_req *req, FILE *out) {
    fprintf(out, "<h1>Run-time Statistics</h1>\n");

    if(iw_cb.runtime != NULL) {
        iw_cb.runtime(out);
    }

    return true;
}

// --------------------------------------------------------------------------

/// @brief Create a web page.
/// @param req The request that was made.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully created.
static bool iw_web_gui_construct_web_page(iw_web_req *req, FILE *out) {
    char *prg = iw_val_store_get_string(&iw_cfg, IW_CFG_PRG_NAME);
    PAGE pg = PG_NONE;
    if(req->path.len > 0 && *(req->buff + req->path.start) == '/') {
        int cnt;
        for(cnt=0;cnt < IW_ARR_LEN(s_menu);cnt++) {
            if(iw_parse_cmp(s_menu[cnt], req->buff, &req->path)) {
                pg = cnt;
            }
        }
    }

    // Special case, no path means the default about page
    if(iw_parse_cmp("/", req->buff, &req->path)) {
        pg = PG_ABOUT;
    }
    if(pg == PG_NONE) {
        return false;
    }

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
        prg);

    // Start of body
    fprintf(out,
        "<body>\n"
        "<h1 style='text-align:center'>%s</h1>\n",
        prg);

    iw_web_gui_construct_menu(req, out);

    switch(pg) {
    case PG_ABOUT :
        iw_web_gui_construct_about_page(req, out);
        break;
    case PG_RUNTIME :
        iw_web_gui_construct_runtime_page(req, out);
        break;
    case PG_CONFIG :
        if(req->method == IW_WEB_METHOD_POST) {
            // A POST for the configuration page, let's set the assigned
            // values.
            iw_web_gui_assign_config_values(req, out);
        }
        iw_web_gui_construct_config_page(req, out);
        break;
    default :
        return false;
        break;
    }

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
        req->path.len, req->buff + req->path.start);
    if(iw_parse_cmp("/style.css", req->buff, &req->path)) {
        LOG(IW_LOG_GUI, "Sending style sheet");
        return iw_web_gui_construct_style_sheet(req, out);
    } else {
        LOG(IW_LOG_GUI, "Sending web page");
        return iw_web_gui_construct_web_page(req, out);
    }
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

void iw_web_gui_exit() {
    if(s_web_gui != NULL) {
        iw_web_srv_exit(s_web_gui);
    }
}

// --------------------------------------------------------------------------
