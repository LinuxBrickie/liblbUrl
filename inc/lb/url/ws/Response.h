#ifndef LIB_LB_URL_WS_RESPONSE_H
#define LIB_LB_URL_WS_RESPONSE_H

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

#include <lb/url/ResponseCode.h>
#include <lb/url/ws/ConnectionID.h>
#include <lb/url/ws/Senders.h>

#include <functional>


namespace lb
{


namespace url
{


namespace ws
{


/**
    \brief Response to the initial WebSocket HTTP(S) GET request.

    If the GET request is successful then the connection is upgraded to a
    two-way WebSocket connection.  This \a Response object provides the means
    to write data to the WebSocket (the \a Request object is where the request
    maker registers the means of reading data from the WebSocket).
 */
struct Response
{
  Response() = default;

  // Default move construction and move assignment. Copy forbidden.
  Response( Response&& ) = default;
  Response& operator=( Response&& ) = default;
  Response( const Response& ) = delete;
  Response& operator=( const Response& ) = delete;

  using Callback = std::function< void(ResponseCode, Response) >;

  /** \brief The unique identifier for the connection.

      This is passes to the ReceiveInterface callbacks so that the messages can
      be identified with a particular WebSocket connection.

      There is no need to specify the ID when sending data as the \a senders
      implementation knows which WebSocket connection it is associated with.
   */
  ConnectionID connectionID;

  /** \brief The interface for sending to the WebSocket */
  Senders senders;
};


} // End of namespace ws


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_WS_RESPONSE_H
