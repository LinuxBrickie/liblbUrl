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

#ifndef LIB_LB_URL_GTEST_HTTPD_SERVERENVIRONEMENT_H
#define LIB_LB_URL_GTEST_HTTPD_SERVERENVIRONEMENT_H

#include <gtest/gtest.h>

#include <memory>

#include "Server.h"


namespace httpd
{


class ServerEnvironment : public testing::Environment
{
public:
  ServerEnvironment( int port, Server::RequestHandler handler )
    : port{ port }, requestHandler{ handler } {}

private:
  void SetUp() override
  {
    server = std::make_unique<Server>( port, requestHandler );
  }

  void TearDown() override
  {
  }

  const int port;
  Server::RequestHandler requestHandler;
  std::unique_ptr< Server > server;
};


} // End of namespace httpd


#endif // LIB_LB_URL_GTEST_HTTPD_SERVERENVIRONEMENT_H
