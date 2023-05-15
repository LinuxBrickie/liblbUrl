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

#include "MimeHelper.h"


namespace lb
{


namespace url
{


MimeHelper::MimeHelper()
{
}

MimeHelper::MimeHelper( mime::Mime mp )
  : mime{ std::move( mp ) }
{
}

MimeHelper::~MimeHelper()
{
  curl_mime_free( mimeParts );
}

bool MimeHelper::setOptions( CURL* easyHandle )
{
  if ( mime.parts.empty() )
  {
    return true;
  }

  mimeParts = curl_mime_init( easyHandle );

  for ( auto& part : mime.parts )
  {
    curl_mimepart* p = curl_mime_addpart( mimeParts );

    if ( !part.type.empty() )
    {
      if ( curl_mime_type( p, part.type.c_str() ) != CURLE_OK )
      {
        return false;
      }
    }

    if ( !part.encoding.empty() )
    {
      if ( curl_mime_encoder( p, part.encoding.c_str() ) != CURLE_OK )
      {
        return false;
      }
    }

    curl_mime_name( p, part.name.c_str() );

    if ( part.dataReader.dataReadFn )
    {
      curl_mime_data_cb( p, part.dataReader.totalNumBytes, &dataRead, &dataSeek, nullptr, &part.dataReader );
    }
    else // data is set in the data string buffer, ready to be copied
    {
      if ( !part.data.empty() && ( part.data.back() == '\0' ) )
      {
        curl_mime_data( p, part.data.c_str(), CURL_ZERO_TERMINATED );
      }
      else
      {
        curl_mime_data( p, part.data.c_str(), part.data.size() );
      }
    }
  }

  curl_easy_setopt( easyHandle, CURLOPT_MIMEPOST, mimeParts );

  return true;
}

void MimeHelper::freeResources( CURL* )
{
}

// static
size_t MimeHelper::dataRead( char* buffer, size_t size, size_t nitems, void* userData )
{
  return ((mime::MimePart::DataReader*)(userData))->dataReadFn( buffer, size * nitems );
}

// static
int MimeHelper::dataSeek( void* userData, curl_off_t offset, int origin )
{
  return ((mime::MimePart::DataReader*)(userData))->dataSeekFn( offset, origin );
}


} // End of namespace url


} // End of namespace lb

