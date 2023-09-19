#ifndef LIB_LB_URL_WS_SENDINTERFACE_H
#define LIB_LB_URL_WS_SENDINTERFACE_H

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

#include <memory>
#include <string>

#include <lb/encoding/websocket.h>
#include <lb/url/ws/DataOpCode.h>
#include <lb/url/ws/SendResult.h>


namespace lb
{


namespace url
{


namespace ws
{


/** \brief The means of writing to the WebSocket.

    This is provided *to* the request maker in the \a Response object.

    If the accompanying \a ResponseCode returned by \a Response::Callback is
    anything other than eSuccess then this object will be default constructed.
 */
class Senders
{
public:
  /** \brief Create an invalid object. Both send methods will immediately return eNoImplementation. */
  Senders() = default;
  Senders( Senders&& ) = default;
  Senders& operator=( Senders&& ) = default;
  Senders( const Senders& ) = default;
  Senders& operator=( const Senders& ) = default;

  static const size_t UNLIMITED_FRAME_SIZE;

  /**
      \brief Send text or binary data to the WebSocket.
      \param dataOpCode Whether the message is text or binary.
      \param message The WebSocket frame payload.
      \param maxFrameSize Maximum frame size. Zero implies unlimited.

      If a frame's size exceeds \a maxFrameSize then the server will split the
      frame up into multiple frames and send a fragmented message.
   */
  SendResult sendData( DataOpCode, std::string message, size_t maxFrameSize = UNLIMITED_FRAME_SIZE ) const;

  SendResult sendClose( encoding::websocket::closestatus::PayloadCode, std::string reason = {} ) const;
  SendResult sendPing( std::string payload ) const;
  SendResult sendPong( std::string payload ) const;

  struct Impl; //!< Opaque implementation detail.

private:
  std::shared_ptr<Impl> d;
};


} // End of namespace ws


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_WS_SENDINTERFACE_H
