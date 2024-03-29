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

#include <lb/url/Version.h>


TEST(Library, Version)
{
  const int major{ 1 };
  const int minor{ 3 };
  const int patch{ 0 };

  const int version{ ( major << 16 ) + ( minor <<  8 ) + ( patch <<  0 ) };

  EXPECT_EQ( LIB_LB_URL_VERSION, std::to_string( major ) + '.'
                               + std::to_string( minor ) + '.'
                               + std::to_string( patch ) );
  EXPECT_EQ( LIB_LB_URL_VERSION_MAJOR, major );
  EXPECT_EQ( LIB_LB_URL_VERSION_MINOR, minor );
  EXPECT_EQ( LIB_LB_URL_VERSION_PATCH, patch );
  EXPECT_EQ( LIB_LB_URL_VERSION_NUM, version );

  EXPECT_EQ( lb::url::version.number, version );
}
