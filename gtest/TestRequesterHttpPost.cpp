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

#include "TestRequesterHttpPost.h"

#include <gtest/gtest.h>

#include <future>

#include <lb/url/Requester.h>


TEST(Requester, HttpPost)
{
  lb::url::Requester requester;

  for ( const auto&[ url, testData ] : POSTTestData )
  {
    std::promise<lb::url::http::Response> promise;

    requester.makeRequest( testData.request
                         , [ &promise ]( lb::url::ResponseCode rc, lb::url::http::Response r )
    {
      promise.set_value( std::move( r ) );
    } );

    lb::url::http::Response actualResponse{ promise.get_future().get() };

    EXPECT_EQ( actualResponse.code   , testData.response.code );
    EXPECT_EQ( actualResponse.content, testData.response.content );
  }
}
