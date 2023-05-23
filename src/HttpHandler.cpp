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

#include "HttpHandler.h"


namespace lb
{


namespace url
{


HttpHandler::HttpHandler( http::Request r, http::Response::Callback c )
  : request{ std::move( r ) }
  , responseCallback{ std::move( c ) }
{
  curl_easy_setopt( easyHandle, CURLOPT_URL, request.url.c_str() );

  switch( request.method )
  {
  case http::Request::Method::eGet:
    curl_easy_setopt( easyHandle, CURLOPT_HTTPGET, 1L );
    break;
  case http::Request::Method::eHead:
    curl_easy_setopt( easyHandle, CURLOPT_NOBODY, 1L );
    break;
  case http::Request::Method::ePost:
    if ( !request.postData.empty() )
    {
      curl_easy_setopt( easyHandle, CURLOPT_POSTFIELDS, request.postData.c_str() );
    }
    else // assume MIME for now
    {
      mimeHelper.setOptions( easyHandle );
    }
    break;
  case http::Request::Method::ePut:
    curl_easy_setopt( easyHandle, CURLOPT_UPLOAD, 1L );
    break;
  case http::Request::Method::eDelete:
    curl_easy_setopt( easyHandle, CURLOPT_CUSTOMREQUEST, "DELETE" );
    break;
  }

  headerList = nullptr;
  for ( const auto& header : request.headers )
  {
    // Note that curl_slist_append copies the string.
    headerList = curl_slist_append( headerList, header.c_str() );
  }
  // Note that curl_easy_setopt will NOT copy the list
  curl_easy_setopt( easyHandle, CURLOPT_HTTPHEADER, headerList );
}

HttpHandler::~HttpHandler()
{
  curl_slist_free_all( headerList );
}

void HttpHandler::respond( ResponseCode rc, std::string receivedData )
{
  http::Response response;

  response.content = std::move( receivedData );

  long httpResponseCode;
  const CURLcode cc{ curl_easy_getinfo( easyHandle, CURLINFO_RESPONSE_CODE, &httpResponseCode ) };
  switch( cc )
  {
  case CURLE_OK:
    response.code = httpResponseCode;
    responseCallback( rc, std::move( response ) );
    break;
  default:
    responseCallback( ResponseCode::eFailure, {} );
    break;
  }
}


} // End of namespace url


} // End of namespace lb
