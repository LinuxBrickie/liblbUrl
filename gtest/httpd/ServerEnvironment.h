#ifndef LIB_LB_URL_GTEST_HTTPD_SERVERENVIRONEMENT_H
#define LIB_LB_URL_GTEST_HTTPD_SERVERENVIRONEMENT_H

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

#include <gtest/gtest.h>

#include <memory>

#include <lb/httpd/Server.h>


namespace httpd
{


class ServerEnvironment : public testing::Environment
{
public:
  ServerEnvironment( int port, lb::httpd::Server::RequestHandler handler )
    : port{ port }, requestHandler{ handler } {}

private:
  void SetUp() override
  {
    server = std::make_unique<lb::httpd::Server>( port, requestHandler );
  }

  void TearDown() override
  {
  }

  const int port;
  lb::httpd::Server::RequestHandler requestHandler;
  std::unique_ptr< lb::httpd::Server > server;
};


} // End of namespace httpd


#endif // LIB_LB_URL_GTEST_HTTPD_SERVERENVIRONEMENT_H
