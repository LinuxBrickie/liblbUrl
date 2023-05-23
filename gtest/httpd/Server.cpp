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

#include "Server.h"

#include <cstring>
#include <iostream>


namespace httpd
{


static
Server::Method parseMethod( const char* methodStr )
{
  if ( strcmp( methodStr, "GET" ) == 0 )
  {
    return Server::Method::eGet;
  }
  else if ( strcmp( methodStr, "HEAD" ) == 0  )
  {
    return Server::Method::eHead;
  }
  else if ( strcmp( methodStr, "POST" ) == 0  )
  {
    return Server::Method::ePost;
  }
  else if ( strcmp( methodStr, "PUT" ) == 0  )
  {
    return Server::Method::ePut;
  }
  else if ( strcmp( methodStr, "DELETE" ) == 0  )
  {
    return Server::Method::eDelete;
  }

  return Server::Method::eInvalid;
}

static
Server::Version parseVersion( const char* versionStr )
{
  if ( versionStr == "HTTP/0.9" )
  {
    return Server::Version::e0_9;
  }
  else if ( versionStr == "HTTP/1.0" )
  {
    return Server::Version::e1_0;
  }
  else if ( versionStr == "HTTP/1.1" )
  {
    return Server::Version::e1_1;
  }
  else if ( versionStr == "HTTP/2.0" )
  {
    return Server::Version::e2_0;
  }

  return Server::Version::eUnknown;
}


Server::Server( int port , RequestHandler rh )
  : mhd{ MHD_start_daemon( MHD_USE_INTERNAL_POLLING_THREAD
                         , port
                         , nullptr // accept policy callback not required
                         , nullptr // accept policy callback user data
                         , &accessHandlerCallback
                         , this
                         , MHD_OPTION_END ) }
  , requestHandler{ rh }
{
}

Server::~Server()
{
  MHD_stop_daemon( mhd );
}

// static
MHD_Result Server::accessHandlerCallback( void* userData
                                        , struct MHD_Connection* connection
                                        , const char* url
                                        , const char* method
                                        , const char* version
                                        , const char* uploadData
                                        , size_t* uploadDataSize
                                        , void** con_cls )
{
  auto server = (Server*)userData;

//  std::cout << "URL:              " << url << '\n';
//  std::cout << "Method:           " << method << '\n';
//  std::cout << "Version:          " << version << '\n';
//  std::cout << "Upload data size: " << *uploadDataSize << '\n';
//  std::cout << "Upload data:      ";
//  if ( uploadData )
//  {
//    std::cout << uploadData << '\n';
//  }
//  else
//  {
//    std::cout << "<null>\n";
//  }
//  std::cout << std::flush;

  const auto response
  {
    server->invokeRequestHandler( std::move( url )
                                , parseMethod( method )
                                , parseVersion( version )
                                , std::move( std::string{ uploadData
                                                        , *uploadDataSize } ) )
  };

  MHD_Response*const mhdResponse
  {
    MHD_create_response_from_buffer( response.content.size()
                                   , (void*)response.content.c_str()
                                   , MHD_RESPMEM_MUST_COPY )
  };

  const auto result
  {
    MHD_queue_response( connection, response.code, mhdResponse )
  };

  MHD_destroy_response( mhdResponse );

  return result;
}

Server::Response Server::invokeRequestHandler( std::string url
                                             , Method method
                                             , Version version
                                             , std::string payload )
{
  return requestHandler( std::move( url ), method, version, std::move( payload ) );
}


} // End of namespace httpd
