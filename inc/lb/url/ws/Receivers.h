#ifndef LIB_LB_URL_WS_RECEIVEINTERFACE_H
#define LIB_LB_URL_WS_RECEIVEINTERFACE_H

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

#include <functional>
#include <memory>
#include <string>

#include <lb/url/ws/ConnectionID.h>
#include <lb/url/ws/ControlOpCode.h>
#include <lb/url/ws/DataOpCode.h>


namespace lb
{


namespace url
{


namespace ws
{


/** \brief The means of receiving from the WebSocket.

    This is provided *by* the request maker in the \a Request object.

    Fragmented (data) messages are reassembled by Requester so that what you
    receive via \a receiveData is the complete message, you do not get access
    to the individual frames. WARNING: That said libcurl curently has a bug
    that means that fragmented messages do not work.

    Control messages are never fragmented so you receive the payload of the
    control frame directly in \a receiveControl. Note that control messages
    are for your information only and do not need replied to as they will be
    handled for you appropriately. Indeed in the case of a connection close
    control frame you will not be able to send anything back as the \a Senders
    will have been closed off to further sends.
 */
class Receivers
{
public:
  using DataReceiver    = std::function<void( ConnectionID, DataOpCode, const std::string& )>;
  using ControlReceiver = std::function<void( ConnectionID, ControlOpCode, const std::string& )>;

  Receivers( DataReceiver, ControlReceiver );

  Receivers( Receivers&& ) = default;
  Receivers& operator=( Receivers&& ) = default;
  Receivers( const Receivers& ) = default;
  Receivers& operator=( const Receivers& ) = default;

  bool receiveData( ConnectionID id, DataOpCode opCode, std::string message );

  bool receiveControl( ConnectionID id, ControlOpCode opCode, std::string payload );

  /** \brief Called by the request maker when it can no longer receive anything.

      This is intended to be used when the functions passed in to the
      constructor are no longer safe to call.

      This is not intended to be called by Requester (although it would be
      perfectly safe to do so).
   */
  void stopReceiving();

private:
  struct Impl;
  std::shared_ptr<Impl> d;
};


} // End of namespace ws


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_WS_RECEIVEINTERFACE_H
