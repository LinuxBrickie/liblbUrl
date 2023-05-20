#ifndef LIB_LB_URL_MIMEHANDLER_H
#define LIB_LB_URL_MIMEHANDLER_H

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
#include <lb/url/mime/MimePart.h>

#include <curl/curl.h>


namespace lb
{


namespace url
{


/** A helper for mime::MimeParts that can be used in RequestHandler subclasses. */
struct MimeHelper
{
  MimeHelper(); //!< No mime
  MimeHelper( mime::Mime mp );
  ~MimeHelper();

  bool setOptions( CURL* );

  // C-style callbacks used by libcurl. The void* is the address of the relevant mime::MimePart::DataReader instance.
  static size_t dataRead( char *buffer, size_t size, size_t nitems, void* userData );
  static int dataSeek( void* userData, curl_off_t offset, int origin );

  mime::Mime mime;

  curl_mime* mimeParts { nullptr };
};


} // End of namespace url


} // End of namespace lb


#endif // LIB_LB_URL_MIMEHANDLER_H
