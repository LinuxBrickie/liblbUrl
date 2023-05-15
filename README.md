# liblbUrl

A GPL-3.0-or-later C++ library for making URL requests.

lb is short for LinuxBrickie, my online handle.

Uses libcurl to do the actual work. Built and tested on Fedora 37 against libcurl version 7.85.0.

Request URLs using an instance of the Requester class.

NOTE: Only basic HTTP is supported at the moment but the design is such that
it can be easily extended.
