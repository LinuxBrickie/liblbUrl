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

#include <lb/url/ws/Senders.h>

#include "SendersImpl.h"


namespace lb
{


namespace url
{


namespace ws
{


const size_t Senders::UNLIMITED_FRAME_SIZE{ 0 };


std::future<SendResult> Senders::sendData( DataOpCode opCode
                                         , std::string message
                                         , size_t maxFrameSize ) const
{
  if ( d )
  {
    return d->sendData( opCode, message, maxFrameSize );
  }
  else
  {
    std::promise<SendResult> promise;
    promise.set_value( SendResult::eNoImplementation );
    return promise.get_future();
  }
}

std::future<SendResult>
Senders::sendClose( encoding::websocket::closestatus::PayloadCode code
                  , std::string reason ) const
{
  if ( d )
  {
    return d->sendClose( code, reason );
  }
  else
  {
    std::promise<SendResult> promise;
    promise.set_value( SendResult::eNoImplementation );
    return promise.get_future();
  }
}

std::future<SendResult> Senders::sendPing( std::string payload ) const
{
  if ( d )
  {
    return d->sendPing( payload );
  }
  else
  {
    std::promise<SendResult> promise;
    promise.set_value( SendResult::eNoImplementation );
    return promise.get_future();
  }
}

std::future<SendResult> Senders::sendPong( std::string payload ) const
{
  if ( d )
  {
    return d->sendPong( payload );
  }
  else
  {
    std::promise<SendResult> promise;
    promise.set_value( SendResult::eNoImplementation );
    return promise.get_future();
  }
}


} // End of namespace ws


} // End of namespace url


} // End of namespace lb
