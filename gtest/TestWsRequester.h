#ifndef TESTWSREQUESTER_H
#define TESTWSREQUESTER_H

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

#include <map>
#include <string>
#include <unordered_map>

#include <lb/httpd/Server.h>
#include <lb/url/ResponseCode.h>


const std::string sendControlCloseMessage{ "SEND BACK CONTROL CLOSE" };


struct ExpectedResponse
{
  lb::url::ResponseCode responseCode; //!< i.e. was it a valid WebSocket URL
  std::map< std::string, std::string > responseMessages;
};

//! Keyed by URL path
extern const std::unordered_map<std::string, ExpectedResponse> WSExpectedMockResponses;


#endif // TESTWSREQUESTER_H