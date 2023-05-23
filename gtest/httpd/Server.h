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

#ifndef LIB_LB_URL_GTEST_HTTPD_SERVER_H
#define LIB_LB_URL_GTEST_HTTPD_SERVER_H

#include <microhttpd.h>

#include <functional>
#include <string>


namespace httpd
{


/** \brief Mock web server for gtests built round the C library libmicrohttpd.

    Runs on its own internal thread. There is _no_ thread per request.
 */
class Server
{
public:
  enum class Method
  {
    eInvalid,
    eGet,
    eHead,
    ePost,
    ePut,
    eDelete
  };

  enum class Version
  {
    eUnknown,
    e0_9,
    e1_0,
    e1_1,
    e2_0,
  };

  struct Response
  {
    unsigned int code;
    std::string content;
  };

  using RequestHandler = std::function< Response( std::string, // url
                                                  Method,
                                                  Version,
                                                  std::string ) >; // request payload

  Server( int port, RequestHandler );
  ~Server();

private:
  static MHD_Result accessHandlerCallback( void* cls
                                         , struct MHD_Connection* connection
                                         , const char* url
                                         , const char* method
                                         , const char* version
                                         , const char* upload_data
                                         , size_t* upload_data_size
                                         , void** con_cls );

  Response invokeRequestHandler( std::string
                               , Method
                               , Version
                               , std::string );

  MHD_Daemon*const mhd;

  RequestHandler requestHandler;
};


} // End of namespace httpd


#endif // LIB_LB_URL_GTEST_HTTPD_SERVER_H
