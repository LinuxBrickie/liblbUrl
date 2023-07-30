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

#include <iostream>

#include <curl/curl.h>


int main()
{
  curl_version_info_data*const vi{ curl_version_info( CURLVERSION_NOW ) };

  std::cout << "\nlibcurl version: " << vi->version << std::endl;

  std::cout << "\nSupported protocols:\n\n";
  const char*const * p{ vi->protocols };
  while ( *p )
  {
    std::cout << *p << std::endl;
    ++p;
  }
}
