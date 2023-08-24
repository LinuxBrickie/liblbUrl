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

#include "WebSocketHandler.h"
#include "ws/SendersImpl.h"

#include <iostream>


namespace lb
{


namespace url
{


// Library-wide counter i.e. shared by all Requester instances effectively.
ws::ConnectionID globalConnectionID{ 0 };


WebSocketHandler::WebSocketHandler( ws::Request r
                                  , ws::Response::Callback c )
  : connectionID{ globalConnectionID++ }
  , request{ std::move( r ) }
  , responseCallback{ std::move( c ) }
{
  curl_easy_setopt( easyHandle, CURLOPT_HTTPGET, 1L );
  curl_easy_setopt( easyHandle, CURLOPT_URL, request.url.c_str() );

  // Set this so that we don't get anything through the write callback and
  // instead use curl_ws_recv. Either way we still send using curl_ws_send.
  curl_easy_setopt( easyHandle, CURLOPT_CONNECT_ONLY, 2L);
}

WebSocketHandler::~WebSocketHandler()
{
}

RequestHandler::Status WebSocketHandler::respond( ResponseCode rc
                                                , std::string receivedData )
{
  long httpResponseCode;
  const CURLcode cc{ curl_easy_getinfo( easyHandle, CURLINFO_RESPONSE_CODE, &httpResponseCode ) };
  switch( cc )
  {
  case CURLE_OK:
    if ( httpResponseCode != 101 ) // server refused the upgrade
    {
      responseCallback( ResponseCode::eFailure, {} );
    }
    else
    {
      if ( rc == ResponseCode::eSuccess )
      {
        senders
          = ws::Senders::Impl::create(
            std::bind( &WebSocketHandler::sendData
                     , this
                     , std::placeholders::_1
                     , std::placeholders::_2 ),
            std::bind( &WebSocketHandler::sendClose
                     , this
                     , std::placeholders::_1,
                       std::placeholders::_2 ),
            std::bind( &WebSocketHandler::sendPing
                     , this
                     , std::placeholders::_1 ),
            std::bind( &WebSocketHandler::sendPong
                     , this
                     , std::placeholders::_1 ) );

        responseCallback( ResponseCode::eSuccess
                        , {
                            connectionID,
                            senders
                          } );
        return Status::ePersisting;
      }
      else
      {
        responseCallback( rc, {} );
      }
    }
    break;
  default:
    responseCallback( ResponseCode::eFailure, {} );
    break;
  }

  return Status::eFinished;
}

void print( const curl_ws_frame& f )
{
// CURLWS_TEXT       (1<<0)
// CURLWS_BINARY     (1<<1)
// CURLWS_CONT       (1<<2)
// CURLWS_CLOSE      (1<<3)
// CURLWS_PING       (1<<4)
// CURLWS_OFFSET     (1<<5)
  std::cout << "Frame meta:" << std::endl;
  std::cout << "       age:" << f.age << std::endl;
  std::cout << "     flags:" << f.flags << std::endl;
  std::cout << "    offset:" << f.offset << std::endl;
  std::cout << "bytes left:" << f.bytesleft << std::endl;
  std::cout << "       len:" << f.len << std::endl;
}

bool WebSocketHandler::update()
{
  std::scoped_lock l{ mutex };

  // Test for time-out while awaiting a close confirmation
  if ( awaitingCloseConfirmation )
  {
    TimePoint now{ std::chrono::steady_clock::now() };
    const auto diffMilliSeconds{ std::chrono::duration_cast<std::chrono::milliseconds>( now - closeSentTimePoint ) };
    if ( diffMilliSeconds.count() > request.closeTimeoutMilliseconds )
    {
      std::cerr << "No close confirmation received within " << request.closeTimeoutMilliseconds
                << " milliseconds, destroying WebSocketHandler." << std::endl;
      return false;
    }
  }

  // May not receive a full message, only a frame. If the message fits in the
  // one frame then we dispatch it right away, otherwise it gets built up in
  // receivedMessage and dispatched on the last frame.
  //
  // We might not even get a full frame. We have to keep calling curl_ws_recv
  // until either the number of bytes received is less than the buffer size or
  // we get CURLE_GOT_NOTHING.
  curl_ws_frame* meta{ nullptr };
  std::string frame;
  bool frameIncomplete{ true };

  while( frameIncomplete )
  {
    size_t numBytesReceived;
    const size_t BUFFER_SIZE{ 256 };
    char buffer[ BUFFER_SIZE ];

    const CURLcode result = curl_ws_recv( easyHandle, buffer, sizeof(buffer), &numBytesReceived, &meta );

    switch( result )
    {
    case CURLE_OK:
      frame.append( buffer, numBytesReceived );
      if ( numBytesReceived < BUFFER_SIZE )
      {
        frameIncomplete = false;
      }
      break;
    case CURLE_GOT_NOTHING:
      frameIncomplete = false;
      break;
    case CURLE_AGAIN:
      // Fine, just means there is no data to receive.
      return true;
    default:
      ws::Senders::Impl::close( senders );
      std::cerr << "curl_ws_recv error: " << curl_easy_strerror( result ) << std::endl;
      return false;
    }
  }

  processFrame( *meta, frame );

  return true;
}

bool WebSocketHandler::close()
{
  std::scoped_lock l{ mutex };

  if ( !awaitingCloseConfirmation )
  {
    sendClose( encoding::websocket::closestatus::toPayload(
                 encoding::websocket::closestatus::ProtocolCode::eGoingAway )
             , "Client shutdown" );
  }

  return true;
}

bool WebSocketHandler::processFrame( curl_ws_frame& meta
                                   , const std::string& frame )
{
  if ( meta.flags & CURLWS_TEXT )
  {
    //if ( FIN bit set )
    //{
    if ( !request.receivers.receiveData( connectionID
                                       , ws::DataOpCode::eText
                                       , frame ) )
    {
      std::cout << "Receiver no longer receiving data." << std::endl;
    }
    //}
    //else
    //{
    //  receivedMessage.append( message, numMessageBytes );
    //}
  }
  else if ( meta.flags & CURLWS_BINARY )
  {
    // In practice they may be supported, just not tested. TODO
    ws::Senders::Impl::close( senders );
    std::cerr << "Binary frames not yet supported!" << std::endl;
    return false;
  }
  else if ( meta.flags & CURLWS_CONT )
  {
    receivedMessage.append( frame );
    //if ( FIN bit set )
    //{
    //  std::string data;
    //  data.swap( receivedMessage );
    //  if ( !request.receivers.receiveData( connectionID
    //                                     , ws::DataOpCode::eText
    //                                     , std::move( data ) ) )
    //  {
    //    std::cout << "Receiver no longer receiving data." << std::endl;
    //  }
    //}
  }
  else if ( meta.flags & CURLWS_CLOSE )
  {
    std::string payload{ frame };

    if ( awaitingCloseConfirmation )
    {
      // Close initiated at this end so clearly the user knows we are going
      // away but no harm in filtering the confirmation up.
      awaitingCloseConfirmation = false;
    }
    else
    {
      const auto payloadCode
      {
        encoding::websocket::closestatus::decodePayloadCode( payload )
      };
      // If the code is the "absent" code what do we send back? - TODO
      senders.sendClose( payloadCode, payload.substr( 2 ) );
    }
    if ( !request.receivers.receiveControl( connectionID
                                          , ws::ControlOpCode::eClose
                                          , std::move( payload ) ) )
    {
      std::cout << "Receiver no longer receiving control." << std::endl;
    }
    ws::Senders::Impl::close( senders );
    return false;
  }
  else if ( meta.flags & CURLWS_PING )
  {
    std::cout << "Received PING frame!" << std::endl;
    senders.sendPong( frame );
  }
  else if ( meta.flags & CURLWS_PONG )
  {
    if ( awaitingPong )
    {
      std::cout << "Received PONG." << std::endl;
      awaitingPong = false;
    }
    else
    {
      std::cout << "Received unsolicited PONG." << std::endl;
    }
  }

  return true;
}

ws::SendResult WebSocketHandler::sendData( ws::DataOpCode opCode
                                         , const std::string& message )
{
  std::scoped_lock l{ mutex };

  // Should not be necessary as we close the Senders::Impl but does no harm.
  if ( awaitingCloseConfirmation )
  {
    return ws::SendResult::eClosed;
  }

  size_t numBytesSent{ 0 };

  // If this needs to be multiple calls then set the CURLWS_OFFSET bit and on
  // the first call the second last argument should be the total size we are
  // sendinf across all calls.
  const CURLcode result
  {
    curl_ws_send( easyHandle
                , message.c_str()
                , message.size()
                , &numBytesSent
                , 0
                , ( ( opCode == ws::DataOpCode::eText ) ? CURLWS_TEXT : CURLWS_BINARY ) )
  };

  switch( result )
  {
  case CURLE_OK:
    break;
  default:
    std::cerr << curl_easy_strerror( result ) << std::endl;
    return ws::SendResult::eFailure;
  }

  return ws::SendResult::eSuccess;
}

ws::SendResult WebSocketHandler::sendClose( encoding::websocket::closestatus::PayloadCode code
                                          , const std::string& reason )
{
  std::scoped_lock l{ mutex };

  // Should not be necessary as we close the Senders::Impl but does no harm.
  if ( awaitingCloseConfirmation )
  {
    return ws::SendResult::eClosed;
  }

  size_t numBytesSent{ 0 };

  std::string payload{ "\0\0", 2 };
  encoding::websocket::closestatus::encodePayloadCode( code, payload );
  payload.append( reason );

  const CURLcode result
  {
    curl_ws_send( easyHandle
                , payload.c_str()
                , payload.size()
                , &numBytesSent
                , 0
                , CURLWS_CLOSE )
  };

  switch( result )
  {
  case CURLE_OK:
    awaitingCloseConfirmation = true;
    closeSentTimePoint = std::chrono::steady_clock::now();
    break;
  default:
    std::cerr << curl_easy_strerror( result ) << std::endl;
    return ws::SendResult::eFailure;
  }
  return ws::SendResult::eSuccess;
}

ws::SendResult WebSocketHandler::sendPing( const std::string &payload )
{
  std::scoped_lock l{ mutex };

  // Should not be necessary as we close the Senders::Impl but does no harm.
  if ( awaitingCloseConfirmation )
  {
    return ws::SendResult::eClosed;
  }

  size_t numBytesSent{ 0 };

  if ( awaitingPong )
  {
    std::cout << "No pong from last ping, sending new ping." << std::endl;
  }

  const CURLcode result
  {
    curl_ws_send( easyHandle
                , payload.c_str()
                , payload.size()
                , &numBytesSent
                , 0
                , CURLWS_PING )
  };
  switch( result )
  {
  case CURLE_OK:
    awaitingPong = true;
    break;
  default:
    std::cerr << curl_easy_strerror( result ) << std::endl;
    return ws::SendResult::eFailure;
  }

  return ws::SendResult::eSuccess;
}

ws::SendResult WebSocketHandler::sendPong( const std::string &payload )
{
  std::scoped_lock l{ mutex };

  // Should not be necessary as we close the Senders::Impl but does no harm.
  if ( awaitingCloseConfirmation )
  {
    return ws::SendResult::eClosed;
  }

  size_t numBytesSent{ 0 };

  const CURLcode result
  {
    curl_ws_send( easyHandle
                , payload.c_str()
                , payload.size()
                , &numBytesSent
                , 0
                , CURLWS_PONG )
  };
  switch( result )
  {
  case CURLE_OK:
    break;
  default:
    std::cerr << curl_easy_strerror( result ) << std::endl;
    return ws::SendResult::eFailure;
  }

  return ws::SendResult::eSuccess;
}


} // End of namespace url


} // End of namespace lb
