Philosophers
=============================================================================
This example shows a program implementing the philosophers problem. The
default mode is to run the program with a bug present which will cause a
dead-lock. The InstaWorks dead-lock detection will detect the dead-lock
and print out the callstacks for the threads in question.

Run the example with log level of 3 to see the dead-lock detection:
 $ examples/philosophers/philosophers -f -l 3

Run the example with -c to run the code in a correct mode. No dead-lock will
occur.
 $ examples/philosophers/philosophers -f -l 3

You can set the number of threads that will be created by passing in a number
(default is 5).


This example also includes a control command 'crash' that causes a NULL
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


Implementation notes
=============================================================================
The philosophers example parses the command-line in its main program before
calling iw_main(). Because of this it has to set the iw_stg.iw_foreground
flag to true if the server should be run when iw_main() is called. Otherwise
the client mode is executed instead.

No log levels or control commands can be registered before the call to
iw_main(). These has to be registered after the callback is done so this
is done in main_callback().

