// --------------------------------------------------------------------------
///
/// @file iw_main.c
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_main.h"

#include "iw_cmdline.h"
#include "iw_cmdline_int.h"
#include "iw_cmds.h"
#include "iw_cmds_int.h"
#include "iw_cmd_clnt.h"
#include "iw_cmd_srv.h"
#include "iw_health_int.h"
#include "iw_log_int.h"
#include "iw_memory_int.h"
#include "iw_mutex_int.h"
#include "iw_settings.h"
#include "iw_syslog.h"
#include "iw_thread_int.h"
#include "iw_util.h"

#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static bool s_initialized = false;

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_init() {
    if(!s_initialized) {
        iw_log_init();
        iw_memory_init();
        iw_mutex_init();
        iw_syslog_reinit(1000);
        iw_cmd_init();
        iw_thread_init();
        iw_health_start();
        s_initialized = true;
    }
}

// --------------------------------------------------------------------------

void iw_exit() {
    iw_syslog_exit();
    iw_mutex_exit();
    iw_memory_exit();
    s_initialized = false;
}

// --------------------------------------------------------------------------

IW_MAIN_EXIT iw_main(
    IW_MAIN_FN main_fn,
    bool parse_options,
    int argc,
    char **argv)
{
    int cnt = 0;

    // Set the program name to the content of argv[0] for future use.
    if(argc > 0 && argv[0] != NULL) {
        const char *prg_name = strrchr(argv[0], '/');
        if(prg_name != NULL) {
            iw_stg.iw_prg_name = prg_name + 1;
        }
    }

    if(parse_options) {
        // Try to process the command line to determine how we should run.
        iw_cmdline_init();
        IW_CMD_OPT_RET ret = iw_cmdline_process(&cnt, argc-1, argv+1);
        if(ret == IW_CMD_OPT_INVALID || ret == IW_CMD_OPT_UNKNOWN) {
            // An invalid option was given. Return to the caller so
            // the caller can decide what to do.
            return IW_MAIN_SRV_INVALID_PARAMETER;
        } else if(argc == 1) {
            // Checking this after the call to iw_cmdline_process() to
            // add the pre-defined options to the help output.
            return IW_MAIN_SRV_NO_OPTS;
        }
    }

    // Decide whether we should run as a server or not. If the
    // 'foreground' option is given then we start the server. Otherwise
    // we start the client.
    if(iw_stg.iw_foreground) {
        iw_init();
        if(iw_stg.iw_log_level != 0) {
            iw_log_set_level("stdout", iw_stg.iw_log_level);
        }
        iw_thread_register_main();
        bool retval = iw_cmd_srv(main_fn, iw_stg.iw_cmd_port, argc-cnt-1, argv+cnt+1);
        iw_exit();
        return retval ? IW_MAIN_SRV_OK : IW_MAIN_SRV_FAILED;
    } else {
        return iw_cmd_clnt(iw_stg.iw_cmd_port, argc-cnt-1, argv+cnt+1) ?
                IW_MAIN_CLNT_OK : IW_MAIN_CLNT_FAILED;
    }
}

// --------------------------------------------------------------------------

void iw_main_loop() {
    // Go into an infinite loop here.
    while(true) {
        sleep(10);
    }
}

// --------------------------------------------------------------------------
