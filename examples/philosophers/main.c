// --------------------------------------------------------------------------
///
/// @file main.c
///
/// The dining philosophers example. Implements the dining philosophers
/// in an way that triggers occasional dead-locks. The framework
/// will allow the dead-locks to be debugged.
///
/// Copyright (c) 2014 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include <iw_cfg.h>
#include <iw_cmds.h>
#include <iw_log.h>
#include <iw_main.h>
#include <iw_memory.h>
#include <iw_mutex.h>
#include <iw_thread.h>
#include <iw_util.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// --------------------------------------------------------------------------
//
// Variables
//
// --------------------------------------------------------------------------

#define PHILO_LOG   8

static long long int num_philosophers = 5;

static bool do_correct = false;

static IW_MUTEX *s_mutexes = NULL;

// --------------------------------------------------------------------------
//
// Internal functions
//
// --------------------------------------------------------------------------

void philo_take_forks(int num, int left, int right) {
    int fork1;
    int fork2;

    if(do_correct) {
        // Avoid dead-locks, make sure each philosopher takes the shared
        // forks in the same order. First the even one, then the odd one.
        if((left % 2) == 0) {
            fork1 = s_mutexes[left];
            fork2 = s_mutexes[right];
        } else {
            fork1 = s_mutexes[right];
            fork2 = s_mutexes[left];
        }
    } else {
        // Allow dead-locks (to show the dead-lock detection feature).
        // Randomly take one fork, then the other. The two forks each
        // philosopher can take is the one with the same number (it's 'right')
        // and the one with one number lower (it's 'left').
        if(random() % 2) {
            fork1 = s_mutexes[left];
            fork2 = s_mutexes[right];
        } else {
            fork1 = s_mutexes[right];
            fork2 = s_mutexes[left];
        }
    }

    printf("Philosopher[%d] - Taking fork %d\n", num+1, fork1+1);
    LOG(PHILO_LOG, "Philosopher[%d] - Taking fork %d", num+1, fork1+1);
    iw_mutex_lock(fork1);
    sched_yield();
    usleep(random() % 100000);

    printf("Philosopher[%d] - Taking fork %d\n", num+1, fork2+1);
    LOG(PHILO_LOG, "Philosopher[%d] - Taking fork %d", num+1, fork2+1);
    iw_mutex_lock(fork2);
    sched_yield();
    printf("Philosopher[%d] - Got forks %d and %d\n",
            num+1, fork1+1, fork2+1);
    LOG(PHILO_LOG, "Philosopher[%d] - Got forks %d and %d",
            num+1, fork1+1, fork2+1);

    // Release forks
    sched_yield();
    usleep(random() % 10000);
    printf("Philosopher[%d] - Releasing forks %d and %d\n",
            num+1, fork1+1, fork2+1);
    LOG(PHILO_LOG, "Philosopher[%d] - Releasing forks %d and %d",
            num+1, fork1+1, fork2+1);

    iw_mutex_unlock(fork1);
    sched_yield();
    iw_mutex_unlock(fork2);
    sched_yield();
}

// --------------------------------------------------------------------------

/// @brief Thread creating entry point.
/// Called by each new created thread.
/// @param param The philosopher number of this instance.
void *philo_callback(void *param) {
    intptr_t num = (intptr_t)param;
    int left = num > 0 ? num - 1 : num_philosophers-1;
    int right = num < num_philosophers ? num : 0;

    // Add a log level for philosopher specific logs.
    iw_log_add_level(PHILO_LOG, "The simple application general log level");

    while(true) {
        philo_take_forks(num, left, right);
    }

    return NULL;
}

// --------------------------------------------------------------------------

/// @brief A crash callback function to show-case the crash handler feature.
/// Executes an invalid pointer de-reference to cause a crash.
bool crash(FILE *out, const char *cmd, iw_cmd_parse_info *info) {
    char *ptr = (char *)0xdeadbeef;
    *ptr = 'w';
    return false;
}

// --------------------------------------------------------------------------

/// @brief The main callback function.
/// Called by the framework once the initial setup is done.
/// @param argc The number of arguments.
/// @param argv The arguments.
/// @return False if the program exited due to an error.
bool main_callback(int argc, char **argv) {
    int max;
    intptr_t cnt;
    char buffer[32];

    // Add a command to display the currently connected clients.
    iw_cmd_add(NULL, "crash", crash,
            "Causes a de-reference of an invalid pointer.",
            "Used to show the InstaWorks crash-handler functionality.\n"
            );

    s_mutexes = (IW_MUTEX *)IW_CALLOC(num_philosophers, sizeof(IW_MUTEX));

    // Set the random seed for the philosophers
    srandom(time(NULL));

    for(cnt=0,max=num_philosophers;cnt < max;cnt++) {
        snprintf(buffer, sizeof(buffer), "Mutex %zd", cnt+1);
        s_mutexes[cnt] = iw_mutex_create(buffer);
    }
    for(cnt=0,max=num_philosophers;cnt < max;cnt++) {
        snprintf(buffer, sizeof(buffer), "Philosopher %zd", cnt+1);
        iw_thread_create(buffer, philo_callback, (void *)cnt);
    }

    // Done, let the main thread go into the framework loop
    iw_main_loop();

    return true;
}

// --------------------------------------------------------------------------

static void print_help(const char *error) {
    printf("philosopher - A simple example to solve the philosopher problem.\n"
           "\n");
    if(error != NULL) {
        printf("Error: %s\n\n", error);
    }
    printf("A simple program that implements the philosopher problem. A number\n"
           "of philosophers are created with each having only one 'fork' (mutex)\n"
           "to use. If the program is run without the -c option, a bug in the\n"
           "implementation will create a dead-lock.\n"
           "\n"
           "Usage: philosopher [options] [num philosophers]\n"
           "\n"
           "[num philosophers]\n"
           "    The number of philosophers to use (default is 5)\n"
           "\n"
           " -c\n"
           "    Run the program correctly, avoiding the dead-lock\n"
           " -f\n"
           "    Run the program.\n"
           " -l <level>\n"
           "    The <loglevel> is the desired log level. The log level is a sum of individual\n"
           "    levels in either decimal or hexadecimal.\n");
    iw_log_list(stdout);
    printf("\n"
           "If the program is started without any command line options it will\n"
           "run in client mode and send control commands to a running server.\n"
           "Run 'philosopher help' once the server is running for more help on this.\n"
           "\n");
}

// --------------------------------------------------------------------------
//
// Main program
//
// --------------------------------------------------------------------------

/// @brief The program main entrypoint.
/// @param argc The number of arguments.
/// @param argv The arguments.
/// @return -1 if an error occurred.
int main(int argc, char **argv) {
    int opt;

    // No calls to the instaworks framework should be done before calling
    // iw_init() or before iw_main() calls the provided callback function.

    // However, default settings should be changed before the call to
    // iw_main() to make sure that settings that are processed by iw_main()
    // are set before they are accessed.
    iw_val_store_set_number(&iw_cfg, IW_CFG_CMD_PORT, 10002);
    iw_val_store_set_string(&iw_cfg, IW_CFG_CRASHHANDLER_FILE, "/tmp/philo.txt");

    if(argc == 1) {
        print_help(NULL);
        exit(0);
    }

    // Process command-line parameters ourselves before calling iw_main().
    while((opt = getopt(argc, argv, ":cfl:")) != -1) {
        long long int loglevel;
        switch(opt) {
        case 'c' :
            do_correct = true;
            break;
        case 'f' :
            iw_val_store_set_number(&iw_cfg, IW_CFG_FOREGROUND, 1);
            break;
        case 'l' :
            if(!iw_strtoll(optarg, &loglevel, 16)) {
                print_help("Invalid log level");
                exit(-1);
            }
            iw_val_store_set_number(&iw_cfg, IW_CFG_FOREGROUND, loglevel);
            break;
        default :
            print_help("Invalid parameter");
            exit(-1);
            break;
        }
    }

    int *foreground = iw_val_store_get_number(&iw_cfg, IW_CFG_FOREGROUND);
    if(optind < argc && foreground != NULL && *foreground) {
        if(!iw_strtoll(argv[optind], &num_philosophers, 10)) {
            print_help("Expected number of philosophers");
            exit(-1);
        }
    }

    // Calling iw_main(). In this case we are parsing command line parameters
    // in the main program to get full control. We set the instaworks
    // settings for whether the program should execute as a server or client
    // before calling iw_main(). Since we processed the parameters we pass
    // 0 and NULL for argc and argv.
    IW_MAIN_EXIT retval = iw_main(main_callback, false, argc, argv);

    unsigned int exit_code = -1;
    switch(retval) {
    case IW_MAIN_SRV_INVALID_PARAMETER :
        print_help("Invalid command-line options");
        break;
    case IW_MAIN_SRV_NO_OPTS :
        print_help(NULL);
        break;
    case IW_MAIN_SRV_OK :
    case IW_MAIN_CLNT_OK :
        exit_code = 0;
        break;
    case IW_MAIN_SRV_FAILED :
        printf("Failed to start program!\n");
        break;
    default :
        break;
    }

    return exit_code;
}

// --------------------------------------------------------------------------
