#ifndef LIB_LB_URL_HTTPHANDLER_H
#define LIB_LB_URL_HTTPHANDLER_H

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

// Private header

#include <lb/url/http/Request.h>
#include <lb/url/http/Response.h>

#include "RequestHandler.h"
#include "MimeHelper.h"


namespace lb
{


namespace url
{


struct HttpHandler : public RequestHandler
{
  HttpHandler( http::Request r, http::Response::Callback c );
  ~HttpHandler();

  virtual Status respond( ResponseCode, std::string );

  http::Request request;
  http::Response::Callback responseCallback;

  curl_slist* headerList{ nullptr };

  MimeHelper mimeHelper;
};


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_HTTPHANDLER_H
