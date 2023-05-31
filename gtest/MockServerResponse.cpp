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

#include "MockServerResponse.h"

#include <stdexcept>

#include "TestHttpRequesterGet.h"
#include "TestHttpRequesterPost.h"

httpd::Server::Response createPostResponse( const std::string& url
                                          , const httpd::Server::PostKeyValues keyValues )
{
  httpd::Server::Response response{ 200 };

  try
  {
    if ( url == POSTFormDataNoEncoding )
    {
       response.content = keyValues.at( "handle" ) + ", your real name is " + keyValues.at( "name" ) + '!';
    }
    else if ( url == POSTFormDataFieldEncoding )
    {
       response.content = keyValues.at( "handle" ) + ", your encoded field has value " + keyValues.at( "+-=#';[]" );
    }
    else if ( url == POSTFormDataValueEncoding )
    {
       response.content = keyValues.at( "handle" ) + ", your encoded value is " + keyValues.at( "encoded" );
    }
    else if ( url == POSTFormDataFieldAndValueEncoding )
    {
       response.content = keyValues.at( "name" ) + ", your encoded field has encoded value " + keyValues.at( "£^&,.+_" );
    }
    else if ( url == POSTFormDataEmptyValue )
    {
       response.content = "Field values are \"" + keyValues.at( "encoded" )
                        + "\" and \"" + keyValues.at( "unencoded" ) + "\"";
    }
    else if ( url == POSTFormDataLarge )
    {
      response.content = "Processed "
                       + std::to_string( keyValues.size() )
                       + " fields with values";
    }
    else
    {
      response.code = 400; // Bad request
    }
  }
  catch( std::out_of_range e )
  {
    response.code = 400; // Bad request
  }

  return response;
}

httpd::Server::Response mockServerResponse( std::string url,
                                            httpd::Server::Method method,
                                            httpd::Server::Version version,
                                            std::string requestPayload,
                                            const httpd::Server::PostKeyValues& postKeyValues )
{
  // Initialise to something clearly wrong and use this if we don't match the URL.
  httpd::Server::Response response
  {
    500U, // Must be 3-digit code to be valid
    "Invalid test URL"
  };

  switch ( method )
  {
  case httpd::Server::Method::eInvalid:
    response.content = "Invalid HTTP method";
    break;
  case httpd::Server::Method::eGet:
  {
    const auto I{ GETExpectedMockResponses.find( url ) };
    if ( I != GETExpectedMockResponses.end() )
    {
      response = I->second;
    }
    break;
  }
  case httpd::Server::Method::eHead:
    break;
  case httpd::Server::Method::ePost:
  {
    const auto I{ POSTTestData.find( url ) };
    if ( I != POSTTestData.end() )
    {
      const TestData& testData{ I->second };
      if ( testData.shouldServerUseResponseVerbatim )
      {
        response = testData.response;
      }
      else
      {
        response = createPostResponse( url, postKeyValues );
      }
    }
    break;
  }
  case httpd::Server::Method::ePut:
    break;
  case httpd::Server::Method::eDelete:
    break;
  }

  return response;
}
