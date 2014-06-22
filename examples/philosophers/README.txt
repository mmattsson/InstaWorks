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


Implementation notes
=============================================================================
The philosophers example parses the command-line in its main program before
calling iw_main(). Because of this it has to set the iw_stg.iw_foreground
flag to true if the server should be run when iw_main() is called. Otherwise
the client mode is executed instead.

No log levels or run-time queries can be registered before the call to
iw_main(). These has to be registered after the callback is done so this
is done in main_callback().

