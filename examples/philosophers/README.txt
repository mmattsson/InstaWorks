Philosophers
=============================================================================

Program usage
-----------------------------------------------------------------------------
This example shows a program implementing the philosophers problem. The
default mode is to run the program with a bug present which will cause a
dead-lock. The InstaWorks dead-lock detection will detect the dead-lock
and print out the callstacks for the threads in question.

Run the example with log level of 3 to see the dead-lock detection:
 $ examples/philosophers/philosophers -f -l 3

Run the example with -c to run the code in a correct mode. No dead-lock will
occur.
 $ examples/philosophers/philosophers -f -c -l 3

You can set the number of threads that will be created by passing in a number
(default is 5).
 $ examples/philosophers/philosophers -f -c 6


Control commands
-----------------------------------------------------------------------------

Crash
-----------------------------------------------------------
This example includes a custom control command 'crash' that causes a NULL
pointer de-reference. This can be used to show how the crash handler feature
works. Issuing the command 'crash' will cause a file /tmp/callstack.txt to
be written with information regarding the thread that caught the signal such
as the callstack and the signal that was caught. For example:
 $ examples/philosophers/philosophers crash
 $ cat /tmp/callstack.txt
Program: philosophers
Caught signal: 11
Callstack:
-------------------
examples/philosophers/philosophers[0x804db19]
[0xb7774400]
examples/philosophers/philosophers(crash+0x10)[0x804a7fb]
examples/philosophers/philosophers[0x804adad]
examples/philosophers/philosophers(iw_cmds_process+0x20)[0x804b5c0]


Thread-specific logging
-----------------------------------------------------------
This example show-cases the thread-specific logging ability built into
InstaWorks. You can see this built-in control command by running the
philosopher example in correct mode (see above) and then turn off logging
for all threads and re-enable logging for one or two threads. This
feature is easiest to see if you do logging in another terminal.
 $ examples/philosophers/philosophers -f -c
 
And in another terminal:
 $ examples/philosophers/philosophers log thread all off
Received request: log thread all off

 $ examples/philosophers/philosophers threads
Received request: threads
== Thread Information ==
Thread-ID  Log Mutex  Thread-name
---------------------------------
[B6F3FB40] on  0000 : "CMD Server"
[B473AB40] on  0005 : "Philosopher 5"
[B7740B40] on  0000 : "Health Check"
[B4F3BB40] on  0003 : "Philosopher 4"
[B7741940] on  0000 : "Main"
[B573CB40] on  0000 : "Philosopher 3"
[B5F3DB40] on  0000 : "Philosopher 2"
[B673EB40] on  0001 : "Philosopher 1"

 $ examples/philosophers/philosophers log thread 0xB673EB40 on
Received request: log thread 0xB673EB40 on

 $ examples/philosophers/philosophers log lvl 0xf `tty`
Received request: log lvl 0xf /dev/pts/2

[B673EB40]main.c(84): Philosopher[1] - Taking fork 2
[B673EB40]main.c(90): Philosopher[1] - Got forks 6 and 2
[B673EB40]main.c(98): Philosopher[1] - Releasing forks 6 and 2
[B673EB40]main.c(78): Philosopher[1] - Taking fork 6
[B673EB40]main.c(84): Philosopher[1] - Taking fork 2
[B673EB40]main.c(90): Philosopher[1] - Got forks 6 and 2
[B673EB40]main.c(98): Philosopher[1] - Releasing forks 6 and 2
[B673EB40]main.c(78): Philosopher[1] - Taking fork 6
[B673EB40]main.c(84): Philosopher[1] - Taking fork 2

As you can see, only logs from the first philosopher thread is printed.


Implementation notes
=============================================================================
The philosophers example parses the command-line in its main program before
calling iw_main() rather than using InstaWorks' command line parsing utilities. 
Because of this it has to set the iw_stg.iw_foreground flag to true if the
server should be run when iw_main() is called. Otherwise the client mode is
executed instead.

No log levels or control commands can be registered before the call to
iw_main(). These have to be registered after the callback is done so this
is done in main_callback().

