// --------------------------------------------------------------------------
///
/// @file iw_main.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_main.h"

#include "iw_cfg.h"
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
#include "iw_syslog.h"
#include "iw_thread_int.h"
#include "iw_util.h"
#include "iw_web_gui.h"

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
        // The order of initialization is important.

        // Configuration should be initialized by the caller before this
        // point so that configuration settings can be set by the program.
        // We read the following variables:
        int *log_level = iw_val_store_get_number(&iw_cfg,
                                                 IW_CFG_LOGLEVEL);
        int *websrv_enable = iw_val_store_get_number(&iw_cfg,
                                                     IW_CFG_WEBGUI_ENABLE);

        // We start the log module first so that we can log all startup.
        if(log_level != NULL && *log_level != 0) {
            iw_log_set_level("stdout", *log_level);
        }

        // We must initialize the thread module and register the main thread.
        // This is so that all other threads that are created can register
        // their thread-info.
        iw_thread_init();
        iw_thread_register_main();

        // After that we initialize the mutex and memory modules.
        iw_mutex_init();
        iw_memory_init();

        // Then syslog, command server, and health check.
        iw_syslog_reinit(1000);
        iw_cmd_init();
        iw_health_start();

        // Finally set up the web server if enabled.
        if(websrv_enable != NULL && *websrv_enable) {
            iw_web_gui_init(NULL, 0);
        }

        s_initialized = true;
    }
}

// --------------------------------------------------------------------------

void iw_exit() {
    iw_web_gui_exit();
    iw_syslog_exit();
    iw_mutex_exit();
    iw_memory_exit();
    iw_cfg_exit();

    // Wait a while to allow threads to exit. We should really do an ordered
    // shutdown and wait for a semaphore but we also shouldn't block
    // indefinitely just because a sub-system doesn't shut down properly.
    usleep(100000);

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
            iw_val_store_set_string(&iw_cfg, IW_CFG_PRG_NAME, prg_name + 1,
                                    NULL, 0);
        }
    }

    // Init config first of all
    iw_cfg_init();

    // Must set log level before command line parsing. If parsing fails
    // we must print out program usage help and that includes log levels.
    iw_log_init();

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
    int *foreground = iw_val_store_get_number(&iw_cfg, IW_CFG_FOREGROUND);
    int *daemonize  = iw_val_store_get_number(&iw_cfg, IW_CFG_DAEMONIZE);
    int *cmd_port   = iw_val_store_get_number(&iw_cfg, IW_CFG_CMD_PORT);
    if((foreground != NULL && *foreground) || (daemonize != NULL && *daemonize)) {
        if(*daemonize) {
            if(daemon(0, 0) != 0) {
                return IW_MAIN_SRV_FAILED;
            }
        }
        iw_init();
        bool retval = iw_cmd_srv(main_fn,
                                 cmd_port != NULL ? *cmd_port : 0,
                                 argc-cnt-1, argv+cnt+1);
        iw_exit();
        return retval ? IW_MAIN_SRV_OK : IW_MAIN_SRV_FAILED;
    } else {
        return iw_cmd_clnt(cmd_port != NULL ? *cmd_port : 0,
                           argc-cnt-1, argv+cnt+1) ?
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
