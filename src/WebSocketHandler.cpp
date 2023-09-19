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

#include <lb/encoding/websocket.h>

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
                     , std::placeholders::_2
                     , std::placeholders::_3 ),
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
  //TimePoint nowDebug{ std::chrono::steady_clock::now() };
  //std::cout << request.url << ": update( " << nowDebug.time_since_epoch().count() * double(std::chrono::steady_clock::period::num)/ std::chrono::steady_clock::period::den << ")" << std::endl;

  // Mutex scope
  {
    std::scoped_lock l{ mutex };

    // Test for time-out while awaiting a close confirmation
    switch ( closeHandshake )
    {
    case CloseHandshake::eNone:
      // Fine, carry on.
      break;
    case CloseHandshake::eClientInitiated:
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
      break;
    case CloseHandshake::eServerInitiated:
      // If we got here then the sendClose in the eNone clause in the switch in
      // processFrame failed.
      if ( sendClose( serverClosePayloadCode, serverCloseReason ) == ws::SendResult::eSuccess )
      {
        // Just go straight to complete as there is nothing more we can do as
        // there is no confirmation of our echoing of the close control frame.
        closeHandshake = CloseHandshake::eComplete;
      }
      --numRemainingCloseResponseAttempts;
      if ( numRemainingCloseResponseAttempts == 0 )
      {
        std::cerr << "Failed to send close control echo back to server. Closing anyway." << std::endl;
        closeHandshake = CloseHandshake::eComplete;
        return false;
      }
      break;
    case CloseHandshake::eComplete:
      // Should never get here.
      return false;
    }

    // May not receive a full message, only a frame. If the message fits in the
    // one frame then we dispatch it right away, otherwise it gets built up in
    // receivedMessage and dispatched on the last frame.
    //
    // We might not even get a full frame. We have to keep calling curl_ws_recv
    // until either the number of bytes received is less than the buffer size or
    // we get CURLE_GOT_NOTHING.
    const curl_ws_frame* meta{ nullptr };
    std::string frame;
    bool frameIncomplete{ true };

    int loop = 0;
    while( frameIncomplete )
    {
      size_t numBytesReceived;
      const size_t BUFFER_SIZE{ 256 };
      char buffer[ BUFFER_SIZE ];

      const CURLcode result = curl_ws_recv( easyHandle, buffer, sizeof(buffer), &numBytesReceived, &meta );

      switch( result )
      {
      case CURLE_OK:
      {
        //std::cout << request.url << "  " << loop << " :CURLE_OK: " << numBytesReceived << " bytes." << std::endl;

        frame.append( buffer, numBytesReceived );
        if ( numBytesReceived < BUFFER_SIZE )
        {
          frameIncomplete = false;
        }
        break;
      }
      case CURLE_GOT_NOTHING:
        // This means the connection is closed.
        //std::cout << request.url << "  " << loop << " :CURLE_GOT_NOTHING" << std::endl;
        return false;
      case CURLE_AGAIN:
        //std::cout << request.url << "  " << loop << " :CURLE_EAGAIN" << std::endl;
        // Fine, just means there is no data to receive.
        return true;
      default:
        ws::Senders::Impl::close( senders );
        std::cerr << "curl_ws_recv error: " << curl_easy_strerror( result ) << std::endl;
        return false;
      }

      ++loop;
    }

    if ( meta )
    {
      processFrame( *meta, frame );
    }
  } // End of mutex scope

  // Do this after our mutex is unlocked as there could be threads who have
  // called into senders (thereby locking its mutex) and are waiting for our
  // mutex in one of our send methods.
  if ( closeHandshake != CloseHandshake::eNone )
  {
    ws::Senders::Impl::close( senders );
  }

  return closeHandshake != CloseHandshake::eComplete;
}

bool WebSocketHandler::close()
{
  std::scoped_lock l{ mutex };

  if ( closeHandshake == CloseHandshake::eNone )
  {
    sendClose( encoding::websocket::closestatus::toPayload(
                 encoding::websocket::closestatus::ProtocolCode::eGoingAway )
             , "Client shutdown" );
  }

  return true;
}

void WebSocketHandler::processFrame( const curl_ws_frame& meta
                                   , const std::string& payload )
{
  if ( meta.flags & CURLWS_TEXT )
  {
    //if ( FIN bit set )
    //{
    if ( !request.receivers.receiveData( connectionID
                                       , ws::DataOpCode::eText
                                       , payload ) )
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
    std::cerr << "Binary frames not yet supported!" << std::endl;

    sendClose( encoding::websocket::closestatus::toPayload(
                 encoding::websocket::closestatus::ProtocolCode::eUnacceptableData )
             , "Cannot send binary data (yet)." );
  }
  else if ( meta.flags & CURLWS_CONT )
  {
    receivedMessage.append( payload );
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
    //std::cout << "   received CLOSE" << std::endl;

    // Even if we are awaiting a close confirmation we still pass out the
    // notification here as it could be useful.
    if ( !request.receivers.receiveControl( connectionID
                                          , ws::ControlOpCode::eClose
                                          , payload ) )
    {
      std::cout << "Receiver no longer receiving control." << std::endl;
    }

    switch ( closeHandshake )
    {
    case CloseHandshake::eNone:
    {
      // If the code is the "absent" code what do we send back? - TODO
      serverClosePayloadCode = encoding::websocket::closestatus::decodePayloadCode( payload );
      serverCloseReason = payload.substr( 2 );
      if ( sendClose( serverClosePayloadCode, serverCloseReason ) == ws::SendResult::eSuccess )
      {
        // Just go straight to complete as there is nothing more we can do as
        // there is no confirmation of our echoing of the close control frame.
        closeHandshake = CloseHandshake::eComplete;
      }
      else
      {
        // Try again on next update()
        closeHandshake = CloseHandshake::eServerInitiated;
      }
      break;
    }
    case CloseHandshake::eServerInitiated:
      // Should never get here.
      break;
    case CloseHandshake::eClientInitiated:
      // Assume this is the response. TODO - double check payload to ensure
      // that it matches.
      closeHandshake = CloseHandshake::eComplete;
      break;
    case CloseHandshake::eComplete:
      // Ignore, we already sent a response.
      break;
    }
  }
  else if ( meta.flags & CURLWS_PING )
  {
    //std::cout << "Received PING frame!" << std::endl;

    if ( !request.receivers.receiveControl( connectionID
                                          , ws::ControlOpCode::ePing
                                          , payload ) )
    {
      std::cout << "Receiver no longer receiving control." << std::endl;
    }

    sendPong( payload );
  }
  else if ( meta.flags & CURLWS_PONG )
  {
    if ( awaitingPong )
    {
      //std::cout << "Received PONG." << std::endl;
      if ( !request.receivers.receiveControl( connectionID
                                            , ws::ControlOpCode::ePong
                                            , payload ) )
      {
        std::cout << "Receiver no longer receiving control." << std::endl;
      }
      awaitingPong = false;
    }
    else
    {
      std::cout << "Received unsolicited PONG." << std::endl;
    }
  }
}

ws::SendResult WebSocketHandler::sendData( ws::DataOpCode opCode
                                         , const std::string& message
                                         , size_t maxFrameSize )
{
  std::scoped_lock l{ mutex };

  // Should not be necessary as we close the Senders::Impl but does no harm.
  if ( closeHandshake != CloseHandshake::eNone )
  {
    return ws::SendResult::eClosed;
  }

  size_t numBytesSent{};
  size_t numPayloadBytesToSend{ message.size() };
  size_t numPayloadBytesRemaining{ message.size() };

  // libcurl calls these "flags" and the CURLWS_ values are bit masks which is
  // completely misleading as they are *not* to be ORed together.
  //
  // TODO - There is no correct combination of flags for curl_ws_send for
  // fragmented messages and any attempt to OR in CURLWS_CONT immediately
  // gives a send error! For now the maxFrameSize > 0 code is a placeholder
  // until the issues with libcurl can be resolved.
  int flags
  {
    ( ( opCode == ws::DataOpCode::eText ) ? CURLWS_TEXT : CURLWS_BINARY )
  };

  const char* p{ message.c_str() };
  if ( maxFrameSize != ws::Senders::UNLIMITED_FRAME_SIZE )
  {
    std::cerr << "Sending fragmented messages not currently possible" << std::endl;
//    const auto maxEncodedHeaderSize
//    {
//      encoding::websocket::Header::encodedSizeInBytes( numPayloadBytesRemaining, true )
//    };
//    if ( maxFrameSize <= maxEncodedHeaderSize )
//    {
//      return ws::SendResult::eFailure;
//    }
//    while ( numPayloadBytesRemaining + maxEncodedHeaderSize > maxFrameSize )
//    {
//      // First or Continuation (if any)
//      numPayloadBytesToSend = maxFrameSize - maxEncodedHeaderSize;

//      //std::cout << "Sending frame..." << std::endl;
//      const CURLcode result
//      {
//        curl_ws_send( easyHandle
//                    , p
//                    , numPayloadBytesToSend
//                    , &numBytesSent
//                    , 0
//                    , flags )
//      };
//      flags |= CURLWS_CONT;
//      numPayloadBytesRemaining -= numPayloadBytesToSend;
//      p += numPayloadBytesToSend;
//    }
  }

  const CURLcode result
  {
    curl_ws_send( easyHandle
                , p
                , numPayloadBytesToSend
                , &numBytesSent
                , 0
                , flags )
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
  if ( closeHandshake != CloseHandshake::eNone )
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
    closeHandshake = CloseHandshake::eClientInitiated;
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
  if ( closeHandshake != CloseHandshake::eNone )
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
  if ( closeHandshake != CloseHandshake::eNone )
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
