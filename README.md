# liblbUrl

A GPL-3.0-or-later C++ library for making URL requests.

lb is short for LinuxBrickie, my online handle.

## Dependencies

The main library dependencies are
- liblbEncoding (available from my github account, licensed under GPL-3.0-or-later)
- libcurl (licensed under the curl license)

In addition the gtest binary dependencies are
- googletest (licensed under BSD 3-Clause)
- liblbHttpd (available from my github account, licensed under AGPL-3.0-or-later)
- - libmicrohttpd (licensed under LGPL-2.1-or-later)

## Licensing

As the copyright holder I am happy to consider alternative licensing if
the GPL-3.0-or-later licence does not suit you. Please feel free to reach
out to me at fotheringham.paul@gmail.com.

## Usage

Request URLs using an instance of the Requester class. Your callback will be
invoked asynchronously.

## Notes

Originally built and tested on Fedora 37 against
- libcurl 7.85.0
- libmicrohttpd 0.9.76

The addition of WebSocket supports requires libcurl 7.86.0 or later.

Latest version built and tested on Fedora 38 against
- libcurl 8.0.1
- libmicrohttpd 0.9.76

Only basic HTTP is supported at the moment but the design is such that
it can be easily extended.
