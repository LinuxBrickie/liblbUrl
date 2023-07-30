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

#include "ConnectionDetails.h"


namespace httpd
{


class ServerEnvironment : public testing::Environment
{
public:
  ServerEnvironment( ConnectionDetails cd
                   , lb::httpd::Server::RequestHandler requestHandler
                   , lb::httpd::ws::Handler webSocketHandler )
    : connectionDetails{ cd }, requestHandler{ requestHandler }, webSocketHandler{ webSocketHandler } {}

private:
  void SetUp() override
  {
    for ( const auto&[type, serverConfigs] : connectionDetails )
    {
      for ( const auto& serverConfig : serverConfigs )
      {
        switch ( type )
        {
        case httpd::ServerType::eBasic:
          servers[ type ].emplace_back( serverConfig
                                      , requestHandler );
          break;
        case httpd::ServerType::eWebSocket:
          servers[ type ].emplace_back( serverConfig
                                      , requestHandler
                                      , webSocketHandler );
          break;
        }

      }
    }
  }

  void TearDown() override
  {
    servers.clear();
  }

  const ConnectionDetails connectionDetails;
  lb::httpd::Server::RequestHandler     requestHandler;
  lb::httpd::ws::Handler webSocketHandler;

  std::map< ServerType, std::vector<lb::httpd::Server> > servers;
};


} // End of namespace httpd


#endif // LIB_LB_URL_GTEST_HTTPD_SERVERENVIRONEMENT_H
