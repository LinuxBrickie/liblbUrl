#ifndef LIB_LB_URL_HTTP_REQUEST_H
#define LIB_LB_URL_HTTP_REQUEST_H

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

#include "../mime/MimePart.h"

#include <functional>
#include <string>
#include <vector>


namespace lb
{


namespace url
{


namespace http
{


struct Request
{
  Request() = default;

  // Default move/copy construction and assignment.
  Request( Request&& ) = default;
  Request& operator=( Request& ) = default;
  Request( const Request& ) = default;
  Request& operator=( const Request& ) = default;

  enum class Method
  {
    eInvalid,
    eGet,
    eHead,
    ePost,
    ePut,
    eDelete
  } method{ Method::eInvalid };

  std::string url;

  using Headers = std::vector<std::string>;
  Headers headers;

  std::string postData;

  mime::Mime mimePost;
};


} // End of namespace http


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_HTTP_REQUEST_H
