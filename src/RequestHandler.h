#ifndef LIB_LB_URL_REQUESTHANDLER_H
#define LIB_LB_URL_REQUESTHANDLER_H

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

#include <lb/url/ResponseCode.h>

#include <curl/curl.h>


namespace lb
{


namespace url
{


/** Abstract base class that wraps the easy handle and the receiverd data.

    An example subclass is HttpHandler.
 */
struct RequestHandler
{
  RequestHandler();
  virtual ~RequestHandler();

  // Move only, no copy
  RequestHandler( RequestHandler&& moveFrom );
  RequestHandler( const RequestHandler& ) = delete;
  RequestHandler& operator=( const RequestHandler& ) = delete;
  RequestHandler& operator=( RequestHandler&& ) = delete;

  CURL* getHandle() const;

  void processInfo();

  void respond( ResponseCode );

protected:
  virtual void respond( ResponseCode, std::string ) = 0;

  CURL* easyHandle;
  std::string receivedData;

private:
  static size_t writeCallback( char* data, size_t size, size_t numBytes, void* userData );

  void processReceivedData( const char* data, size_t numBytes );
};


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_REQUESTHANDLER_H
