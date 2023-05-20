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

#include <gtest/gtest.h>

#include "../src/HttpHandler.h"


TEST(HttpHandler, Config)
{
  struct ResponseCallback
  {
    void operator()( lb::url::ResponseCode rc, lb::url::http::Response r )
    {
    }
  } responseCallback;

  lb::url::HttpHandler httpHandler
  {
    {
      lb::url::http::Request::Method::ePost,
      "a/test/url.suffix", // URL
      {}, // headers
      {}, // POST data
      {}, // MIME data
    },
    responseCallback
  };

  // TODO - Unfortunately libcurl has no way to query the options set on the
  //        easy handle. That would have made an ideal unit test :( If this
  //        changes we can query for stuff here (and expand the request).
}
