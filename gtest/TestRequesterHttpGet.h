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

#ifndef LIB_LB_URL_GTEST_TESTREQUESTERHTTPGET_H
#define LIB_LB_URL_GTEST_TESTREQUESTERHTTPGET_H

#include <unordered_map>
#include <string>

#include "httpd/Server.h"

//! Keyed by URL path
const std::unordered_map<std::string, httpd::Server::Response> GETExpectedMockResponses
{
  {
    "/test/url/http/get200",
    {
      200,
      "GET test response SUCCESS"
    }
  },
  {
    "/test/url/http/get401",
    {
      404,
      "GET test response UNAUTHORISED"
    }
  },
  {
    "/test/url/http/get404",
    {
      404,
      "GET test response NOT FOUND"
    }
  },
  {
    "/test/url/http/get/multline",
    {
      200,
      "GET test response multi-line\n"
      "Line 1\n"
      "Line 2"
    }
  },
  // Note that the following test is a bit misleading. Requester returns the
  // data in a std::string so it is impossible to know if it is null-terminated
  // or not. That said libcurl does not appear to explicitly give us a null
  // terminator for any of the previous tests so in reality nothing is
  // null-terminated at point of receipt.
  {
    "/test/url/http/get/unterminated",
    {
      200,
      { "GET test response unterminated", 30 }
    }
  },
  {
    "/test/url/http/get/containsnull",
    {
      200,
      { "GET test response contains \0 and \0", 34 }
    }
  },
};


#endif // LIB_LB_URL_GTEST_TESTREQUESTERHTTPGET_H
