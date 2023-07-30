#ifndef LIB_LB_URL_WS_REQUEST_H
#define LIB_LB_URL_WS_REQUEST_H

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
#include <string>

#include <lb/url/ws/Receivers.h>


namespace lb
{


namespace url
{


namespace ws
{


struct Request
{
  Request() = default;

  Request( std::string url
         , Receivers::DataReceiver dr
         , Receivers::ControlReceiver cr
         , size_t closeTimeoutMilliseconds = 2000 )
    : url{ url }
    , receivers{ std::move( dr ), std::move( cr ) }
    , closeTimeoutMilliseconds{ closeTimeoutMilliseconds }
  {
  }

  // Default move/copy construction and assignment.
  Request( Request&& ) = default;
  Request& operator=( Request&& ) = default;
  Request( const Request& ) = default;
  Request& operator=( const Request& ) = default;

  std::string url;

  /** \brief The interface for receiving from the WebSocket */
  Receivers receivers;

  size_t closeTimeoutMilliseconds{ 2000 };
};


} // End of namespace ws


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_WS_REQUEST_H
