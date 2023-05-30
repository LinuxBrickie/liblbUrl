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

#include <lb/url/http/UrlEncodedValuesCreator.h>

#include <curl/curl.h>


namespace lb
{


namespace url
{


namespace http
{


bool UrlEncodedValuesCreator::add( const Encodable& field
                                 , const Encodable& value )
{
  if ( field.s.empty() )
  {
    return false;
  }

  if ( !empty )
  {
    ss << '&';
  }

  if ( !addEncodable( field ) )
  {
    return false;
  }

  ss << '=';

  if ( !addEncodable( value ) )
  {
    return false;
  }

  empty = false;

  return true;
}

std::string UrlEncodedValuesCreator::str() const
{
  return ss.str();
}

void UrlEncodedValuesCreator::clear()
{
  ss.str( {} );
  ss.clear();
  empty = true;
}

bool UrlEncodedValuesCreator::addEncodable( const Encodable& encodable )
{
  if ( encodable.needsEncoded )
  {
    auto escaped
    {
      curl_easy_escape( nullptr /* Not used, don't panic. */
                      , encodable.s.c_str()
                      , encodable.s.size() )
    };
    if ( !escaped )
    {
      return false;
    }

    ss << escaped;

    curl_free( escaped );
  }
  else
  {
    ss << encodable.s;
  }

  return true;
}

} // End of namespace http


} // End of namespace url


} // End of namespace lb
