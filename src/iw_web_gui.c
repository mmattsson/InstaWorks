// --------------------------------------------------------------------------
///
/// @file iw_web_sr.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
//
// Variables and structures
//
// --------------------------------------------------------------------------

static iw_web_srv m s_web_srv;

// --------------------------------------------------------------------------
//
// Helper functions
//
// --------------------------------------------------------------------------

/// @brief Create a response and send it.
/// @param out The file stream to write the response to.
/// @return True if the response was successfully written.
static bool iw_web_gui_construct_response(FILE *out) {
    char *content =
        "<html><head><title>Response2</title></head>\n"
        "<body>\n"
        "<h1>Response</h1>\n"
        "<form action='/submit'>\n"
        "Name: <input type='text' name='name'></input>\n"
        "<input type='submit' value='Submit'></input>\n"
        "</form>\n"
        "</body>\n"
        "</html>";
    fprintf(out, "HTTP/1.1 200 Ok\r\n"
                    "Content-Length: %ld\r\n"
                    "\r\n"
                    "%s\r\n",
                    (long int)strlen(content),
                    content);

    return true;
}

// --------------------------------------------------------------------------

bool iw_web_gui_init() {
}

// --------------------------------------------------------------------------
