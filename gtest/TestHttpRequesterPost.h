#ifndef TESTREQUESTERHTTPPOST_H
#define TESTREQUESTERHTTPPOST_H

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
#include <vector>

#include "httpd/Server.h"

#include <lb/url/http/Request.h>


struct TestData
{
  lb::url::http::Request request;
  httpd::Server::Response response;
  bool shouldServerUseResponseVerbatim = true;
};

//! Keyed by URL path
extern const std::unordered_map<std::string, TestData> POSTTestData;

// Some URLs that need sharing with MockServerResponse
extern const std::string POSTFormDataNoEncoding;
extern const std::string POSTFormDataFieldEncoding;
extern const std::string POSTFormDataValueEncoding;
extern const std::string POSTFormDataFieldAndValueEncoding;
extern const std::string POSTFormDataEmptyValue;
extern const std::string POSTFormDataLarge;
extern const std::string POSTMimeFormDataSimple;
extern const std::string POSTMimeFormDataContainsNull;
extern const std::string POSTMimeFormDataLarge;
extern const std::string POSTMimeFormDataMulti;


#endif // TESTREQUESTERHTTPPOST_H
