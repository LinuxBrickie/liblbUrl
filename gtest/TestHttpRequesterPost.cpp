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

#include "ServerList.h"

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


const std::string baseUrl( int port ) { return "http://" + hostColonPort( port ); }

const std::string POST200Url{ "/test/url/http/post/200" };
const std::string POST404Url{ "/test/url/http/post/404" };
const std::string POSTMultilineUrl{ "/test/url/http/post/multiline" };
const std::string POSTUnterminatedUrl{ "/test/url/http/post/unterminated" };
const std::string POSTContainsNullUrl{ "/test/url/http/post/contains-null" };
const std::string POSTFormDataNoEncoding{ "/test/url/http/post/form/no-encoding" };
const std::string POSTFormDataFieldEncoding{ "/test/url/http/post/form/field-encoding" };
const std::string POSTFormDataValueEncoding{ "/test/url/http/post/form/value-encoding" };
const std::string POSTFormDataFieldAndValueEncoding{ "/test/url/http/post/form/field-and-value-encoding" };
const std::string POSTFormDataEmptyValue{ "/test/url/http/post/form/empty-field" };
const std::string POSTFormDataLarge{ "/test/url/http/post/form/large" };
const std::string POSTMimeFormDataSimple{ "/test/url/http/post/mime/form/simple" };
const std::string POSTMimeFormDataContainsNull{ "/test/url/http/post/mime/form/contains-null" };
const std::string POSTMimeFormDataLarge{ "/test/url/http/post/mime/form/large" };
const std::string POSTMimeFormDataMulti{ "/test/url/http/post/mime/form/multi" };


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

const int POSTFormDataLargeNumFields{ 1000000 };
std::string POSTFormDataLargeDataString()
{
  lb::url::http::UrlEncodedValuesCreator creator;
  for ( int i = 0; i < POSTFormDataLargeNumFields; ++i )
  {
    const auto s{ std::to_string( i ) };
    creator.add( { "field#" + s }, { "value#" + s } );
  }
  return creator.str();
}

const int POSTMimeFormDataSimpleNumBytes{ 100 };
const int POSTMimeFormDataLargeNumBytes{ 1000000000 };
const int POSTMimeFormDataMulti1NumBytes{ 1000 };
const int POSTMimeFormDataMulti2NumBytes{ 200 };
const int POSTMimeFormDataMulti3NumBytes{ 333 };


const std::unordered_map<std::string, TestData> POSTTestData
{
  //////////////////////////////////////////////
  // Tests for application/x-www-form-urlencoded
  //
  // For the following tests note that libcurl automatically sets the
  // Content-Type header to application/x-www-form-urlencoded.

  // First five tests do not use the POSTed data at all at server side.
  {
    POST200Url,
    {
      {
        lb::url::http::Request::Method::ePost,
        POST200Url,
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
        POST404Url,
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
        POSTMultilineUrl,
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
        POSTUnterminatedUrl,
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
        POSTContainsNullUrl,
        {}, // headers
        "Some data for " + POSTContainsNullUrl
      },
      {
        200,
        { "POST test response contains \0 and \0", 35 }
      }
    }
  },

  {
    POSTFormDataNoEncoding,
    {
      {
        lb::url::http::Request::Method::ePost,
        POSTFormDataNoEncoding,
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
        POSTFormDataFieldEncoding,
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
        POSTFormDataValueEncoding,
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
        POSTFormDataFieldAndValueEncoding,
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
        POSTFormDataEmptyValue,
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

  {
    POSTFormDataLarge,
    {
      {
        lb::url::http::Request::Method::ePost,
        POSTFormDataLarge,
        {}, // headers
        POSTFormDataLargeDataString()
      },
      {
        200,
        { "Processed " + std::to_string( POSTFormDataLargeNumFields ) + " fields with values" }
      },
      false
    }
  },

  /////////////////////////////////
  // Tests for multipart/form-data
  //
  // For the following tests note that libcurl automatically sets the
  // Content-Type header to multipart/form-data (and specifies the boundary).

  {
    POSTMimeFormDataSimple,
    {
      {
        lb::url::http::Request::Method::ePost,
        POSTMimeFormDataSimple,
        {}, // headers
        {}, // x-www-form-urlencoded data, not relevant here
        {
          {
            {
              {}, // type, not required
              { "binary" }, // encoding
              { "simple" }, // name of MIME part
              { std::string( POSTMimeFormDataSimpleNumBytes, '0' ) }  // Note curly braces here would initialise a 2-character string.
            }
          }
        }
      },
      {
        200,
        { "Processed " + std::to_string( POSTMimeFormDataSimpleNumBytes ) + " bytes of data from MIME part" }
      },
      false
    }
  },

  {
    POSTMimeFormDataContainsNull,
    {
      {
        lb::url::http::Request::Method::ePost,
        POSTMimeFormDataContainsNull,
        {}, // headers
        {}, // x-www-form-urlencoded data, not relevant here
        {
          {
            {
              {}, // type, not required
              { "binary" }, // encoding
              { "contains-null" }, // name of MIME part
              { "aaa\0bbb\0ccc\0dddd", 16 }
            }
          }
        }
      },
      {
        200,
        { "Processed " + std::to_string( 16 ) + " bytes of data from MIME part" }
      },
      false
    }
  },

  {
    POSTMimeFormDataLarge,
    {
      {
        lb::url::http::Request::Method::ePost,
        POSTMimeFormDataLarge,
        {}, // headers
        {}, // x-www-form-urlencoded data, not relevant here
        {
          {
            {
              {}, // type, not required
              { "binary" }, // encoding
              { "large" }, // name of MIME part
              { std::string( POSTMimeFormDataLargeNumBytes, '0' ) }  // Note curly braces here would initialise a 2-character string.
            }
          }
        }
      },
      {
        200,
        { "Processed " + std::to_string( POSTMimeFormDataLargeNumBytes ) + " bytes of data from MIME part" }
      },
      false
    }
  },

  {
    POSTMimeFormDataMulti,
    {
      {
        lb::url::http::Request::Method::ePost,
        POSTMimeFormDataMulti,
        {}, // headers
        {}, // x-www-form-urlencoded data, not relevant here
        {
          {
            {
              {}, // type, not required
              { "binary" }, // encoding
              { "multi1" }, // name of MIME part
              { std::string( POSTMimeFormDataMulti1NumBytes, '1' ) }  // Note curly braces here would initialise a 2-character string.
            },
            {
              {}, // type, not required
              { "binary" }, // encoding
              { "multi2" }, // name of MIME part
              { std::string( POSTMimeFormDataMulti2NumBytes, '2' ) }  // Note curly braces here would initialise a 2-character string.
            },
            {
              {}, // type, not required
              { "binary" }, // encoding
              { "multi3" }, // name of MIME part
              { std::string( POSTMimeFormDataMulti3NumBytes, '3' ) }  // Note curly braces here would initialise a 2-character string.
            }
          }
        }
      },
      {
        200,
        { "Processed 3 parts, with "
        + std::to_string( POSTMimeFormDataMulti1NumBytes ) + ", "
        + std::to_string( POSTMimeFormDataMulti2NumBytes ) + ", and "
        + std::to_string( POSTMimeFormDataMulti3NumBytes ) + " bytes of data from MIME" }
      },
      false
    }
  },
};

TEST(Http, RequesterPost)
{
  lb::url::Requester requester;

  for ( auto&[type, serverConfigs] : serverList )
  {
    for ( const auto& serverConfig : serverConfigs )
    {
      for ( const auto&[ urlPath, testData ] : POSTTestData )
      {
        std::promise< std::pair< lb::url::ResponseCode
                               , lb::url::http::Response > > promise;

        // Map Request only contains a URL path so need to build up full URL
        lb::url::http::Request request{ testData.request };
        request.url = baseUrl( serverConfig.port ) + request.url;

        requester.makeRequest( request
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
  }
}

