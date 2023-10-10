#ifndef LIB_LB_URL_REQUESTER_H
#define LIB_LB_URL_REQUESTER_H

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

#include <lb/url/http/Request.h>
#include <lb/url/http/Response.h>

#include <lb/url/ws/Request.h>
#include <lb/url/ws/Response.h>

#include <functional>
#include <memory>
#include <string>


namespace lb
{


namespace url
{


/** \brief Handles one or more requests without blocking.

    Runs a polling loop in it's own thread. When a request's data is available
    the Response callback is invoked. This is invoked in the Requester's own
    thread so ideally don't do any heavy lifting there.
 */
class Requester
{
public:
    struct Config
    {
      size_t pollTimeoutMilliseconds{ 50 };
    };

    static Config defaultConfig() { return Config{}; } // gcc bug workaround

    Requester( Config = defaultConfig() );
    ~Requester();

    /** \brief Submit request for URL asynchronously.

        The call will not block. Instead the request will be serviced in
        a thread and the response function will be invoked upon completion.
     */
    void makeRequest( http::Request, http::Response::Callback );

    /** \brief Submit request to open a WebSocket.

        This is not a typical URL request although it starts out like that. An
        intiial HTTP connection is made which is then upgraded to a WebSocket
        connection which is a two-way persistent connection. As such the response
        "callback" contains an object allowing the caller to send further data
        over the WebSocket connection. The request object contains an object
        that \a Requester can pass received data to.
     */
    void makeRequest( ws::Request, ws::Response::Callback );

    /** \brief Check that the global initialisation of the curl library is successful.

       Global initialisation happens before main(). If false then the library is unusable.
     */
    static bool wasGlobalInitSuccessful();

    /** \brief A human readable string returned directly from the curl library.
     */
    static std::string getCurlVersion();

private:
    struct Private;
    std::unique_ptr<Private> d;
};


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_REQUESTER_H
