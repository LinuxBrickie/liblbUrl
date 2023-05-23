# liblbUrl

A GPL-3.0-or-later C++ library for making URL requests.

lb is short for LinuxBrickie, my online handle.

## Dependencies

The main library dependencies are
- libcurl

In addition the gtest binary dependencies are
- libmicrohttpd

## Usage

Request URLs using an instance of the Requester class. Your callback will be
invoked asynchronously.

## Notes

Built and tested on Fedora 37 against
- libcurl 7.85.0
- libmicrohttpd 0.9.76

Only basic HTTP is supported at the moment but the design is such that
it can be easily extended.
