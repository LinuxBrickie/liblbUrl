#ifndef LIB_LB_URL_GTEST_TESTREQUESTERHTTPGET_H
#define LIB_LB_URL_GTEST_TESTREQUESTERHTTPGET_H

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

#include <unordered_map>
#include <string>

#include "httpd/Server.h"


//! Keyed by URL path
extern const std::unordered_map<std::string, httpd::Server::Response> GETExpectedMockResponses;


#endif // LIB_LB_URL_GTEST_TESTREQUESTERHTTPGET_H
