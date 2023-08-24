#ifndef LIB_LB_URL_VERSION_H
#define LIB_LB_URL_VERSION_H

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

// Version macros
#define LIB_LB_URL_VERSION "1.1.0"
#define LIB_LB_URL_VERSION_MAJOR 1
#define LIB_LB_URL_VERSION_MINOR 1
#define LIB_LB_URL_VERSION_PATCH 0
#define LIB_LB_URL_VERSION_NUM 0x010000


namespace lb
{


namespace url
{

/** \brief Contains library version information. */
const struct Version
{
  const unsigned int number{ LIB_LB_URL_VERSION_NUM };
}
version;


} // End of namespace url


} // End of namespace lb


#endif // VLIB_LB_URL_ERSION_H
