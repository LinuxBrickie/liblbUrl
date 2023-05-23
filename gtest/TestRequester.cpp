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

#include "TestRequester.h"

#include <gtest/gtest.h>

#include <future>

#include <lb/url/Requester.h>

#include "ConnectionDetails.h"


TEST(Requester, HttpGet)
{
  lb::url::Requester requester;

  for ( const auto&[ url, expectedResponse ] : GETExpectedMockResponses )
  {
    std::promise<lb::url::http::Response> promise;

    requester.makeRequest( { lb::url::http::Request::Method::eGet
                           , "http://" + HOST_COLON_PORT + url }
                         , [ &promise ]( lb::url::ResponseCode rc, lb::url::http::Response r )
    {
      promise.set_value( std::move( r ) );
    } );

    lb::url::http::Response actualResponse{ promise.get_future().get() };

    EXPECT_EQ( actualResponse.code   , expectedResponse.code );
    EXPECT_EQ( actualResponse.content, expectedResponse.content );
  }
}
