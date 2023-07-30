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

#include <lb/url/ws/Receivers.h>

#include "ReceiversImpl.h"


namespace lb
{


namespace url
{


namespace ws
{


Receivers::Receivers( DataReceiver ds, ControlReceiver cs )
  : d{ std::make_shared<Impl>( std::move( ds ), std::move( cs ) ) }
{
}


bool Receivers::receiveData( ConnectionID id, DataOpCode opCode, std::string message )
{
  std::scoped_lock l{ d->mutex };
  if ( d->dataReceiver )
  {
    d->dataReceiver( id, opCode, message );
    return true;
  }

  return false;
}

bool Receivers::receiveControl( ConnectionID id, ControlOpCode opCode, std::string payload )
{
  std::scoped_lock l{ d->mutex };
  if ( d->controlReceiver )
  {
    d->controlReceiver( id, opCode, payload );
    return true;
  }

  return false;
}

void Receivers::stopReceiving()
{
  std::scoped_lock l{ d->mutex };

  d->dataReceiver = {};
  d->controlReceiver = {};
}


} // End of namespace ws


} // End of namespace url


} // End of namespace lb
