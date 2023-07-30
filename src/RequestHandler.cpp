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

#include "RequestHandler.h"

#include "HttpHandler.h"

#include <stdexcept>


namespace lb
{


namespace url
{


RequestHandler::RequestHandler()
  : easyHandle{ curl_easy_init() }
{
  if ( !easyHandle )
  {
    throw std::runtime_error( "Failed to create curl easy handle." );
  }

  curl_easy_setopt( easyHandle, CURLOPT_WRITEFUNCTION, &writeCallback );
  curl_easy_setopt( easyHandle, CURLOPT_WRITEDATA, this );
  curl_easy_setopt( easyHandle, CURLOPT_VERBOSE, 0 );
}

RequestHandler::RequestHandler( RequestHandler&& moveFrom )
  : easyHandle{ moveFrom.easyHandle }
{
  moveFrom.easyHandle = nullptr;
  curl_easy_setopt( easyHandle, CURLOPT_WRITEDATA, this );
}

RequestHandler::~RequestHandler()
{
  if ( easyHandle )
  {
    curl_easy_cleanup( easyHandle );
  }
}

CURL* RequestHandler::getHandle() const
{
  return easyHandle;
}

void RequestHandler::processInfo()
{
}

RequestHandler::Status RequestHandler::respond( ResponseCode rc )
{
  switch ( rc )
  {
  case ResponseCode::eSuccess:
    return respond( ResponseCode::eSuccess, std::move( receivedData ) );
  default:
    return respond( rc, {} );
  }
}

bool RequestHandler::updatePersisting()
{
  return update();
}

bool RequestHandler::closePersisting()
{
  return close();
}

bool RequestHandler::update()
{
  // Do nothing
  return true;
}

bool RequestHandler::close()
{
  // Do nothing
  return true;
}

// static
size_t RequestHandler::writeCallback( char* data, size_t size, size_t numBytes, void* userData )
{
  ((RequestHandler*)(userData))->processReceivedData( data, numBytes );
  return numBytes;
}

void RequestHandler::processReceivedData( const char* data, size_t numBytes )
{
  receivedData.append( data, numBytes );
}


} // End of namespace url


} // End of namespace lb

