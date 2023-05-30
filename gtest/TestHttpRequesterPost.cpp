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

#include "TestHttpRequesterPost.h"

#include <gtest/gtest.h>

#include <future>

#include "ConnectionDetails.h"

#include <lb/url/http/UrlEncodedValuesCreator.h>
#include <lb/url/Requester.h>
#include <lb/url/ResponseCode.h>


// Printer for correct gtest output.
namespace lb { namespace url {
  void PrintTo( const ResponseCode& rc, std::ostream* os )
  {
    *os << "ResponseCode " << toString( rc );
  }
} }


const std::string baseUrl{ "http://" + HOST_COLON_PORT };

const std::string POST200Url{ "/test/url/http/post/200" };
const std::string POST404Url{ "/test/url/http/post/404" };
const std::string POSTMultilineUrl{ "/test/url/http/post/multiline" };
const std::string POSTUnterminatedUrl{ "/test/url/http/post/unterminated" };
const std::string POSTContainsNullUrl{ "/test/url/http/post/containsnull" };
const std::string POSTFormDataNoEncoding{ "/test/url/http/post/form/no-encoding" };
const std::string POSTFormDataFieldEncoding{ "/test/url/http/post/form/field-encoding" };
const std::string POSTFormDataValueEncoding{ "/test/url/http/post/form/value-encoding" };
const std::string POSTFormDataFieldAndValueEncoding{ "/test/url/http/post/form/field-and-value-encoding" };
const std::string POSTFormDataEmptyValue{ "/test/url/http/post/form/empty-field" };


std::string POSTFormDataUrlNoEncodingDataString()
{
  lb::url::http::UrlEncodedValuesCreator creator;
  creator.add( { "name", false }, { "Paul", false } );
  creator.add( { "handle", false }, { "LinuxBrickie", false } );
  return creator.str();
}

std::string POSTFormDataFieldEncodingDataString()
{
  lb::url::http::UrlEncodedValuesCreator creator;
  creator.add( { "+-=#';[]" }, { "gobbledygook", false } );
  creator.add( { "handle", false }, { "LinuxBrickie", false } );
  return creator.str();
}

std::string POSTFormDataValueEncodingDataString()
{
  lb::url::http::UrlEncodedValuesCreator creator;
  creator.add( { "encoded", false }, { "%&$!*()^" } );
  creator.add( { "handle", false }, { "LinuxBrickie", false } );
  return creator.str();
}

std::string POSTFormDataFieldAndValueEncodingDataString()
{
  lb::url::http::UrlEncodedValuesCreator creator;
  creator.add( { "name", false }, { "Paul", false } );
  creator.add( { "£^&,.+_" }, { "<>(){}?/\\|" } );
  return creator.str();
}

std::string POSTFormDataEmptyValueDataString()
{
  lb::url::http::UrlEncodedValuesCreator creator;
  creator.add( { "encoded", false }, {} );
  creator.add( { "unencoded" }, { "", false } );
  return creator.str();
}

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
  },

  // For the following four tests note that libcurl automatically sets the
  // Content-Type header to application/x-www-form-urlencoded.
  {
    POSTFormDataNoEncoding,
    {
      {
        lb::url::http::Request::Method::ePost,
        baseUrl + POSTFormDataNoEncoding,
        {}, // headers
        POSTFormDataUrlNoEncodingDataString()
      },
      {
        200,
        { "LinuxBrickie, your real name is Paul!" }
      },
      false
    }
  },

  {
    POSTFormDataFieldEncoding,
    {
      {
        lb::url::http::Request::Method::ePost,
        baseUrl + POSTFormDataFieldEncoding,
        {}, // headers
        POSTFormDataFieldEncodingDataString()
      },
      {
        200,
        { "LinuxBrickie, your encoded field has value gobbledygook" }
      },
      false
    }
  },

  {
    POSTFormDataValueEncoding,
    {
      {
        lb::url::http::Request::Method::ePost,
        baseUrl + POSTFormDataValueEncoding,
        {}, // headers
        POSTFormDataValueEncodingDataString()
      },
      {
        200,
        { "LinuxBrickie, your encoded value is %&$!*()^" }
      },
      false
    }
  },

  {
    POSTFormDataFieldAndValueEncoding,
    {
      {
        lb::url::http::Request::Method::ePost,
        baseUrl + POSTFormDataFieldAndValueEncoding,
        {}, // headers
        POSTFormDataFieldAndValueEncodingDataString()
      },
      {
        200,
        { "Paul, your encoded field has encoded value <>(){}?/\\|" }
      },
      false
    }
  },

  {
    POSTFormDataEmptyValue,
    {
      {
        lb::url::http::Request::Method::ePost,
        baseUrl + POSTFormDataEmptyValue,
        {}, // headers
        POSTFormDataEmptyValueDataString()
      },
      {
        200,
        { "Field values are \"\" and \"\"" }
      },
      false
    }
  },

  // Still to do the following
  // - "multipart/formdata"
  // - large data
};

TEST(Http, RequesterPost)
{
  lb::url::Requester requester;

  for ( const auto&[ url, testData ] : POSTTestData )
  {
    std::promise< std::pair< lb::url::ResponseCode
                           , lb::url::http::Response > > promise;

    requester.makeRequest( testData.request
                         , [ &promise ]( lb::url::ResponseCode rc, lb::url::http::Response r )
    {
      promise.set_value( { rc, std::move( r ) } );
    } );

    const auto actualResponse{ promise.get_future().get() };

    ASSERT_EQ( actualResponse.first         , lb::url::ResponseCode::eSuccess );
    EXPECT_EQ( actualResponse.second.code   , testData.response.code );
    EXPECT_EQ( actualResponse.second.content, testData.response.content );
  }
}

