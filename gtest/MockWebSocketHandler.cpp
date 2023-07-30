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

#include "MockWebSocketHandler.h"

#include "TestWsRequester.h"

#include <lb/encoding/websocket.h>

#include <iostream>


struct WSInfo
{
  std::string url;
  lb::httpd::ws::Senders dataSender;
};
using WSInfoLookup = std::unordered_map< lb::httpd::ws::ConnectionID, WSInfo >;
WSInfoLookup wsInfoLookup;

bool mockIsHandled( const std::string& urlPath )
{
  const std::string invalid{ "invalid" };
  return ( ( urlPath.size() >= invalid.size() )
        && ( urlPath.substr( urlPath.size() - invalid.size() ) != invalid ) );
}

void dataReceiver( lb::httpd::ws::ConnectionID id, std::string data )
{
  std::string response;

  const auto I{ wsInfoLookup.find( id ) };
  if ( I != wsInfoLookup.end() )
  {
    WSInfo& wsInfo{ I->second };

    const auto U{ WSExpectedMockResponses.find( wsInfo.url ) };
    if ( U != WSExpectedMockResponses.end() )
    {
      const auto R{ U->second.responseMessages.find( data ) };
      if ( R != U->second.responseMessages.end() )
      {
        response = R->second;

        if ( data == sendControlCloseMessage )
        {
          const lb::httpd::ws::SendResult result
          {
            wsInfo.dataSender.sendClose( lb::encoding::websocket::CloseStatusCode::eNormal
                                       , response )
          };
          switch ( result )
          {
          case lb::httpd::ws::SendResult::eSuccess:
            break;
          default:
            std::cerr << "Failed to send close frame!" << std::endl;
            break;
          }
          return;
        }
      }
      else
      {
        response = "Invalid test challenge";
      }
    }
    else
    {
      response = "Invalid test URL";
    }

    const lb::httpd::ws::SendResult result{ wsInfo.dataSender.sendData( std::move( response ), 0 ) };
    switch ( result )
    {
    case lb::httpd::ws::SendResult::eSuccess:
      break;
    default:
      std::cerr << "Failed to send data frame!" << std::endl;
      break;
    }
  }
  else
  {
    // Can't send response as we have no DataSender.
    std::cerr << "Unrecognised WebSocket connection ID" << std::endl;
  }
}

void controlReceiver( lb::httpd::ws::ConnectionID id
                    , lb::httpd::ws::Receivers::ControlOpCode opCode
                    , std::string payload )
{
  // Left as a no-op since Server will handle all ctronol frames for us, this
  // callback is just a notification that one was received, there is nothing
  // expected from us in here.
}

lb::httpd::ws::Receivers mockConnectionEstablished( WSH::Connection connection )
{
  wsInfoLookup[ connection.id ] = { connection.url, connection.senders };
  return { dataReceiver, controlReceiver };
}


WSH mockWSHandler
{
  mockIsHandled,
  mockConnectionEstablished
};
