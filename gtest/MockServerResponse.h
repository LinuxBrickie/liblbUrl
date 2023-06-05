#ifndef LIB_LB_URL_GTEST_MOCKSERVERRESPONSE_H
#define LIB_LB_URL_GTEST_MOCKSERVERRESPONSE_H

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


lb::httpd::Server::Response mockServerResponse( std::string url,
                                                lb::httpd::Server::Method method,
                                                lb::httpd::Server::Version version,
                                                std::string requestPayload,
                                                const lb::httpd::Server::PostKeyValues& );


#endif // LIB_LB_URL_GTEST_MOCKSERVERRESPONSE_H
