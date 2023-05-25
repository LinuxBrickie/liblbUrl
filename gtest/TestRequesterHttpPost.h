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
#ifndef TESTREQUESTERHTTPPOST_H
#define TESTREQUESTERHTTPPOST_H

#include <unordered_map>
#include <string>
#include <vector>

#include "ConnectionDetails.h"
#include "httpd/Server.h"

#include <lb/url/http/Request.h>


//! Keyed by URL path
//struct TestData
//{
//  lb::url::http::Request request;
////  std::string postData;
//  httpd::Server::Response mockResponse;
//};

const std::string baseUrl{ "http://" + HOST_COLON_PORT };

const std::string POST200Url{ "/test/url/http/post/200" };
const std::string POST404Url{ "/test/url/http/post/404" };
const std::string POSTMultilineUrl{ "/test/url/http/post/multiline" };
const std::string POSTUnterminatedUrl{ "/test/url/http/post/unterminated" };
const std::string POSTContainsNullUrl{ "/test/url/http/post/containsnull" };

struct TestData
{
  lb::url::http::Request request;
  httpd::Server::Response response;
};

const std::unordered_map<std::string, TestData> POSTTestData
{
  // First five tests do not use the POSTed data at all at server side.
  {
    POST200Url,
    {
      {
        lb::url::http::Request::Method::ePost,
        baseUrl + POST200Url,
        {}, // headers
        "Some data for " + POST200Url
      },
      {
        200,
        "POST test response SUCCESS"
      }
    }
  },

  {
    POST404Url,
    {
      {
        lb::url::http::Request::Method::ePost,
        baseUrl + POST404Url,
        {}, // headers
        "Some data for " + POST404Url
      },
      {
        404,
        "POST test response NOT FOUND"
      }
    }
  },

  {
    POSTMultilineUrl,
    {
      {
        lb::url::http::Request::Method::ePost,
        baseUrl + POSTMultilineUrl,
        {}, // headers
        "Some data for " + POSTMultilineUrl
      },
      {
        200,
        "POST test response multi-line\n"
        "Line 1\n"
        "Line 2"
      }
    }
  },

  // Note that the following test is a bit misleading. Requester returns the
  // data in a std::string so it is impossible to know if it is null-terminated
  // or not. That said libcurl does not appear to explicitly give us a null
  // terminator for any of the previous tests so in reality nothing is
  // null-terminated at point of receipt.
  {
    POSTUnterminatedUrl,
    {
      {
        lb::url::http::Request::Method::ePost,
        baseUrl + POSTUnterminatedUrl,
        {}, // headers
        "Some data for " + POSTUnterminatedUrl
      },
      {
        200,
        { "POST test response unterminated", 31 }
      }
    }
  },

  {
    POSTContainsNullUrl,
    {
      {
        lb::url::http::Request::Method::ePost,
        baseUrl + POSTContainsNullUrl,
        {}, // headers
        "Some data for " + POSTContainsNullUrl
      },
      {
        200,
        { "POST test response contains \0 and \0", 35 }
      }
    }
  }
};


#endif // TESTREQUESTERHTTPPOST_H
