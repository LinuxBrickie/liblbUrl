#ifndef LIB_LB_URL_SENDERSIMPL_H
#define LIB_LB_URL_SENDERSIMPL_H

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

#include <lb/url/ws/SendResult.h>
#include <lb/url/ws/Senders.h>

#include <functional>
#include <mutex>


namespace lb
{


namespace url
{


namespace ws
{


struct Senders::Impl
{
  using DataSender  = std::function<SendResult( DataOpCode, const std::string& )>;
  using CloseSender = std::function<SendResult( encoding::websocket::CloseStatusCode
                                              , const std::string& )>;
  using PingSender  = std::function<SendResult( const std::string& )>;
  using PongSender = PingSender;

  static Senders create( DataSender ds, CloseSender cs
                       , PingSender pis, PongSender pos )
  {
    Senders senders;
    senders.d = std::make_shared<Senders::Impl>( std::move( ds )
                                               , std::move( cs )
                                               , std::move( pis )
                                               , std::move( pos ) );
    return senders;
  }

  static void close( Senders senders )
  {
    if ( senders.d )
    {
      senders.d->close();
    }
  }

  Impl( DataSender ds, CloseSender cs, PingSender pis, PongSender pos )
    : dataSender{ ds }
    , closeSender{ cs }
    , pingSender{ pis }
    , pongSender{ pos }
  {
  }

  SendResult sendData( DataOpCode opCode, const std::string& message ) const
  {
    std::scoped_lock l{ mutex };
    if ( dataSender )
    {
      return dataSender( opCode, message );
    }

    return SendResult::eClosed;
  }

  SendResult sendClose( encoding::websocket::CloseStatusCode code
                      , const std::string& reason ) const
  {
    std::scoped_lock l{ mutex };
    if ( closeSender )
    {
      return closeSender( code, reason );
    }

    return SendResult::eClosed;
  }

  SendResult sendPing( const std::string& payload ) const
  {
    std::scoped_lock l{ mutex };
    if ( pingSender )
    {
      return pingSender( payload );
    }

    return SendResult::eClosed;
  }

  SendResult sendPong( const std::string& payload ) const
  {
    std::scoped_lock l{ mutex };
    if ( pongSender )
    {
      return pongSender( payload );
    }

    return SendResult::eClosed;
  }

  /** \brief Called via Manager when WebSocketHandler can no longer service sends.

      This is not intended to be called by the Request maker (although it would
      be perfectly safe to do so).
   */
  void close()
  {
    std::scoped_lock l{ mutex };

    dataSender  = {};
    closeSender = {};
    pingSender  = {};
    pongSender  = {};
  }

  mutable std::mutex mutex;

  /** \brief An object for sending WebSocket data messages.

      Valid until a close control frame is either sent or received at which
      point \a close() should be invoked.
   */
  DataSender dataSender;

  /** \brief An object for sending a WebSocket close connection control frame.

      Sending a close control immediately suspends all sending capabilities.
   */
  CloseSender closeSender;

  /** \brief An object for sending a WebSocket ping control frame. */
  PingSender pingSender;

  /** \brief An object for sending a WebSocket pong control frame. */
  PongSender pongSender;
};


} // End of namespace ws


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_SENDERSIMPL_H
