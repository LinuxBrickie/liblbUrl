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

#include "MockServerResponse.h"

#include "TestRequester.h"


httpd::Server::Response mockServerResponse( std::string url,
                                            httpd::Server::Method method,
                                            httpd::Server::Version version,
                                            std::string requestPayload )
{
  httpd::Server::Response response;

  switch ( method )
  {
  case httpd::Server::Method::eInvalid:
    response.content = "Invalid HTTP method";
    break;
  case httpd::Server::Method::eGet:
    response = GET_ExpectedMockResponse;
    break;
  case httpd::Server::Method::eHead:
    break;
  case httpd::Server::Method::ePost:
    break;
  case httpd::Server::Method::ePut:
    break;
  case httpd::Server::Method::eDelete:
    break;
  }

  return response;
}
