===========================
Inesonic SpeedSentry Pinger
===========================
The Inesonic SpeedSentry Pinger project provides a small Daemon based on a
patched version of Octo's liboping library that can issue ICMP echo requests
to remote servers, monitoring for a reply.  The SpeedSentry Pinger daemon can
communicate with a SpeedSentry Polling Server to monitor the status of remote
webservers and REST API endpoints that accept ICMP echo messages.

The project includes a small GUI tool you can use to exercise and test the
daemon.

You will need to download Octo's liboping library from
https://noping.cc/#download and patch it using the supplied patch (in the
extra directory).

Like most of SpeedSentry, the SpeedSentry pinger tool uses the QMAKE build
tool and depends on the Qt libraries.  Be sure to set the following QMAKE
variables on the QMAKE command line:

* OPING_INCLUDE

* OPING_LIBDIR


Licensing
=========
This code is licensed under the GNU Public License, version 3.0.
