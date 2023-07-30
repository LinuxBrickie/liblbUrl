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
#ifndef CONNECTIONDETAILS_H
#define CONNECTIONDETAILS_H

#include <lb/httpd/Server.h>


namespace httpd
{


enum class ServerType
{
  eBasic,     // Just HTTP handling
  eWebSocket, // HTTP handling and WebSocket support
};


using ServerConfigs = std::vector< lb::httpd::Server::Config >;


using ConnectionDetails = std::map< ServerType, ServerConfigs >;


} // End of namespace httpd


#endif // CONNECTIONDETAILS_H
