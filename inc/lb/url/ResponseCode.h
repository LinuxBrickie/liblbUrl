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

#ifndef LIB_LB_URL_RESPONSECODE_H
#define LIB_LB_URL_RESPONSECODE_H

#include <string>


namespace lb
{


namespace url
{


enum class ResponseCode
{
  eSendFailure,
  eFailure,
  eAborted,
  eTimedOut,
  eSuccess
};


std::string toString( ResponseCode );


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_RESPONSECODE_H
