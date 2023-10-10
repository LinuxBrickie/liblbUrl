#ifndef LIB_LB_URL_WSHANDLER_H
#define LIB_LB_URL_WSHANDLER_H

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

// Private header

#include <lb/url/ws/ConnectionID.h>
#include <lb/url/ws/Request.h>
#include <lb/url/ws/Response.h>
#include <lb/url/ws/Senders.h>

#include "RequestHandler.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>


namespace lb
{


namespace url
{


/** \brief The WebSocket handler.

    WebSocket is not an ideal fit for a URL fetching library, be it this one or
    libcurl, as it is really a two-way communication protocol that lives as long
    as is required. That said libcurl provides basic support so this class
    attempts to take advantage of that.

    Note that libcurl will handle WebSocket frames (also known as fragments) but
    no more than that. It it still our responsibility to concatentate frames
    together to re-assemble the whole message.

    NOTE: The libcurl docs currently say that it only supports frame sizes up to
    a maximum of 64K.

    Like other handlers, e.g. HttpHandler, this handler exists for the lifetime
    of the connection. However, since the WebSocket connection can be kept open
    for an arbitrary length of time this handler typically sticks around for a
    lot longer than others.

    The handler will poll for data when \a update() is called.
*/
class WebSocketHandler : public RequestHandler
{
public:
  WebSocketHandler( ws::Request r
                  , ws::Response::Callback c );
  ~WebSocketHandler();

  virtual Status respond( ResponseCode, std::string );

  /**
      Invoked periodically on the Requester thread. libcurl is only threadsafe
      if handles are not used across threads so we must do all sends and handle
      all received data here. This means that when the user tries to send data
      via ws::Senders we actually cache the data and dispatch it in this method.

      On a false return the handler should have already dealt with closing the
      connection. If the server initiated the close then the handler should send
      a close control frame before returning false and if the handler initiates
      the close then it should wait for the mirrored close control frame from
      the server before returning false.

      From RFC 6455:

      "As such, when a server is instructed to _Close the WebSocket Connection_
      it SHOULD initiate a TCP Close immediately, and when a client is instructed
      to do the same, it SHOULD wait for a TCP Close from the server.
   */
  virtual bool update();

  virtual bool close();

private:
  void processPendingSends();
  bool maybeAdvanceCloseHandshake();
  void processFrame( const curl_ws_frame& meta, const std::string& frame );
  bool receive();
  void discardPendingSends();

  // Note that these send methods are all bound to the \a senders object that
  // is passed back outside this instance. Calls to these may therefore be made
  // asynchronously to the public API.
  std::future<ws::SendResult> queueSendData( ws::DataOpCode
                                           , const std::string&
                                           , size_t maxFrameSize );
  std::future<ws::SendResult> queueSendClose( encoding::websocket::closestatus::PayloadCode code
                                            , const std::string& reason );
  std::future<ws::SendResult> queueSendPing( const std::string& payload );
  std::future<ws::SendResult> queueSendPong( const std::string& payload );

  ws::SendResult sendData( ws::DataOpCode, const std::string&, size_t maxFrameSize );
  ws::SendResult sendClose( encoding::websocket::closestatus::PayloadCode code
                          , const std::string& reason );
  ws::SendResult sendPing( const std::string& payload );
  ws::SendResult sendPong( const std::string& payload );

  // These three require no mutex protection.
  ws::ConnectionID connectionID;
  ws::Request request;
  ws::Response::Callback responseCallback;

  // Mutex to protect concurrent access to all remanining members.
  std::recursive_mutex mutex;

  // This is shared with the ws::Response object we pass to the response callback
  // We need to retain part ownership because we need to invalidate it if we go
  // away.
  ws::Senders senders;

  //! THe message currently being received (if split across multiple frames)
  std::string receivedMessage;

  // Note that there is no transition from eSergverInitiated to eComplete as we
  // have no way to detect it. They are, effectively, the same state.
  enum class CloseHandshake
  {
    eNone,
    eServerInitiated,
    eClientInitiated,
    eComplete
  };
  std::atomic<CloseHandshake> closeHandshake{ CloseHandshake::eNone };

  using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
  TimePoint closeSentTimePoint;
  // Re-try three times at most to the server if it initiates a close.
  int numRemainingCloseResponseAttempts = 3;
  // Cache these out in case we need to retry echoing the close control frame.
  encoding::websocket::closestatus::PayloadCode serverClosePayloadCode;
  std::string serverCloseReason;

  bool awaitingPong{ false };

  std::mutex pendingSendMutex;

  struct PendingSend
  {
    std::promise<ws::SendResult> sendResultPromise;

    // One and only one of these is ever set.
    std::optional<ws::DataOpCode> dataOpCode;
    std::optional<ws::ControlOpCode> controlOpCode;

    size_t maxFrameSize; //!< If sending data.

    encoding::websocket::closestatus::PayloadCode closePayloadCode;
    CloseHandshake handshake;

    //! For data this is the message, for a close it is the reason and for
    //! anything else it is the payload.
    std::string s;
  };
  std::vector<PendingSend> pendingSends;
};


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_WSHANDLER_H
