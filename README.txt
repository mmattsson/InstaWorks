Introduction
=============================================================================
InstaWorks is a support library for adding a debug framework to programs or
daemons. A debug framework is usually not the first thing being considered
when creating a new program. When creating a new program, the first priority
is to quickly get a proof of concept working. This may be due to general
excitement of trying out something new, or because a dead-line is imposed
by the manager of the project. Once the proof of concept is done, the next
priority is usually to extend the functionality to a usable first version.
Again, dead-lines have to be met.

Because of this, it isn't until after the first version has shipped
that the matter of debugging the program is considered. At this point it
may be hard to graft a debug framework onto the program in question. Also,
since there are always more features to add, there is never a good time to
take the time out of the schedule to add the debug facilities.

Therefore, InstaWorks was created to provide an instant debug framework
support library that can be used when creating new programs. By simply
linking InstaWorks and use the provided API, a number of services are
provided that helps provide debug facilities to any new program with a
minimal amount of time needed. The time savings from not having to create
debug facilities can be spent on adding more features to the new program
instead.


Features
=============================================================================
The following is a list of features that InstaWorks provides. This is not an
exhaustive list.

Debug Logs
-------------------
InstaWorks provides a debug log mechanism so that a program can send debug
logs to any terminal. Different log levels are supported as well as the
ability to enable and disable these log levels during run-time.

It is also possible to enable and disable logs based on threads. Control
commands for enabling logging based on thread-id is available by default.
Programs can create commands for logging based on other criteria, e.g.
client IP address, by enabling thread logging when handling client requests
from the specified IP address.

Control commands
-------------------
InstaWorks has a control command feature which allows the same program to be
invoked in two different modes. The main mode is the server mode. This is
used when running the program to provide whatever feature or service that
the program provides. The other mode is a client mode. All command line
input in client mode is sent to the currently running server instance and
the server can respond to the client and have the output be shown on the
terminal that the client is executed from.

This allows you to provide control commands where the user can issue a
command and have the server print out information, e.g. the number of
clients connected to the server and what addresses they are connected
from.

Syslog buffer
-------------------
The syslog buffer can be used to retain syslogs for a longer period of
time. The syslogs are still sent to the normal syslog() call but a copy
of the message is stored in an internal buffer. This can be helpful to
debug issues where the server is running over a long period of time and
you have a limited amount of syslog storage.

Dead-lock detection
-------------------
InstaWorks provides a dead-lock detection by periodically scanning the
mutexes and the threads owning them. If a cyclical dependency is detected,
a notification is printed.

Memory tracking
-------------------
InstaWorks can run in a memory tracking mode. In this mode, each memory
allocation can be tracked and the current memory usage can be queried at
run-time. This allows for easier debugging of memory leaks in target
environments.


Building InstaWorks
=============================================================================
To build the InstaWorks library, documentation, self-test, and all examples,
simply type make in the InstaWorks directory:
 $ make
 

Project Organization
=============================================================================
The main library source code is contained in the 'src' folder. This folder
contains all the C code for the library as well as internal headers that a
user would not need to care about. All externally visible API's and 
data structures are found in the 'includes' folder. The resulting linkable
library is found in the 'bin' directory.

The structure is as follows:
  bin      - The resulting library and self-test
  examples - Example programs using the InstaWorks library
  includes - All include files needed to work with InstaWorks
  objs     - Temporary directory containing object files
  selftest - The selftest source code
  src      - The InstaWorks library source


Self-test
=============================================================================
In order to verify that all the code is working as expected, there is a 
self-test directory that contains test code for various code modules. These
self-tests are compiled into a selftest binary that can be run under valgrind.
There is a 'run_selftest.sh' shell script that does this. This can be run
by issuing the command:
 InstaWorks $ selftest/run_selftest.sh

If any of the tests fails, this will be noted. If there are memory access
errors or memory leaks, valgrind will note that.


Examples
=============================================================================

Simple
-------------------
This example shows a simple TCP server. The server accepts connections on
the listening port. If data is received on any connection, this data is sent
on all other connections.

See examples/simple/README.txt for more information.
 

Philosophers
-------------------
This example shows a simple multi-threaded program. The program has a bug
in it that will eventually lead to a dead-lock. The dead-lock is detected
by the framework's health-check thread.

See examples/philosophers/README.txt for more information.


Documentation
=============================================================================
In order to create the InstaWorks documentation you should have doxygen
installed on your system. Once doxygen is installed you can issue the
following command:
 InstaWorks $ make dox

This will create the documentation in the html sub-folder. It can be
accessed by opening the file html/index.html in a browser.


License
=============================================================================
InstaWorks is covered by the license in LICENSE.txt in the top InstaWorks
directory.

