#ifndef LIB_LB_URL_GTEST_MOCKWEBSOCKETHANDLER_H
#define LIB_LB_URL_GTEST_MOCKWEBSOCKETHANDLER_H

/*
    Copyright (C) 2023  Paul Fotheringham (LinuxBrickie)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <lb/httpd/Server.h>
using WSH = lb::httpd::ws::Handler;

bool mockIsHandled( const std::string& );
lb::httpd::ws::Receivers mockConnectionEstablished( WSH::Connection );

extern WSH mockWSHandler;


#endif // LIB_LB_URL_GTEST_MOCKWEBSOCKETHANDLER_H
