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

#include "ConnectionDetails.h"
#include "httpd/ServerEnvironment.h"
#include "MockServerResponse.h"


int main( int argc, char** argv )
{
  testing::InitGoogleTest( &argc, argv );
  AddGlobalTestEnvironment( new httpd::ServerEnvironment{ PORT
                                                        , mockServerResponse } );
  return RUN_ALL_TESTS();
}
