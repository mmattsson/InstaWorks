// --------------------------------------------------------------------------
///
/// @file test_opts.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_cmdline.h"
#include "iw_cmdline_int.h"
#include "iw_util.h"
#include "iw_cfg.h"

#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------

/// The maximum number of arguments in the command-line for tests.
#define MAX_ARGC    10

// --------------------------------------------------------------------------

enum {
    OPT_Y = 0,
    OPT_Z,
    OPT_1,
    OPT_2,
    OPT_X
};

// --------------------------------------------------------------------------

static char *argv[][MAX_ARGC] = {
    // Test character options
    { "-y" },
    { "-y", "-Q" },
    { "-y", "Q" },
    { "-y", "4" },
    { "-y", "abc" },
    { "-z", "1" },

    // Test number options
    { "-z" },
    { "-z", "-Q" },
    { "-z", "Q" },
    { "-z", "123" },
    { "-z", "0x123" },
    { "-z", "123abc" },
    { "-z", "0x123abc" },
    { "-z", "0x1aq" },
    { "-z", "qwerty" },

    // Test string options
    { "--opt1" },
    { "--opt1", "-Q" },
    { "--opt1", "123" },
    { "--opt1", "!@#$%" },

    // Test callback options
    { "--opt2" },
    { "--opt2", "-Q" },
    { "--opt2", "123" },
    { "--opt2", "!@#$%" },
    { "--opt2", "a:b:c:1:2:3" },
    { "--opt2", "123", "456" },
    { "--opt2", "123", "456", "789" },

    // Test pre-defined options as well as unknown options
    { "-f", "-l", "0x3", "--unknown", "abc", "def" },
    { "-f", "-l", "0x3", "abc", "def" },

    // Test mandatory options
    { "-f" },
    { "-x" }
};

// --------------------------------------------------------------------------

static iw_opt s_opts[5];
static int s_num = 0;
static char *s_arg[3];

// --------------------------------------------------------------------------

static bool proc_opt(int *cnt, int argc, char **argv, iw_opt *opt) {
    s_num = argc;
    for(*cnt=0;*cnt < s_num && *cnt < 3;(*cnt)++) {
        s_arg[*cnt] = argv[*cnt];
    }
    for(;*cnt < 3;cnt++) {
        s_arg[*cnt] = NULL;
    }
    return true;
}

// --------------------------------------------------------------------------

static IW_CMD_OPT_RET test_line(char **argv) {
    int cnt = 0;
    int proc = 0;
    char buff[256];

    int offset = snprintf(buff, sizeof(buff), "Parsing command-line: \"");
    for(cnt=0;cnt < MAX_ARGC;cnt++) {
        if(argv[cnt] == '\0') {
            break;
        }
        if(cnt > 0) {
            offset += snprintf(buff+offset, sizeof(buff)-offset, " ");
        }
        offset += snprintf(buff+offset, sizeof(buff)-offset, "%s", argv[cnt]);
    }
    test_display("%s\"", buff);

    return iw_cmdline_process(&proc, cnt, argv);
}

// --------------------------------------------------------------------------

void test_opts(test_result *result) {
    IW_CMD_OPT_RET retval;
    int cnt = 0;

    iw_cmdline_init();

    // Add some basic options, char, num, string, and callback.
    iw_cmdline_add_option("-y", "Option Y", false, IW_OPT_CHAR, &(s_opts[OPT_Y]), NULL, NULL);
    iw_cmdline_add_option("-z", "Option Z", false, IW_OPT_NUM, &(s_opts[OPT_Z]), NULL, NULL);
    iw_cmdline_add_option("--opt1", "Option 1", false, IW_OPT_STR, &(s_opts[OPT_1]), NULL, NULL);
    iw_cmdline_add_option("--opt2", "Option 2", false, IW_OPT_CALLBACK, &(s_opts[OPT_2]), proc_opt, NULL);

    // Test character option
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Fail to parse missing option?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Fail to parse missing option?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_opts[OPT_Y].is_set && s_opts[OPT_Y].val.ch == 'Q', "Successfully parses 'Q'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_opts[OPT_Y].is_set && s_opts[OPT_Y].val.ch == '4', "Successfully parses '4'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Fail to parse invalid option type?");
    retval = test_line(argv[cnt++]);
    test(result, s_opts[OPT_Y].is_set == false, "Make sure -y 'set' flag is cleared");

    // Test number option
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Fail to parse missing option?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Fail to parse missing option?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Fail to parse incorrect type?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_opts[OPT_Z].is_set && s_opts[OPT_Z].val.num == 123, "Successfully parses '123'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_opts[OPT_Z].is_set && s_opts[OPT_Z].val.num == 0x123, "Successfully parses '0x123'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Fail to parse '123abc'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_opts[OPT_Z].is_set && s_opts[OPT_Z].val.num == 0x123abc, "Successfully parse '0x123abc'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Fail to parse '0x1aq'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Fail to parse 'qwerty'?");

    // Test the string option
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Fail to parse missing option?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_opts[OPT_1].is_set && strcmp(s_opts[OPT_1].val.str, "-Q")==0, "Successfully parses '-Q'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_opts[OPT_1].is_set && strcmp(s_opts[OPT_1].val.str, "123")==0, "Successfully parses '-Q'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_opts[OPT_1].is_set && strcmp(s_opts[OPT_1].val.str, "!@#$%")==0, "Successfully parses '-Q'?");

    // Test callback options, callback will set a few variables as a result of test
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_num == 0, "Successfully parses '--opt2'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_num == 1 && strcmp(s_arg[0], "-Q") == 0, "Successfully parses '--opt2 -Q'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_num == 1 && strcmp(s_arg[0], "123") == 0, "Successfully parses '--opt2 123'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_num == 1 && strcmp(s_arg[0], "!@#$%") == 0, "Successfully parses '--opt2 !@#$%'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_num == 1 && strcmp(s_arg[0], "a:b:c:1:2:3") == 0, "Successfully parses '--op2 a:b:c:1:2:3'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_num == 2 && strcmp(s_arg[0], "123") == 0 && strcmp(s_arg[1], "456") == 0, "Successfully parses '--opt2 123 456'?");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK && s_num == 3 && strcmp(s_arg[0], "123") == 0 && strcmp(s_arg[1], "456") == 0 && strcmp(s_arg[2], "789") == 0, "Successfully parses '--opt2 123 456 789'?");

    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_UNKNOWN, "Unknown option");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK, "Parses OK");

    // Add a mandatory option
    iw_cmdline_add_option("-x", "Option X", true, IW_OPT_FLAG, &(s_opts[0]), NULL, NULL);

    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_INVALID, "Missing mandatory parameter");
    retval = test_line(argv[cnt++]);
    test(result, retval == IW_CMD_OPT_OK, "Parses OK");
}

// --------------------------------------------------------------------------
